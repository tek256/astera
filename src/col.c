#include <astera/col.h>

#include <math.h>

#define _PI    3.141592654
#define RADPI  180 / _PI
#define DEGRAD _PI / 180

static float distpow(vec2 a, vec2 b) {
  return ((a[0] - b[0]) * (a[0] - b[0])) + ((a[1] - b[1]) * (a[1] - b[1]));
}

static float distsqrt(vec2 a, vec2 b) {
  return (float)(sqrt(distpow(a, b)));
}

static inline float _fabsf(float val) {
  return (val < 0) ? val * -1.f : val;
}

static inline float fclamp(float value, float min, float max) {
  return (value < min) ? min : (value > max) ? max : value;
}

c_aabb c_aabb_get(vec2 pos, vec2 size) {
  c_aabb a;
  vec2_dup(a.center, pos);
  vec2_dup(a.halfsize, size);
  return a;
}

c_circle c_circ_get(vec2 pos, float rad) {
  c_circle a;
  vec2_dup(a.center, pos);
  a.radius = rad;
  return a;
}

int c_aabb_vs_aabb(c_man* man, c_aabb a, c_aabb b) {
  vec2  n      = {a.center[0] - b.center[0], a.center[1] - b.center[1]};
  float x_over = a.halfsize[0] + b.halfsize[0] - _fabsf(n[0]);
  if (x_over > 0.f) {
    float y_over = a.halfsize[1] + b.halfsize[1] - _fabsf(n[1]);

    if (y_over > 0.f) {
      if (x_over > y_over) {
        if (man) {
          if (n[0] < 0.f) {
            man->normal[0] = -1.f;
            man->normal[1] = 0.f;
          } else {
            man->normal[0] = 0.f;
            man->normal[1] = 0.f;
          }
          man->pen = x_over;
        }

        return 1;
      } else {
        if (man) {
          if (n[1] < 0.f) {
            man->normal[0] = 0.f;
            man->normal[1] = -1.f;
          } else {
            man->normal[0] = 0.f;
            man->normal[1] = 1.f;
          }
          man->pen = y_over;
        }

        return 1;
      }
    }
  }

  return 0;
}

int c_aabb_vs_circ(c_man* man, c_aabb a, c_circle b) {
  vec2 n, closest;
  vec2_sub(n, a.center, b.center);
  vec2_dup(closest, n);

  closest[0] = fclamp(closest[0], -a.halfsize[0], a.halfsize[0]);
  closest[1] = fclamp(closest[1], -a.halfsize[1], a.halfsize[1]);

  int inside = 0;

  // This is surprisingly written out, I just realized I can just fucking take
  // _fabsf (my abs func)
  // I'm smart, I swear.
  if (vec2_cmp(closest, n)) {
    inside = 1;
    if (_fabsf(n[0]) > _fabsf(n[1])) {
      closest[0] = _fabsf(a.halfsize[0]);
    } else {
      if (closest[1] > 0) {
        closest[1] = _fabsf(a.halfsize[1]);
      }
    }
  }

  vec2 normal;
  vec2_sub(normal, n, closest);

  float lensq = vec2_lensq(normal);
  float rad   = b.radius;

  if (lensq > rad * rad && !inside) {
    return 0;
  }

  if (man) {
    if (inside) {
      man->normal[0] = normal[0] * -1.f;
      man->normal[1] = normal[1] * -1.f;
      man->pen       = rad - lensq;
    } else {
      vec2_dup(man->normal, normal);
      man->pen = rad - lensq;
    }
  }

  return 1;
}

int c_circ_vs_circ(c_man* man, c_circle a, c_circle b) {
  float rsq  = (a.radius + b.radius) * (a.radius + b.radius);
  float dist = (((a.center[0] - b.center[0]) * (a.center[0] - b.center[0])) +
                (a.center[1] - b.center[1]) * (a.center[1] - b.center[1]));

  if (rsq < dist) {
    if (man) {
      man->pen = dist - rsq;
      vec2 d   = {a.center[0] - b.center[0], a.center[1] - b.center[1]};
      vec2_norm(man->normal, d);
    }
    return 1;
  }
  return 0;
}
