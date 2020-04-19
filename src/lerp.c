#include <astera/lerp.h>

static float _clampf(float value, float min, float max) {
  return (value > max) ? max : (value < min) ? min : value;
}

static double _clampd(double value, double min, double max) {
  return (value > max) ? max : (value < min) ? min : value;
}

inline float l_lerp(float start, float end, float percent) {
  return start + percentage * (end - start);
}

void l_lerpv2(vec2 res, vec2 start, vec2 end, float percent) {
  res[0] = start[0] + percent * (end[0] - start[0]);
  res[1] = start[1] + percent * (end[1] - start[1]);
}

void l_lerpv3(vec3 res, vec3 start, vec3 end, float percent) {
  res[0] = start[0] + percent * (end[0] - start[0]);
  res[1] = start[1] + percent * (end[1] - start[1]);
  res[2] = start[2] + percent * (end[2] - start[2]);
}

void l_lerpv4(vec4 res, vec4 start, vec4 end, float percent) {
  res[0] = start[0] + percent * (end[0] - start[0]);
  res[1] = start[1] + percent * (end[1] - start[1]);
  res[2] = start[2] + percent * (end[2] - start[2]);
  res[3] = start[3] + percent * (end[3] - start[3]);
}

void l_nlerpv2(vec2 res, vec2 start, vec2 end, float percent) {
  l_lerpv2(res, start, end, percent);
  vec2_norm(res);
}

void l_nlerpv3(vec3 res, vec3 start, vec3 end, float percent) {
  l_lerpv3(res, start, end, percent);
  vec3_norm(res);
}

void l_nlerpv4(vec4 res, vec4 start, vec4 end, float percent) {
  l_lerpv4(res, start, end, percent);
  vec4_norm(res);
}

float smoothstepf(float min, float max, float value) {
  float t = _clampf((value - min) / (max - min), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}

double smoothstepd(double min, double max, double value) {
  double t = _clampd((value - min) / (max - min), 0.0, 1.0);
  return t * t * (3.0 - 2.0 * t);
}

