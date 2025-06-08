#ifndef COLLISION_H
#define COLLISION_H

#include <stdint.h>
#include <psxgpu.h>

#define CLAMP(v, min, max) \
    ((v < min) ? min : (v > max) ? max : v)

typedef struct {
    int32_t vx;
    int32_t vy;
} vec2;

vec2    closest_point_aabb0(RECT *r0, vec2 *p0);
vec2    ptdiff0(vec2 *a0, vec2 *b0);
int32_t sqdist0(vec2 *a0, vec2 *b0, vec2 *r_delta);
uint8_t test_circ0_aabb0(vec2 *circp, int32_t r, RECT *aabb, vec2 *r_delta);


uint8_t collision_ball_box(vec2 *ballpos, int32_t r,
                           RECT *box,
                           vec2 *r_mov, vec2 *r_pos);

#endif
