#ifndef MONKEY_BALL_H
#define MONKEY_BALL_H

#include "engine/math_util.h"

#define BALL_STOP_SPEED 8.0f

// ball cam lmao

#define BALL_CAM_FOV 74.0f

// Added camera distance
#define BALL_CAM_DISTANCE_MUL 0.54968f
#define BALL_CAM_DISTANCE_ADD 0.0f

// The standard camera uses a height offset while others use pitch
#define BALL_CAM_HEIGHT 0.0f
#define BALL_CAM_PITCH  DEGREES(0)

// Adjust nearz so it isn't hit when the camera is near the ground
#define BALL_MAX_NEARZ 15

// calc_y_to_curr_floor parameters
#define BALL_CAM_POS_MUL   1.0f
#define BALL_CAM_POS_BOUND 200.0f
#define BALL_CAM_FOC_MUL   0.9f
#define BALL_CAM_FOC_BOUND 200.0f

struct GraphNodeCamera;
struct MarioState;
struct Surface;

s32 ball_allow_tilt(struct MarioState *m);
void ball_update_world_tilt(struct MarioState *m);
void ball_update_floor_normal(struct MarioState *m, struct Surface *floor);
void ball_update_wall_normal(struct MarioState *m, struct Surface *wall);
void ball_update_surface_normals(struct MarioState *m);
void ball_rotate_vector(struct MarioState *m, Vec3f out, Vec3f v, s32 invert);
void ball_get_camera_transform(Mat4 transform, Mat4 rollMatrix, struct MarioState *m,
                               struct GraphNodeCamera *node);
s32 ball_can_interact(struct MarioState *m);
s32 ball_is_moving(struct MarioState *m);

#endif // MONKEY_BALL_H
