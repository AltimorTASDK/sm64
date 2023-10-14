#ifndef MONKEY_BALL_H
#define MONKEY_BALL_H

#include "engine/math_util.h"

#define BALL_STOP_SPEED 8.0f

struct GraphNodeCamera;
struct MarioState;
struct Surface;

void ball_update_world_tilt(struct MarioState *m);
void ball_update_floor_normal(struct MarioState *m, struct Surface *floor);
void ball_update_wall_normal(struct MarioState *m, struct Surface *wall);
void ball_update_surface_normals(struct MarioState *m);
void ball_rotate_vector(struct MarioState *m, Vec3f out, Vec3f v, s32 invert);
void ball_get_camera_transform(Mat4 transform, struct MarioState *m, struct GraphNodeCamera *node);

#endif // MONKEY_BALL_H
