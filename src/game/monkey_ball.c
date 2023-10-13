#include <PR/ultratypes.h>

#include "mario.h"
#include "area.h"
#include "engine/math_util.h"
#include "game_init.h"

#define MAX_TILT  DEGREES(15.0f)
#define LERP_RATE (f32)(2.0 * M_PI / 180.0)

void ball_update_world_tilt(struct MarioState *m) {
    s16 pitch = sqr(m->controller->stickMag / 64.0f) * -MAX_TILT;
    s16 yaw = atan2s(-m->controller->stickY, m->controller->stickX) + m->area->camera->yaw;

    // Swap sin and cos for pitch to get up vector
    Vec3f targetWorldUp;
    targetWorldUp[0] = sins(pitch) * sins(yaw);
    targetWorldUp[1] = coss(pitch);
    targetWorldUp[2] = sins(pitch) * coss(yaw);

    vec3f_slerp_rate(m->worldUp, m->worldUp, targetWorldUp, LERP_RATE);
    vec3f_normalize(m->worldUp); // Prevent cumulative magnitude error
}

void ball_rotate_vector(struct MarioState *m, Vec3f v) {
    Vec3f right, forward;
    f32 xzLength = sqrtf(v[0] * v[0] + v[2] * v[2]);

    if (xzLength != 0.0f) {
        vec3f_set(right, -v[2] / xzLength, 0.0f, v[0] / xzLength);
        vec3f_cross(forward, m->worldUp, right);
        vec3f_normalize(forward);
        v[0] = m->worldUp[0] * v[1] + forward[0] * xzLength;
        v[2] = m->worldUp[2] * v[1] + forward[2] * xzLength;
        v[1] = m->worldUp[1] * v[1] + forward[1] * xzLength;
    } else {
        v[0] = m->worldUp[0] * v[1];
        v[2] = m->worldUp[2] * v[1];
        v[1] = m->worldUp[1] * v[1];
    }
}
