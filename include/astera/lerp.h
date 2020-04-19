#if !defined ASTERA_LERP_H
#define ASTERA_LERP_H

#include <astera/export.h>

ASTERA_API inline float l_lerp(float start, float end, float percent);

ASTERA_API void l_lerpv2(vec2 res, vec2 start, vec2 end, float percent);
ASTERA_API void l_lerpv3(vec3 res, vec3 start, vec3 end, float percent);
ASTERA_API void l_lerpv4(vec4 res, vec4 start, vec4 end, float percent);

ASTERA_API void l_nlerpv2(vec2 res, vec2 start, vec2 end, float percent);
ASTERA_API void l_nlerpv3(vec3 res, vec3 start, vec3 end, float percent);
ASTERA_API void l_nlerpv4(vec4 res, vec4 start, vec4 end, float percent);

ASTERA_API float  l_smoothstepf(float min, float max, float value);
ASTERA_API double l_smoothstepd(double min, double max, double value);

#endif
