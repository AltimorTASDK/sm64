#ifndef MONKEY_BALL_H
#define MONKEY_BALL_H

#include "engine/math_util.h"

#define BALL_STOP_SPEED 8.0f

// ball cam lmao

#define BALL_CAM_FOV 60.0f

// Added camera distance
#define BALL_CAM_DISTANCE_MUL 0.7f
#define BALL_CAM_DISTANCE_ADD 400.0f

// The standard camera uses a height offset while others use pitch
#define BALL_CAM_HEIGHT 380.f
#define BALL_CAM_PITCH  DEGREES(30)

struct GraphNodeCamera;
struct MarioState;
struct Surface;

void ball_update_world_tilt(struct MarioState *m);
void ball_update_floor_normal(struct MarioState *m, struct Surface *floor);
void ball_update_wall_normal(struct MarioState *m, struct Surface *wall);
void ball_update_surface_normals(struct MarioState *m);
void ball_rotate_vector(struct MarioState *m, Vec3f out, Vec3f v, s32 invert);
void ball_get_camera_transform(Mat4 transform, struct MarioState *m, struct GraphNodeCamera *node);
s32 ball_can_interact(struct MarioState *m);

#endif // MONKEY_BALL_H
