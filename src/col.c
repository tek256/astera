#include <astera/col.h>
#include <astera/debug.h>

#include <stdio.h>

#include <math.h>

c_ray c_ray_create(vec2 center, vec2 direction, float distance) {
  c_ray ray = {.distance = distance};
  vec2_dup(ray.direction, direction);
  vec2_dup(ray.center, center);
  return ray;
}

c_aabb c_aabb_create(vec2 center, vec2 halfsize) {
  c_aabb aabb = {0};
  vec2_sub(aabb.min, center, halfsize);
  vec2_add(aabb.max, center, halfsize);
  return aabb;
}

void c_aabb_set(c_aabb* aabb, vec2 center, vec2 halfsize) {
  vec2_sub(aabb->min, center, halfsize);
  vec2_add(aabb->max, center, halfsize);
}

c_circle c_circle_create(vec2 center, float radius) {
  c_circle circle = (c_circle){.radius = radius};
  vec2_dup(circle.center, center);
  return circle;
}

c_aabb c_circle_to_aabb(c_circle circle) {
  c_aabb aabb = (c_aabb){0};
  vec2_set(aabb.min, circle.center[0] - circle.radius,
           circle.center[1] - circle.radius);
  vec2_set(aabb.min, circle.center[0] + circle.radius,
           circle.center[1] + circle.radius);
  return aabb;
}

void c_aabb_move(c_aabb* aabb, vec2 distance) {
  vec2_add(aabb->min, aabb->min, distance);
  vec2_add(aabb->max, aabb->max, distance);
}

void c_aabb_adjust(c_aabb* aabb, c_manifold manifold) {
  vec2 dist = {manifold.direction[0] * manifold.distance,
               manifold.direction[1] * manifold.distance};
  c_aabb_move(aabb, dist);
}

void c_circle_move(c_circle* circle, vec2 distance) {
  circle->center[0] += distance[0];
  circle->center[1] += distance[1];
}

void c_ray_move(c_ray* ray, vec2 distance) {
  vec2_add(ray->center, ray->center, distance);
}

void c_aabb_get_size(vec2 dst, c_aabb aabb) {
  vec2_sub(dst, aabb.max, aabb.min);
}

void c_aabb_get_center(vec2 dst, c_aabb aabb) {
  dst[0] = aabb.min[0] + ((aabb.max[0] - aabb.min[0]) * 0.5f);
  dst[1] = aabb.min[1] + ((aabb.max[1] - aabb.min[1]) * 0.5f);
}

// signed distance point to plane one dimensional
static inline float _sdpo(float p, float n, float d) {
  return p * n - d * n;
}

// one dimensional day vs plane
static inline float _r2plane(float da, float db) {
  if (da < 0)
    return 0;
  else if (da * db >= 0)
    return 1.f;
  else {
    float d = da - db;
    if (d != 0)
      return da / d;
    else
      return 0;
  }
}

uint8_t c_ray_vs_circle(c_ray ray, c_circle B) {
  vec2 p, m;
  vec2_dup(p, B.center);
  float c, b, disc;

  vec2_sub(m, ray.center, p);
  c = vec2_dot(m, m) - B.radius * B.radius;
  b = vec2_dot(m, ray.direction);

  disc = b * b - c;

  if (disc < 0)
    return 0;

  float t = -b - sqrtf(disc);
  if (t >= 0 && t <= ray.distance) {
    return 1;
  }

  return 0;
}

uint8_t c_ray_vs_aabb(c_ray A, c_aabb B) {
  float diff_x = 1.f / A.direction[0];
  float diff_y = 1.f / A.direction[1];

  float t1 = (B.min[0] - A.center[0]) * diff_x;
  float t2 = (B.max[0] - A.center[0]) * diff_x;
  float t3 = (B.min[1] - A.center[1]) * diff_y;
  float t4 = (B.max[1] - A.center[1]) * diff_y;

  float tmin = fmax(fmin(t1, t2), fmin(t3, t4));
  float tmax = fmin(fmax(t1, t2), fmax(t3, t4));

  // behind ray
  if (tmax < 0 || tmin < 0) {
    return 0;
  }

  // non-intersectional
  if (tmin > tmax) {
    return 0;
  }

  // out of range
  if (tmin > A.distance) {
    return 0;
  }

  return 1;
}

c_manifold c_ray_vs_aabb_man(c_ray A, c_aabb B) {
  float diff_x = 1.f / A.direction[0];
  float diff_y = 1.f / A.direction[1];

  float t1 = (B.min[0] - A.center[0]) * diff_x;
  float t2 = (B.max[0] - A.center[0]) * diff_x;
  float t3 = (B.min[1] - A.center[1]) * diff_y;
  float t4 = (B.max[1] - A.center[1]) * diff_y;

  float tmin = fmax(fmin(t1, t2), fmin(t3, t4));
  float tmax = fmin(fmax(t1, t2), fmax(t3, t4));

  c_manifold man = (c_manifold){0};

  // behind ray
  if (tmax < 0 || tmin < 0) {
    return man;
  }

  if (tmin > tmax) {
    return man;
  }

  // out of range
  if (tmin > A.distance) {
    return man;
  }

  man.distance = tmin;
  vec2 delta   = {0};

  vec2_scale(delta, A.direction, tmin);
  vec2_add(man.point, A.center, delta);
  vec2_dup(man.direction, A.direction);

  return man;
}

/* Test a ray vs circle
 * ray - the ray to test
 * b - the circle to test
 * out - the raycast output
 * returns: manifold, 0 length = fail/not colliding */
c_manifold c_ray_vs_circle_man(c_ray a, c_circle B) {
  vec2 p, m;
  vec2_dup(p, B.center);
  float c, b, disc;

  vec2_sub(m, a.center, p);
  c = vec2_dot(m, m) - B.radius * B.radius;
  b = vec2_dot(m, a.direction);

  disc = b * b - c;

  c_manifold man = (c_manifold){0};

  if (disc < 0)
    return man;

  float t = -b - sqrtf(disc);
  if (t >= 0 && t <= a.distance) {
    // set the output distance (time)
    man.distance = t;
    vec2 impact, dist;

    // Calculate the impact point
    vec2_scale(dist, a.direction, t);
    vec2_add(impact, dist, a.center);
    vec2_dup(man.point, impact);

    // Calculate the normal
    vec2_sub(dist, impact, p);
    vec2_norm(impact, dist);
    vec2_dup(man.direction, impact);
  }

  return man;
}

uint8_t c_aabb_vs_aabb(c_aabb a, c_aabb b) {
  int x0 = b.max[0] < a.min[0];
  int x1 = a.max[0] < b.min[0];
  int y0 = b.max[1] < a.min[1];
  int y1 = a.max[1] < b.min[1];
  return !(x0 | x1 | y0 | y1);
}

uint8_t c_aabb_vs_point(c_aabb a, vec2 point) {
  int x0 = point[0] < a.min[0];
  int x1 = point[0] > a.max[0];
  int y0 = point[1] < a.min[1];
  int y1 = point[1] > a.max[1];
  return !(x0 | x1 | y0 | y1);
}

uint8_t c_aabb_vs_circle(c_aabb a, c_circle b) {
  vec2 len = {0.f, 0.f}, ab = {0.f, 0.f};
  vec2_clamp(len, b.center, a.min, a.max);
  vec2_sub(ab, b.center, len);
  float dot = vec2_dot(ab, ab);
  float rad = b.radius * b.radius;
  return dot < rad;
}

uint8_t c_circle_vs_point(c_circle a, vec2 point) {
  vec2 norm = {0.f};
  vec2_sub(norm, a.center, point);
  float dot = vec2_dot(norm, norm);
  return dot < a.radius * a.radius;
}

uint8_t c_circle_vs_circle(c_circle a, c_circle b) {
  vec2 norm = {0.f};
  vec2_sub(norm, b.center, a.center);
  float dot = vec2_dot(norm, norm);
  float rad = a.radius + b.radius;
  return dot < rad * rad;
}

c_manifold c_aabb_vs_aabb_man(c_aabb a, c_aabb b) {
  vec2 mid_a, mid_b, eA, eB, d, tmp;

  vec2_add(tmp, a.min, a.max);
  vec2_scale(mid_a, tmp, 0.5f);

  vec2_add(tmp, b.min, b.max);
  vec2_scale(mid_b, tmp, 0.5f);

  vec2_sub(tmp, a.max, a.min);
  vec2_scale(tmp, tmp, 0.5f);
  vec2_abs(eA, tmp);

  vec2_sub(tmp, b.max, b.min);
  vec2_scale(tmp, tmp, 0.5f);
  vec2_abs(eB, tmp);

  vec2_sub(d, mid_b, mid_a);

  c_manifold man = (c_manifold){0};

  float dx = eA[0] + eB[0] - fabs(d[0]);
  if (dx < 0)
    return man;

  float dy = eA[1] + eB[1] - fabs(d[1]);
  if (dy < 0)
    return man;

  vec2  n;
  float depth;
  vec2  p;

  if (dx <= dy) {
    depth = dx + ASTERA_COL_SKIN_WIDTH;

    tmp[0] = eA[0];
    tmp[1] = 0.f;

    n[1] = 0.f;

    if (d[0] < 0) {
      n[0] = 1.f;

      vec2_sub(p, mid_a, tmp);
    } else {
      n[0] = -1.f;

      vec2_add(p, mid_a, tmp);
    }
  } else {
    depth = dy + ASTERA_COL_SKIN_WIDTH;

    n[0] = 0.f;
    n[1] = (d[1] < 0) ? 1.f : -1.f;

    tmp[0] = 0.f;
    tmp[1] = eA[1];

    if (d[1] < 0)
      vec2_sub(p, mid_a, tmp);
    else
      vec2_add(p, mid_a, tmp);
  }

  vec2_dup(man.point, p);
  vec2_dup(man.direction, n);
  man.distance = depth;

  return man;
}

c_manifold c_aabb_vs_circle_man(c_aabb a, c_circle b) {
  c_manifold man = (c_manifold){0};

  vec2 L, ab;
  vec2_clamp(L, b.center, a.min, a.max);
  vec2_sub(ab, L, b.center);
  float d2 = vec2_dot(ab, ab);
  float r2 = b.radius * b.radius;

  if (d2 < r2) {
    if (d2 != 0) {
      float d = sqrtf(d2);
      vec2  norm;
      vec2_norm(norm, ab);
      man.distance = b.radius - d;
      vec2_dup(man.direction, norm);
      vec2_scale(norm, norm, d);
      vec2_add(man.point, norm, b.center);
    } else {
      vec2 mid, e, d, abs_d, tmp;

      vec2_add(tmp, a.min, a.max);
      vec2_scale(mid, tmp, 0.5f);

      vec2_sub(tmp, a.max, a.min);
      vec2_scale(e, tmp, 0.5f);

      vec2_sub(d, b.center, mid);

      vec2_abs(abs_d, d);

      float x_overlap = e[0] - abs_d[0];
      float y_overlap = e[1] - abs_d[1];

      float depth = 0.f;

      if (x_overlap < y_overlap) {
        depth            = x_overlap;
        man.direction[0] = (d[0] < 0) ? -1.f : 1.f;
        man.direction[1] = 0.f;
      } else {
        depth            = y_overlap;
        man.direction[0] = 0.f;
        man.direction[1] = (d[1] < 0) ? -1.f : 1.f;
      }

      man.distance = depth;
      vec2_scale(tmp, man.direction, depth);
      vec2_sub(man.point, b.center, tmp);
    }
  }

  return man;
}

c_manifold c_circle_vs_circle_man(c_circle a, c_circle b) {
  vec2 d;
  vec2_sub(d, a.center, b.center);
  float d2 = vec2_dot(d, d);

  float r = a.radius + b.radius;

  c_manifold man = (c_manifold){0};

  if (d2 < r * r) {
    float l = sqrtf(d2);
    if (l != 0)
      vec2_scale(man.direction, d, 1.f / l);
    else {
      man.direction[0] = 0.f;
      man.direction[1] = 1.f;
    }

    vec2 tmp;
    vec2_scale(tmp, man.direction, b.radius);
    vec2_sub(man.point, b.center, tmp);

    man.distance = r - l;
  }

  return man;
}

c_manifold c_circle_vs_aabb_man(c_circle a, c_aabb b) {
  c_manifold man = (c_manifold){0};

  vec2 L, ab;
  vec2_clamp(L, a.center, b.min, b.max);
  vec2_sub(ab, L, a.center);
  float d2 = vec2_dot(ab, ab);
  float r2 = a.radius * a.radius;

  if (d2 < r2) {
    if (d2 != 0) {
      float d = sqrtf(d2);
      vec2  norm;
      vec2_norm(norm, ab);
      man.distance = a.radius - d;
      vec2_dup(man.direction, norm);
      vec2_scale(norm, norm, d);
      vec2_add(man.point, norm, a.center);
    } else {
      vec2 mid, e, d, abs_d, tmp;

      vec2_add(tmp, b.min, b.max);
      vec2_scale(mid, tmp, 0.5f);

      vec2_sub(tmp, b.max, b.min);
      vec2_scale(e, tmp, 0.5f);

      vec2_sub(d, a.center, mid);
      vec2_abs(abs_d, d);

      float x_overlap = e[0] - abs_d[0];
      float y_overlap = e[1] - abs_d[1];

      if (x_overlap < y_overlap) {
        man.distance     = x_overlap;
        man.direction[0] = (d[0] < 0 ? 1.f : -1.f);
        man.direction[1] = 0.f;
      } else {
        man.distance     = y_overlap;
        man.direction[0] = 0.f;
        man.direction[1] = (d[1] < 0 ? 1.f : -1.f);
      }

      vec2_scale(tmp, man.direction, man.distance);
      vec2_add(man.point, a.center, tmp);
    }
  }

  return man;
}

c_manifold c_test(c_shape a, c_shape b) {
  // zeroed out manifold for returning fail
  c_manifold manifold = (c_manifold){0};

  switch (a.type) {
    case C_AABB:
      switch (b.type) {
        case C_AABB:
          return c_aabb_vs_aabb_man(a.col.aabb, b.col.aabb);
        case C_CIRCLE:
          return c_aabb_vs_circle_man(a.col.aabb, b.col.circle);
        case C_RAY:
          return c_ray_vs_aabb_man(a.col.ray, b.col.aabb);
        case C_NONE:
          return manifold;
      }

      break;
    case C_CIRCLE:
      switch (b.type) {
        case C_AABB:
          return c_aabb_vs_circle_man(b.col.aabb, a.col.circle);
        case C_CIRCLE:
          return c_circle_vs_circle_man(b.col.circle, a.col.circle);
        case C_RAY:
          return c_ray_vs_circle_man(b.col.ray, a.col.circle);
        case C_NONE:
          return manifold;
      }
      break;
    case C_RAY:
      switch (b.type) {
        case C_AABB:
          return c_ray_vs_aabb_man(a.col.ray, b.col.aabb);
        case C_CIRCLE:
          return c_ray_vs_circle_man(a.col.ray, b.col.circle);
        case C_RAY:
          return manifold; // TODO ray vs ray
        case C_NONE:
          return manifold;
      }
      break;
    case C_NONE:
      return manifold;
  }

  return manifold;
}

c_aabb c_reduce(c_shape* shapes, uint32_t count) {
  c_aabb col = (c_aabb){0};

  for (uint32_t i = 0; i < count; ++i) {
    c_shape shape = shapes[i];
    switch (shapes[i].type) {
      case C_AABB:
        vec2_min(col.min, shape.col.aabb.min, col.min);
        vec2_max(col.max, shape.col.aabb.max, col.max);
        break;
      case C_CIRCLE: {
        vec2_min(col.min, shape.col.aabb.min, col.min);
        vec2_max(col.max, shape.col.aabb.max, col.max);
      } break;
      case C_NONE:
      case C_RAY: // todo, cause I'm lazy
      default: {
        // uhhh, it's nothing
      } break;
    }
  }

  return col;
}
