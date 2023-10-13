#include <PR/ultratypes.h>

#include "monkey_ball.h"
#include "mario.h"
#include "area.h"
#include "engine/math_util.h"
#include "game_init.h"

#define MAX_TILT  DEGREES(15.0f)
#define LERP_RATE (f32)(5.0 * M_PI / 180.0)

void ball_update_world_tilt(struct MarioState *m) {
    s16 pitch = sqr(m->controller->stickMag / 64.0f) * MAX_TILT;
    s16 yaw = atan2s(-m->controller->stickY, m->controller->stickX) + m->area->camera->yaw;

    // Swap sin and cos for pitch to get up vector
    Vec3f targetWorldUp;
    targetWorldUp[0] = sins(pitch) * sins(yaw);
    targetWorldUp[1] = coss(pitch);
    targetWorldUp[2] = sins(pitch) * coss(yaw);

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
    Vec3f forward, right, up;
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
