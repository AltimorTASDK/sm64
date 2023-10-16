#include <PR/ultratypes.h>

#include "monkey_ball.h"
#include "mario.h"
#include "area.h"
#include "sm64.h"
#include "game_init.h"
#include "camera.h"
#include "engine/graph_node.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"

#define MAX_TILT  DEGREES(30.0f)
#define LERP_RATE (f32)(5.0 * M_PI / 180.0)

#define CAMERA_SIZE 40.0f

s32 ball_allow_tilt(struct MarioState *m) {
    if (m->input & INPUT_FIRST_PERSON) {
        return FALSE;
    } else if (m->action & (ACT_FLAG_ON_POLE | ACT_FLAG_RIDING_SHELL | ACT_FLAG_SWIMMING)) {
        return FALSE;
    } else if (m->action == ACT_LAVA_BOOST) {
        return FALSE;
    } else if (m->action == ACT_FIRST_PERSON) {
        return FALSE;
    } else if (m->action == ACT_IN_CANNON) {
        return FALSE;
    } else if (m->action == ACT_PICKING_UP_BOWSER) {
        return FALSE;
    } else if (m->action == ACT_HOLDING_BOWSER) {
        return FALSE;
    } else if (m->action == ACT_RELEASING_BOWSER) {
        return FALSE;
    } else if (m->action == ACT_READING_AUTOMATIC_DIALOG) {
        return FALSE;
    } else if (m->action == ACT_READING_NPC_DIALOG) {
        return FALSE;
    } else if (m->action == ACT_READING_SIGN) {
        return FALSE;
    } else if (m->action == ACT_INTRO_CUTSCENE) {
        return FALSE;
    } else {
        return TRUE;
    }
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
    } else if (m->action == ACT_PUNCHING) {
        return TRUE;
    } else if (m->action == ACT_MOVE_PUNCHING) {
        return TRUE;
    } else {
        return FALSE;
    }
}

s32 ball_is_moving(struct MarioState *m) {
    if (sqr(m->vel[0]) + sqr(m->vel[2]) <= sqr(BALL_STOP_SPEED)) {
        return FALSE;
    } else if (!ball_allow_tilt(m)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

void ball_update_world_tilt(struct MarioState *m) {
    Vec3f targetWorldUp;

    if (ball_allow_tilt(m)) {
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

void ball_rotate_vector(struct MarioState *m, Vec3f out, Vec3f v, s32 invert) {
    Vec3f forward;
    Vec3f right;
    Vec3f up;
    f32 xzLength = sqrtf(sqr(v[0]) + sqr(v[2]));

    if (invert) {
        vec3f_set(up, -m->worldUp[0], m->worldUp[1], -m->worldUp[2]);
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

static s32 should_apply_tilt(struct MarioState *m) {
    if (!gMarioIsInitialized) {
        return FALSE;
    } else if (m->worldUp[1] >= 1.0f) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void create_tilt_matrix(struct MarioState *m, Mat4 tiltMatrix, s32 invert) {
    Vec3f right;
    f32 angleSine = sqrtf(sqr(m->worldUp[0]) + sqr(m->worldUp[2]));
    vec3f_right(right, m->worldUp);
    mtxf_create_rot_matrix(tiltMatrix, right, invert ? -angleSine : angleSine, m->worldUp[1]);
}

static s32 should_tilt_camera(struct MarioState *m) {
    if (!should_apply_tilt(m)) {
        return FALSE;
    } else if (gLakituState.isPauseCam) {
        return FALSE;
    } else {
        return TRUE;
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
        height = find_floor(cameraPos[0], node->pos[1], cameraPos[2], &surface) + CAMERA_SIZE;
        if (surface == NULL) {
            height = find_floor(node->pos[0], node->pos[1], node->pos[2], &surface) + CAMERA_SIZE;
        }
        if (surface == NULL || height > node->pos[1]) {
            height = node->pos[1];
        } else if (height < cameraPos[1]) {
            return;
        }
    } else if (cameraPos[1] > node->pos[1]) {
        // Lower camera to ceiling
        height = find_ceil(cameraPos[0], node->pos[1], cameraPos[2], &surface) - CAMERA_SIZE;
        if (surface == NULL) {
            height = find_floor(node->pos[0], node->pos[1], node->pos[2], &surface) - CAMERA_SIZE;
        }
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

static void split_roll_matrix(Mat4 transform, Mat4 rollMatrix) {
    Mat4 transformRollMatrix;
    f32 roll;
    f32 pitchCos = sqrtf(1.0f - sqr(transform[1][2]));

    if (pitchCos <= 1e-4f) {
        // gumbo lock
        return;
    }

    roll = atan2f(transform[1][1] / pitchCos, transform[1][0] / pitchCos);

    // Remove roll from transform
    mtxf_rotate_xyf(transformRollMatrix, roll);
    mtxf_mul(transform, transform, transformRollMatrix);

    // Negate and apply roll to its own matrix
    transformRollMatrix[0][1] *= -1.0f;
    transformRollMatrix[1][0] *= -1.0f;
    mtxf_mul(rollMatrix, rollMatrix, transformRollMatrix);
}

void ball_get_camera_transform(Mat4 transform, Mat4 rollMatrix, struct MarioState *m,
                               struct GraphNodeCamera *node) {
    Vec3f pivotPoint;
    Vec3f pivotOffset;
    Vec3f relativePos;
    Vec3f translation;
    Mat4 translationMatrix;
    Mat4 tiltMatrix;

    if (!should_tilt_camera(m)) {
        mtxf_lookat(transform, node->pos, node->focus, node->roll);
        return;
    }

    // Make the world/camera pivot around Mario's centerpoint
    vec3f_set(pivotPoint, m->pos[0], m->pos[1] + 80.0f, m->pos[2]);
    vec3f_copy(pivotOffset, node->focus);
    vec3f_sub(pivotOffset, pivotPoint);

    vec3f_copy(relativePos, node->pos);
    vec3f_sub(relativePos, node->focus);
    vec3f_add(relativePos, pivotOffset);
    mtxf_lookat(transform, relativePos, pivotOffset, node->roll);

    create_tilt_matrix(m, tiltMatrix, TRUE);
    mtxf_mul(transform, tiltMatrix, transform);

    vec3f_set(translation, -node->focus[0], -node->focus[1], -node->focus[2]);
    vec3f_add(translation, pivotOffset);
    mtxf_translate(translationMatrix, translation);
    mtxf_mul(transform, translationMatrix, transform);

    handle_camera_collision(transform, node);
    split_roll_matrix(transform, rollMatrix);
}

static s32 should_tilt_model(struct MarioState *m) {
    if (!should_apply_tilt(m)) {
        return FALSE;
    } else if (m->marioObj->header.gfx.throwMatrix != NULL) {
        return FALSE;
    } else if (!(m->action & ACT_FLAG_AIR)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

void ball_update_mario_rotation(struct MarioState *m) {
    if (should_tilt_model(m)) {
        Mat4 tiltMatrix;
        Mat4 rotationMatrix;
        struct Object *obj = m->marioObj;

        mtxf_translate(m->tiltTransform, obj->header.gfx.pos);

        create_tilt_matrix(m, tiltMatrix, FALSE);
        mtxf_mul(m->tiltTransform, tiltMatrix, m->tiltTransform);

        mtxf_rotate_zxy_and_translate(rotationMatrix, gVec3fZero, obj->header.gfx.angle);
        mtxf_mul(m->tiltTransform, rotationMatrix, m->tiltTransform);

        obj->header.gfx.throwMatrix = &m->tiltTransform;
    }
}
