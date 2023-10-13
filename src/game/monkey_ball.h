#ifndef MONKEY_BALL_H
#define MONKEY_BALL_H

#include "types.h"
#include "engine/math_util.h"

#define ball_normal_y(m, normal) (vec3f_dot(normal, m->worldUp))

void ball_update_world_tilt(struct MarioState *m);
void ball_rotate_vector(struct MarioState *m, Vec3f v);

#endif // MONKEY_BALL_H
