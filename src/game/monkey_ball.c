#include <PR/ultratypes.h>

#include "monkey_ball.h"
#include "mario.h"
#include "area.h"
#include "sm64.h"
#include "game_init.h"
#include "engine/graph_node.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"

#define MAX_TILT  DEGREES(30.0f)
#define LERP_RATE (f32)(5.0 * M_PI / 180.0)

static s32 allow_tilt(struct MarioState *m) {
    if (m->input & INPUT_IN_WATER) {
        return FALSE;
    } else if (m->action & (ACT_FLAG_ON_POLE | ACT_FLAG_RIDING_SHELL)) {
        return FALSE;
    } else if (m->action == ACT_LAVA_BOOST) {
        return FALSE;
    } else {
        return TRUE;
    }
}

void ball_update_world_tilt(struct MarioState *m) {
    Vec3f targetWorldUp;

    if (allow_tilt(m)) {
        s16 pitch = sqr(m->controller->stickMag / 64.0f) * MAX_TILT;
        s16 yaw = atan2s(-m->controller->stickY, m->controller->stickX) + m->area->camera->yaw;
        // Swap sin and cos for pitch to get up vector
        targetWorldUp[0] = sins(pitch) * sins(yaw);
        targetWorldUp[1] = coss(pitch);
        targetWorldUp[2] = sins(pitch) * coss(yaw);
    } else {
        vec3f_set(targetWorldUp, 0.0f, 1.0f, 0.0f);
    }

    vec3f_slerp_rate(m->worldUp, m->worldUp, targetWorldUp, LERP_RATE);
    vec3f_normalize(m->worldUp); // Prevent cumulative magnitude error
}

void ball_update_floor_normal(struct MarioState *m, struct Surface *floor) {
    ball_rotate_vector(m, &m->floorNormal.x, &floor->normal.x, FALSE);
}

void ball_update_wall_normal(struct MarioState *m, struct Surface *wall) {
    ball_rotate_vector(m, &m->wallNormal.x, &wall->normal.x, FALSE);
}

void ball_update_surface_normals(struct MarioState *m) {
    if (m->floor != NULL) {
        ball_rotate_vector(m, &m->floorNormal.x, &m->floor->normal.x, FALSE);
    }
    if (m->wall != NULL) {
        ball_rotate_vector(m, &m->wallNormal.x, &m->wall->normal.x, FALSE);
    }
}

void ball_rotate_vector(struct MarioState *m, Vec3f out, Vec3f v, s32 invert) {
    Vec3f forward;
    Vec3f right;
    Vec3f up;
    f32 xzLength = sqrtf(v[0] * v[0] + v[2] * v[2]);

    if (invert) {
        up[0] = -m->worldUp[0];
        up[1] = m->worldUp[1];
        up[2] = -m->worldUp[2];
    } else {
        vec3f_copy(up, m->worldUp);
    }

    if (xzLength != 0.0f) {
        vec3f_set(right, -v[2] / xzLength, 0.0f, v[0] / xzLength);
        vec3f_cross(forward, up, right);
        vec3f_normalize(forward);
        out[0] = up[0] * v[1] + forward[0] * xzLength;
        out[2] = up[2] * v[1] + forward[2] * xzLength;
        out[1] = up[1] * v[1] + forward[1] * xzLength;
    } else {
        out[0] = up[0] * v[1];
        out[2] = up[2] * v[1];
        out[1] = up[1] * v[1];
    }
}

static void handle_camera_collision(Mat4 transform, struct GraphNodeCamera *node) {
    Vec3f cameraPos;
    f32 height;
    Vec3f translation;
    Mat4 translationMatrix;
    struct Surface *surface;

    mtxf_get_transform_position(cameraPos, transform);

    if (cameraPos[1] < node->pos[1]) {
        // Raise camera to floor
        height = find_floor(cameraPos[0], node->pos[1], cameraPos[2], &surface) + 125.0f;
        if (surface == NULL || height > node->pos[1]) {
            height = node->pos[1];
        } else if (height < cameraPos[1]) {
            return;
        }
    } else if (cameraPos[1] > node->pos[1]) {
        // Lower camera to ceiling
        height = find_ceil(cameraPos[0], node->pos[1], cameraPos[2], &surface) - 125.0f;
        if (surface == NULL || height < node->pos[1]) {
            height = node->pos[1];
        } else if (height > cameraPos[1]) {
            return;
        }
    } else {
        return;
    }

    vec3f_set(translation, 0.0f, cameraPos[1] - height, 0.0f);
    mtxf_translate(translationMatrix, translation);
    mtxf_mul(transform, translationMatrix, transform);
}

void ball_get_camera_transform(Mat4 transform, struct MarioState *m, struct GraphNodeCamera *node) {
    Vec3f offset;
    Vec3f right;
    Vec3f translation;
    Mat4 translationMatrix;
    Mat4 rotationMatrix;
    f32 *worldUp;
    f32 angleSine;

    if (!gMarioIsInitialized || m->worldUp[1] >= 1.0f) {
        mtxf_lookat(transform, node->pos, node->focus, node->roll);
        return;
    }

    worldUp = m->worldUp;
    angleSine = sqrtf(sqr(worldUp[0]) + sqr(worldUp[2]));

    vec3f_copy(offset, node->pos);
    vec3f_sub(offset, node->focus);
    mtxf_lookat(transform, offset, gVec3fZero, node->roll);

    vec3f_right(right, worldUp);
    right[0] *= -1.0f;
    right[2] *= -1.0f;
    mtxf_create_rot_matrix(rotationMatrix, right, angleSine, worldUp[1]);
    mtxf_mul(transform, rotationMatrix, transform);

    vec3f_set(translation, -node->focus[0], -node->focus[1], -node->focus[2]);
    mtxf_translate(translationMatrix, translation);
    mtxf_mul(transform, translationMatrix, transform);

    handle_camera_collision(transform, node);
}

s32 ball_can_interact(struct MarioState *m) {
    if (m->action & ACT_FLAG_IDLE) {
        return TRUE;
    } else if (m->action == ACT_WALKING) {
        return TRUE;
    } else if (m->action & ACT_FLAG_BUTT_OR_STOMACH_SLIDE) {
        return TRUE;
    } else if (m->action == ACT_DIVE_SLIDE) {
        return TRUE;
    } else {
        return FALSE;
    }
}
