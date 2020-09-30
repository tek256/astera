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
  c_aabb aabb = {0.f};
  vec2_sub(aabb.min, center, halfsize);
  vec2_add(aabb.max, center, halfsize);
  return aabb;
}

c_circle c_circle_create(vec2 center, float radius) {
  c_circle circle = (c_circle){.radius = radius};
  vec2_dup(circle.center, center);
  return circle;
}

void c_aabb_move(c_aabb* aabb, vec2 distance) {
  vec2_add(aabb->min, aabb->min, distance);
  vec2_add(aabb->max, aabb->max, distance);
}

void c_circle_move(c_circle* circle, vec2 distance) {
  vec2_add(circle->center, circle->center, distance);
}

void c_ray_move(c_ray* ray, vec2 distance) {
  vec2_add(ray->center, ray->center, distance);
}

void c_aabb_get_size(vec2 dst, c_aabb aabb) {
  vec2_sub(dst, aabb.max, aabb.min);
}

// signed distance point to plane one dimensional
static inline float _sdpo(float p, float n, float d) { return p * n - d * n; }

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

uint8_t c_ray_vs_aabb(c_ray A, c_aabb B, c_raycast* out) {
  vec2 p0, p1, tmp;

  vec2_scale(tmp, A.direction, A.distance);
  vec2_add(p0, tmp, A.center);

  c_aabb a_box;
  vec2_min(a_box.min, p0, p1);
  vec2_max(a_box.max, p0, p1);

  if (!c_aabb_vs_aabb(a_box, B))
    return 0;

  vec2 ab, n, abs_n, half, center_of_b;

  vec2_sub(ab, p1, p0);
  vec2_dup(tmp, ab);

  ab[0] = -tmp[1];
  ab[1] = tmp[0];

  vec2_abs(abs_n, n);

  vec2_sub(tmp, B.max, B.min);
  vec2_scale(half, tmp, 0.5f);

  vec2_add(tmp, B.max, B.min);
  vec2_scale(center_of_b, tmp, 0.5f);

  vec2_sub(tmp, p0, center_of_b);
  float d = fabs(vec2_dot(n, tmp) - vec2_dot(abs_n, half));

  if (d > 0)
    return 0;

  float da0 = _sdpo(p0[0], -1.f, B.min[0]);
  float db0 = _sdpo(p1[0], -1.f, B.min[0]);
  float da1 = _sdpo(p0[0], 1.f, B.max[0]);
  float db1 = _sdpo(p1[0], 1.f, B.max[0]);
  float da2 = _sdpo(p0[1], -1.f, B.min[1]);
  float db2 = _sdpo(p1[1], -1.f, B.min[1]);
  float da3 = _sdpo(p0[1], 1.f, B.max[1]);
  float db3 = _sdpo(p1[1], 1.f, B.max[1]);

  float t0 = _r2plane(da0, db0);
  float t1 = _r2plane(da1, db1);
  float t2 = _r2plane(da2, db2);
  float t3 = _r2plane(da3, db3);

  int hit0 = t0 < 1.f;
  int hit1 = t1 < 1.f;
  int hit2 = t2 < 1.f;
  int hit3 = t3 < 1.f;
  int hit  = hit0 | hit1 | hit2 | hit3;

  if (hit) {
    t0 = (float)hit0 * t0;
    t1 = (float)hit1 * t1;
    t2 = (float)hit2 * t2;
    t3 = (float)hit3 * t3;

    if (t0 >= t1 && t0 >= t2 && t0 >= t3) {
      out->distance  = t0 * A.distance;
      out->normal[0] = -1.f;
      out->normal[1] = 0.f;
    } else if (t1 >= t0 && t1 >= t2 && t1 >= t3) {
      out->distance  = t1 * A.distance;
      out->normal[0] = 1.f;
      out->normal[1] = 0.f;
    } else if (t2 >= t0 && t2 >= t1 && t2 >= t3) {
      out->distance  = t2 * A.distance;
      out->normal[0] = 0.f;
      out->normal[1] = -1.f;
    } else {
      out->distance  = t3 * A.distance;
      out->normal[0] = 0.f;
      out->normal[1] = 1.f;
    }

    // Get the point of impact
    vec2_scale(tmp, out->normal, out->distance);
    vec2_add(out->point, A.center, tmp);

    return 1;
  }

  return 0;
}

uint8_t c_ray_vs_circle(c_ray ray, c_circle B, c_raycast* out) {
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
  if (t >= 0 && t <= ray.distance && out) {
    // set the output distance (time)
    out->distance = t;
    vec2 impact, dist;

    // Calculate the impact point
    vec2_scale(dist, ray.direction, t);
    vec2_add(impact, dist, ray.center);
    vec2_dup(out->point, impact);

    // Calculate the normal
    vec2_sub(dist, impact, p);
    vec2_norm(impact, dist);
    vec2_dup(out->normal, impact);
  }

  return 1;
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
  int x1 = point[0] > a.max[2];
  int y0 = point[1] < a.min[1];
  int y1 = point[1] > a.max[3];
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

      float depth;

      if (x_overlap < y_overlap) {
        depth            = x_overlap;
        man.direction[0] = (d[0] < 0) ? -1.f : 1.f;
        man.direction[1] = 0.f;
      } else {
        depth            = y_overlap;
        man.direction[0] = 0.f;
        man.direction[1] = (d[1] < 0) ? -1.f : 1.f;
      }

      man.distance = b.radius + depth;
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

  c_manifold man = (c_manifold){0.f};

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

c_manifold c_circle_vs_aabb_man(c_circle a, c_aabb b){
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

      vec2 overlap;
      vec2_sub(overlap, e, abs_d);
      float len = vec2_len(overlap);

      printf("e: %.2f %.2f\n", e[0], e[1]);
      printf("abs_d: %.2f %.2f\n", abs_d[0], abs_d[1]);
      printf("overlap: %.2f %.2f, len: %f\n", overlap[0], overlap[1], len);

      // Calculate contact point
      vec2_sub(man.point, a.center, overlap);

      vec2_scale(man.direction, overlap, 1.f / len);
      man.distance = len + a.radius;

      // if (x_overlap < y_overlap) {
      //   depth            = x_overlap;
      //   man.direction[0] = (d[0] < 0) ? -1.f : 1.f;
      //   man.direction[1] = 0.f;
      // } else {
      //   depth            = y_overlap;
      //   man.direction[0] = 0.f;
      //   man.direction[1] = (d[1] < 0) ? -1.f : 1.f;
      // }

      // man.distance = a.radius + depth;
      // vec2_scale(tmp, man.direction, depth);
      // vec2_sub(man.point, b.center, tmp);
    }
  }

  return man;
}

c_manifold c_test(void* a, c_types a_type, void* b, c_types b_type) {
  // zeroed out manifold for returning fail
  c_manifold manifold = (c_manifold){0};

  // We don't want to test against nothing
  if (a_type == C_NONE || b_type == C_NONE) {
    return manifold;
  }

  if (!a || !b) {
    ASTERA_FUNC_DBG("invalid parameters passed.\n");
    return manifold;
  }

  switch (a_type) {
    case C_AABB:
      switch (b_type) {
        case C_AABB:
          return c_aabb_vs_aabb_man(*(c_aabb*)a, *(c_aabb*)a);
        case C_CIRCLE:
          return c_aabb_vs_circle_man(*(c_aabb*)a, *(c_circle*)b);
      }

      break;
    case C_CIRCLE:
      switch (b_type) {
        case C_AABB:
          return c_aabb_vs_circle_man(*(c_aabb*)b, *(c_circle*)a);
        case C_CIRCLE:
          return c_circle_vs_circle_man(*(c_circle*)b, *(c_circle*)a);
      }
      break;
  }

  return manifold;
}
