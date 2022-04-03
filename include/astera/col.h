// TODO Collision Layers, GJK & SAT, Broadphase, Capsules
// Forked from RandyGaul/cute_headers/cute_c2.h

#if !defined ASTERA_COL_H
#define ASTERA_COL_H

#include <astera/linmath.h>
#include <stdint.h>

#if !defined(ASTERA_COL_SKIN_WIDTH)
#define ASTERA_COL_SKIN_WIDTH 0.001f
#endif

typedef struct {
  vec2 min, max;
} c_aabb;

typedef struct {
  vec2  center;
  float radius;
} c_circle;

// NOTE: direction vector should be normalized
typedef struct {
  vec2  center, direction;
  float distance;
} c_ray;

typedef enum {
  C_NONE,
  C_AABB,
  C_CIRCLE,
  C_RAY,
} c_types;

typedef struct {
  union {
    c_aabb   aabb;
    c_circle circle;
    c_ray    ray;
  } col;
  c_types type;
} c_shape;

typedef struct {
  vec2  point, direction;
  float distance;
} c_manifold;

/* Create a ray
 * NOTE: this will normalize the direction if the length is greater
 *       than 1
 * center - the point/center to cast from direction
 * direction - the direction of the ray
 * distance - the max distance of the ray
 * returns: ray structure, 0 length = fail */
c_ray c_ray_create(vec2 center, vec2 direction, float distance);

/* Create an AABB (Axis Aligned Bounding Box)
 * center - the center of the aabb
 * halfsize - the size / 2
 * returns: aabb structure */
c_aabb c_aabb_create(vec2 center, vec2 halfsize);

/* Change an existing AABB to match the passed values
 * center - the center of the aabb
 * halfsize - the size / 2 */
void c_aabb_set(c_aabb* aabb, vec2 center, vec2 halfsize);

/* Move an AABB by distance
 * aabb - the AABB to move
 * distance - the distance to move the aabb */
void c_aabb_move(c_aabb* aabb, vec2 distance);

/* Move an AABB by adjustment manifold
 * aabb - the AABB to adjust
 * manifold - manifold to move/adjust by */
void c_aabb_adjust(c_aabb* aabb, c_manifold manifold);

/* Move a circle by distance
 * circle - the circle to move
 * distance - the distance to move the circle*/
void c_circle_move(c_circle* circle, vec2 distance);

/* Get the size of an AABB
 * dst - the destination to store the size
 * aabb - the aabb to get size of */
void c_aabb_get_size(vec2 dst, c_aabb aabb);

/* Get the center point of an AABB
 * dst - the destination to store the center
 * aabb - the aabb to get the center of */
void c_aabb_get_center(vec2 dst, c_aabb aabb);

/* Create a Circle
 * center - the center of the circle
 * radius - the radius of the circle
 * returns: circle structure */
c_circle c_circle_create(vec2 center, float radius);

/* Create an AABB from cirlce dimensions
 * circle - dimensions to use
 * returns: aabb representation of circle's dimension */
c_aabb c_circle_to_aabb(c_circle circle);

/* Test a ray vs aabb
 * a - the ray to test
 * b - the aabb to test
 * out - the raycast output
 * returns: 1 = colliding, 0 = not colliding */
uint8_t c_ray_vs_aabb(c_ray a, c_aabb b);

/* Test a ray vs circle
 * a - the ray to test
 * b - the circle to test
 * out - the raycast output
 * returns: 1 = colliding, 0 = not colliding */
uint8_t c_ray_vs_circle(c_ray a, c_circle b);

/* Test a ray vs aabb
 * a - the ray to test
 * b - the aabb to test
 * returns: manifold, 0 length = fail/not colliding */
c_manifold c_ray_vs_aabb_man(c_ray a, c_aabb b);

/* Test a ray vs circle
 * a - the ray to test
 * b - the circle to test
 * out - the raycast output
 * returns: manifold, 0 length = fail/not colliding */
c_manifold c_ray_vs_circle_man(c_ray a, c_circle b);

/* Test an AABB vs AABB
 * a - the first aabb to test
 * b - the second aabb to test
 * returns: 1 = colliding, 0 = not colliding */
uint8_t c_aabb_vs_aabb(c_aabb a, c_aabb b);

/* Test an AABB vs point
 * a - the aabb to test
 * point - the point to check
 * returns: 1 = intersecting, 0 = not */
uint8_t c_aabb_vs_point(c_aabb a, vec2 point);

/* Test an aabb vs a circle
 * a - the aabb
 * b - the circle
 * returns: 1 = colliding, 0 = not colliding */
uint8_t c_aabb_vs_circle(c_aabb a, c_circle b);

/* Test a circle vs a point
 * a - the circle
 * point - the point
 * returns: 1 = colliding, 0 = not colliding */
uint8_t c_circle_vs_point(c_circle a, vec2 point);

/* Test a circle vs a circle
 * a - the first circle
 * b - the second circle
 * returns: 1 = colliding, 0 = not colliding */
uint8_t c_circle_vs_circle(c_circle a, c_circle b);

/* Test collision of AABB vs AABB
 * a - the 1st aabb
 * b - the 2nd aabb
 * returns: manifold, 0 length = fail/not colliding */
c_manifold c_aabb_vs_aabb_man(c_aabb a, c_aabb b);

/* Test collision of AABB vs Circle (aabb resolves)
 * a - the AABB
 * b - the Circle
 * returns: manifold, 0 length = fail/not colliding */
c_manifold c_aabb_vs_circle_man(c_aabb a, c_circle b);

/* Test collision of circle vs circle
 * a - the 1st circle
 * b - the 2nd circle
 * returns: manifold, 0 length = fail/not colliding */
c_manifold c_circle_vs_circle_man(c_circle a, c_circle b);

/* Test collision of circle vs aabb (circle resolves)
 * a - the circle
 * b - the aabb
 * returns: manifold, 0 length = fail/not colliding */
c_manifold c_circle_vs_aabb_man(c_circle a, c_aabb b);

/* Test 2 Collider types
 * a - 1st collider
 * b - 2nd collider
 * returns: manifold, 0 length = fail/not colliding */
c_manifold c_test(c_shape a, c_shape b);

/* Get an AABB encompasing all shapes in the list
 * shapes - the list of shapes to check
 * count - the number of shapes */
c_aabb c_reduce(c_shape* shapes, uint32_t count);

#endif
