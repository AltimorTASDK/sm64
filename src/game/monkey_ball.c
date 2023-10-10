/*#include <PR/ultratypes.h>

#include "mario.h"
#include "area.h"
#include "engine/math_util.h"
#include "game_init.h"

#define MAX_TILT 45.0f

static void get_desired_world_tilt(struct MarioState *m, Vec3f *up)
{
    struct Controller *controller = m->controller;
    f32 mag = ((controller->stickMag / 64.0f) * (controller->stickMag / 64.0f)) * MAX_TILT;

    if (m->squishTimer == 0) {
        m->intendedMag = mag / 2.0f;
    } else {
        m->intendedMag = mag / 8.0f;
    }

    if (m->intendedMag > 0.0f) {
        m->intendedYaw = atan2s(-controller->stickY, controller->stickX) + m->area->camera->yaw;
    } else {
        m->intendedYaw = m->faceAngle[1];
    }
}
*/