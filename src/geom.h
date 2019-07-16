#ifndef MATH_H
#define MATH_H
#include "types.h"

#define M_PRECISE
#ifdef M_PRECISE
	#define M_SCALAR double
#else
	#define M_SCALAR float
#endif

typedef union v4 {
    M_SCALAR v[4];
    struct {
        M_SCALAR x, y, z, w;
    };
    struct {
        M_SCALAR r, g, b, a;
    };
} v4;

typedef union v3 {
    M_SCALAR v[3];
    struct {
        M_SCALAR x, y, z;
    };
    struct {
        M_SCALAR r, g, b;
    };
} v3;

typedef union v2 {
    M_SCALAR v[2];
    struct {
        M_SCALAR x, y;
    };
    struct {
        M_SCALAR s, t;
    };
} v2;

typedef union m4 {
    M_SCALAR v[4][4];
    struct {
        M_SCALAR m00, m10, m20, m30;
        M_SCALAR m01, m11, m21, m31;
        M_SCALAR m02, m12, m22, m32;
        M_SCALAR m03, m13, m23, m33;
    };
} m4;

void     v2_zero(v2* dst);
void     v2_add(v2* dst, v2 a, v2 b);
void     v2_sub(v2* dst, v2 a, v2 b);
void     v2_mul(v2* dst, v2 a, v2 b);
void     v2_scale(v2* dst, v2 a, M_SCALAR s);
void     v2_div(v2* dst, v2 a, v2 b);
M_SCALAR v2_mul_inner(v2 a, v2 b);
M_SCALAR v2_len(v2 a);
void     v2_norm(v2* dst, v2 v);
M_SCALAR v2_dist(v2 a, v2 b);
void     v2_setv4(v2* dst, v3 v);
void     v2_reflect(v2* dst, v2 v, v2 n);

void     v3_zero(v3* vec);
void     v3_add(v3* dst, v3 a, v3 b);
void     v3_sub(v3* dst, v3 a, v3 b);
void     v3_mul(v3* dst, v3 a, v3 b);
void     v3_scale(v3* dst, v3 a, M_SCALAR s);
void     v3_div(v3* dst, v3 a, v3 b);
M_SCALAR v3_mul_inner(v3 a, v3 b);
M_SCALAR v3_len(v3 a);
void     v3_norm(v3* dst, v3 v);
M_SCALAR v3_dist(v3 a, v3 b);
void     v3_set(v3* dst, v3 src);
void     v3_setv2(v3* dst, v2 v);
void     v3_setv4(v3* dst, v4 v);
void     v3_mul_cross(v3* dst, v3 a, v3 b);
void     v3_reflect(v3* dst, v3 v, v3 n);
v3       v3_rot_x(v3 v, double a);
v3       v3_rot_y(v3 v, double a);
v3       v3_rot_z(v3 v, double a);
v3       v3_move_dir(v3 p, v3 rot, v3 amt);

void     v4_zero(v4* dst);
void     v4_add(v4* dst, v4 a, v4 b);
void     v4_sub(v4* dst, v4 a, v4 b);
void     v4_mul(v4* dst, v4 a, v4 b);
void     v4_scale(v4* dst, v4 a, M_SCALAR s);
void     v4_div(v4* dst, v4 a, v4 b);
M_SCALAR v4_mul_inner(v4 a, v4 b);
M_SCALAR v4_len(v4 a);
void     v4_norm(v4* dst, v4 v);
void     v4_set(v4* dst, v4 src);
void     v4_setv3(v4* dst, v3 v);
void     v4_setv2(v4* dst, v2 v);
M_SCALAR v4_dist(v4 a, v4 b);
void     v4_reflect(v4* dst, v4 v, v4 n);
void     v4_mul_cross(v4* dst, v4 a, v4 b);

void     m4_identity(m4* dst);
void     m4_set(m4* dst, m4 src);
void     m4_transpose(m4* dst, m4 src);
void     m4_add(m4* dst, m4 a, m4 b);
void     m4_sub(m4* dst, m4 a, m4 b);
void     m4_scale(m4* dst, m4 a, M_SCALAR s);
void     m4_scale_ansio(m4* dst, m4 a, M_SCALAR x, M_SCALAR y, M_SCALAR z);
void     m4_mul(m4* dst, m4 left, m4 right);
void     m4_size_scale(m4* dst, m4 a, M_SCALAR x, M_SCALAR y, M_SCALAR z);
void     m4_mul_v4(v4* dst, m4 m, v4 vec);
void     m4_translate(m4* dst, M_SCALAR x, M_SCALAR y, M_SCALAR z);
void     m4_translatev(m4* dst, v3 v);
void     m4_rotate(m4* dst, m4 m, M_SCALAR x, M_SCALAR y, M_SCALAR z, M_SCALAR angle);
void     m4_rotate_x(m4* dst, M_SCALAR angle);
void     m4_rotate_y(m4* dst, M_SCALAR angle);
void     m4_rotate_z(m4* dst, M_SCALAR angle);
void     m4_ortho(m4* dst, M_SCALAR l, M_SCALAR r, M_SCALAR b, M_SCALAR t, M_SCALAR n, M_SCALAR f);
int      m4_is_ortho(m4 a, M_SCALAR l, M_SCALAR r, M_SCALAR b, M_SCALAR t, M_SCALAR n, M_SCALAR f);


M_SCALAR lerpf(M_SCALAR a, M_SCALAR b, M_SCALAR t);
M_SCALAR trunc_deg(M_SCALAR deg);
M_SCALAR deg2rad(M_SCALAR deg);
M_SCALAR cos_deg(M_SCALAR deg);
M_SCALAR sin_deg(M_SCALAR deg);

int m4_is_ident(m4* m);

#endif
