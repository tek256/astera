#ifndef MATH_C
#define MATH_C

#include "math.h"

#if defined(_MSC_VER)
//Force M_PI definition on MS platforms
#define _USE_MATH_DEFINES
#endif

#include <math.h>

void vec2_zero(vec2* dst){
    dst->x = 0.f;
    dst->y = 0.f;
}

void vec2_add(vec2* dst, vec2 a, vec2 b){
    *dst = (vec2){a.x + b.x, a.y + b.y};
}

void vec2_sub(vec2* dst, vec2 a, vec2 b){
    *dst = (vec2){a.x - b.x, a.y - b.y};
}

void vec2_mul(vec2* dst, vec2 a, vec2 b){
    *dst = (vec2){a.x * b.x, a.y * b.y};
}

void vec2_scale(vec2* dst, vec2 a, float s){
    *dst = (vec2){a.x * s, a.y * s};
}

void vec2_div(vec2* dst, vec2 a, vec2 b){
    *dst = (vec2){a.x / b.x, a.y / b.y};
}

float vec2_mul_inner(vec2 a, vec2 b){
    float p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    return p;
}

float vec2_len(vec2 a){
    return (float)sqrt(vec2_mul_inner(a, a));
}

void  vec2_norm(vec2* dst, vec2 v){
    float k = 1/ vec2_len(v);
    vec2_scale(dst, v, k);
}

float vec2_dist(vec2 a, vec2 b){
    float x2 = a.x - b.x;
    float y2 = a.y - b.y;
    return (x2*x2)+(y2*y2);
}

void vec2_set(vec2* dst, vec2 src){
    dst->x = src.x;
    dst->y = src.y;
}

void vec2_setfv(vec2* dst, float v[], int len){
    if(len < 2){
        return;
    }
    dst->x = v[0];
    dst->y = v[1];
}

void vec2_setf(vec2* dst, float x, float y){
    dst->x = x;
    dst->y = y;
}

void vec2_setv(vec2* dst, float val, int index){
    dst->v[index] = val;
}

void vec2_set_array(vec2* dst[], float v[], int len, int* set){
    int dstv = 0;
    for(int i=0;i<len;i++){
        dst[dstv]->v[i%2] = v[i];
        dstv ++;
    }
    *set = dstv;
}

void vec2_set3v(vec2* dst, vec3 v){
    dst->x = v.x;
    dst->y = v.y;
}

void vec2_set4v(vec2* dst, vec4 v){
    dst->x = v.x;
    dst->y = v.y;
}

void vec2_reflect(vec2* dst, vec2 v, vec2 n){
    float p = 2.f * vec2_mul_inner(v, n);
    dst->x = v.x - p * n.x;
    dst->y = v.y - p * n.y;
}

void vec3_zero(vec3* vec){
    vec->x = 0.f;
    vec->y = 0.f;
}

void vec3_add(vec3* dst, vec3 a, vec3 b){
    *dst = (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

void vec3_sub(vec3* dst, vec3 a, vec3 b){
    *dst = (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

void vec3_mul(vec3* dst, vec3 a, vec3 b){
    *dst = (vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

void vec3_scale(vec3* dst, vec3 a, float s){
    *dst = (vec3){a.x * s, a.y * s, a.z * s};
}

void vec3_div(vec3* dst, vec3 a, vec3 b){
    *dst = (vec3){a.x / b.x, a.y / b.y, a.z / b.z};
}

float vec3_mul_inner(vec3 a, vec3 b){
    float p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    p += a.z * b.z;
    return p;
}

float vec3_len( vec3 a){
    return (float)sqrt(vec3_mul_inner(a, a));
}

void  vec3_norm(vec3* dst, vec3 v){
    float k = 1/ vec3_len(v);
    vec3_scale(dst, v, k);
}

float vec3_dist(vec3 a, vec3 b){
    float x2 = a.x - b.x;
    float y2 = a.y - b.y;
    float z2 = a.z - b.z;
    return (x2*x2) + (y2*y2) + (z2*z2);
}

void vec3_set(vec3* dst, vec3 src){
    dst->x = src.x;
    dst->y = src.y;
    dst->z = src.z;
}

void vec3_setf(vec3* dst, float x, float y, float z){
    dst->x = x;
    dst->y = y;
    dst->z = z;
}

void vec3_setfv(vec3* dst, float v[], int len){
    if(len < 3){
        return;
    }
    dst->x = v[0];
    dst->y = v[1];
    dst->z = v[2];
}

void vec3_set_array(vec3* dst[], float v[], int len, int* set){
    int dstv = 0;
    for(int i=0;i<len;i+=3){
        dst[dstv]->x = v[i];
        dst[dstv]->y = v[i+1];
        dst[dstv]->z = v[i+2];
        dstv ++;
    }
    *set = dstv;
}

void vec3_setv(vec3* dst, float val, int index){
    if(index == 0){
        dst->x = val;
    }else if(index == 1){
        dst->y = val;
    }else if(index == 2){
        dst->z = val;
    }
}

void vec3_setq(vec3* dst, quat q){
    dst->x = q.x;
    dst->y = q.y;
    dst->z = q.z;
}

void vec3_set2v(vec3* dst, vec2 v){
    dst->x = v.x;
    dst->y = v.y;
}

void vec3_set4v(vec3* dst, vec4 v){
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
}

float vec3_getv(vec3 src, int index){
    return src.v[index];
}

void vec3_mul_cross(vec3* dst, vec3 a, vec3 b){
    dst->x = a.y*b.z - a.z*b.y;
    dst->y = a.z*b.x - a.x*b.z;
    dst->z = a.x*b.y - a.y*b.x;
}

void vec3_reflect(vec3* dst, vec3 v, vec3 n){
    float p = 2.f * vec3_mul_inner(v, n);
    dst->x = v.x - p * n.x;
    dst->y = v.y - p * n.y;
    dst->z = v.z - p * n.z;
}

void vec4_zero(vec4* dst){
    dst->x = 0.f;
    dst->y = 0.f;
    dst->z = 0.f;
    dst->w = 0.f;
}

void vec4_add(vec4* dst, vec4 a, vec4 b){
    *dst = (vec4){a.x + b.x, a.y + b.y, a.z + b.z};
}

void vec4_sub(vec4* dst, vec4 a, vec4 b){
    *dst = (vec4){a.x - b.x, a.y - b.y, a.z - b.z};
}

void vec4_mul(vec4* dst, vec4 a, vec4 b){
    *dst = (vec4){a.x * b.x, a.y * b.y, a.z * b.z};
}

void vec4_scale(vec4* dst, vec4 a, float s){
    *dst = (vec4){a.x * s, a.y * s, a.z * s};
}

void vec4_div(vec4* dst, vec4 a, vec4 b){
    *dst = (vec4){a.x / b.x, a.y / b.y, a.z / b.z};
}

float vec4_mul_inner(vec4 a, vec4 b){
    float p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    p += a.z * b.z;
    return p;
}

float vec4_len(vec4 a){
    return (float)sqrt(vec4_mul_inner(a, a));
}

void  vec4_norm(vec4* dst, vec4 v){
    float k = 1/ vec4_len(v);
    vec4_scale(dst, v, k);
}

void vec4_set(vec4* dst, vec4 src){
    dst->x = src.x;
    dst->y = src.y;
    dst->z = src.z;
    dst->w = src.w;
}

void vec4_setf(vec4* dst, float x, float y, float z, float w){
    dst->x = x;
    dst->y = y;
    dst->z = z;
    dst->w = w;
}

void vec4_setfv(vec4* dst, float v[], int len){
    if(len < 3){
        return;
    }
    dst->x = v[0];
    dst->y = v[1];
    dst->z = v[2];
}

void vec4_set_array(vec4* dst[], float v[], int len, int* set){
    int dstv = 0;
    for(int i=0;i<len;i+=3){
        dst[dstv]->x = v[i];
        dst[dstv]->y = v[i+1];
        dst[dstv]->z = v[i+2];
        dstv ++;
    }
    *set = dstv;
}

void vec4_set3v(vec4* dst, vec3 v){
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
}

void vec4_set2v(vec4* dst, vec2 v){
    dst->x = v.x;
    dst->y = v.y;
}

void vec4_setv(vec4* dst, float v, int index){
    if(index == 0){
        dst->x = v;
    }else if(index == 1){
        dst->y = v;
    }else if(index == 2){
        dst->z = v;
    }else if(index == 3){
        dst->w = v;
    }
}

float vec4_dist(vec4 a, vec4 b){
    float x2 = a.x - b.x;
    float y2 = a.y - b.y;
    float z2 = a.z - b.z;
    return (x2*x2) + (y2*y2) + (z2*z2);
}

float vec4_getv(vec4 src, int index){
    if(index == 0){
        return src.x;
    }else if(index == 1){
        return src.y;
    }else if(index == 2){
        return src.z;
    }else if(index == 3){
        return src.w;
    }
    return 0.f;
}

void vec4_reflect(vec4* dst, vec4 v, vec4 n){
    float p = 2.f * vec4_mul_inner(v, n);
    dst->x = v.x - p * n.x;
    dst->y = v.y - p * n.y;
    dst->z = v.z - p * n.z;
}

void vec4_mul_cross(vec4* dst, vec4 a, vec4 b){
    dst->x = a.y * b.z - a.z * b.y;
    dst->y = a.z * b.x - a.x * b.z;
    dst->z = a.x * b.y - a.y * b.x;
    dst->w = 1.f;
}

void mat4_identity(mat4* dst){
    dst->v[0][0] = 1;
    dst->v[0][1] = 0;
    dst->v[0][2] = 0;
    dst->v[0][3] = 0;

    dst->v[1][0] = 0;
    dst->v[1][1] = 1;
    dst->v[1][2] = 0;
    dst->v[1][3] = 0;

    dst->v[2][0] = 0;
    dst->v[2][1] = 0;
    dst->v[2][2] = 1;
    dst->v[2][3] = 0;

    dst->v[3][0] = 0;
    dst->v[3][1] = 0;
    dst->v[3][2] = 0;
    dst->v[3][3] = 1;
}

void mat4_set(mat4* dst, mat4 src){
    dst->v[0][0] = src.m00;
    dst->v[0][1] = src.m01;
    dst->v[0][2] = src.m02;
    dst->v[0][3] = src.m03;

    dst->v[1][0] = src.m10;
    dst->v[1][1] = src.m11;
    dst->v[1][2] = src.m12;
    dst->v[1][3] = src.m13;

    dst->v[2][0] = src.m20;
    dst->v[2][1] = src.m21;
    dst->v[2][2] = src.m22;
    dst->v[2][3] = src.m23;

    dst->v[3][0] = src.m30;
    dst->v[3][1] = src.m31;
    dst->v[3][2] = src.m32;
    dst->v[3][3] = src.m33;
}

void mat4_row(vec4* r, mat4 m, int i){
    if(i == 0){
        r->x = m.m00;
        r->y = m.m01;
        r->z = m.m02;
        r->w = m.m03;
    }else if(i == 1){
        r->x = m.m10;
        r->y = m.m11;
        r->z = m.m12;
        r->w = m.m13;
    }else if(i == 2){
        r->x = m.m20;
        r->y = m.m21;
        r->z = m.m22;
        r->w = m.m23;
    }else if(i == 3){
        r->x = m.m30;
        r->y = m.m31;
        r->z = m.m32;
        r->w = m.m33;
    }
}

void mat4_column(vec4* r, mat4 m, int i){
    if(i == 0){
        r->x = m.m00;
        r->y = m.m10;
        r->z = m.m20;
        r->w = m.m30;
    }else if(i == 1){
        r->x = m.m01;
        r->y = m.m11;
        r->z = m.m21;
        r->w = m.m31;
    }else if(i == 2){
        r->x = m.m02;
        r->y = m.m12;
        r->z = m.m22;
        r->w = m.m32;;
    }else if(i == 3){
        r->x = m.m03;
        r->y = m.m13;
        r->z = m.m23;
        r->w = m.m33;
    }
}

void mat4_transpose(mat4* dst, mat4 src){
    dst->v[0][0] = src.m00;
    dst->v[0][1] = src.m10;
    dst->v[0][2] = src.m20;
    dst->v[0][3] = src.m30;

    dst->v[1][0] = src.m01;
    dst->v[1][1] = src.m11;
    dst->v[1][2] = src.m21;
    dst->v[1][3] = src.m31;

    dst->v[2][0] = src.m02;
    dst->v[2][1] = src.m12;
    dst->v[2][2] = src.m22;
    dst->v[2][3] = src.m32;

    dst->v[3][0] = src.m03;
    dst->v[3][1] = src.m13;
    dst->v[3][2] = src.m23;
    dst->v[3][3] = src.m33;
}

void mat4_add(mat4* dst, mat4 a, mat4 b){
    dst->v[0][0] = a.m00 + b.m00;
    dst->v[0][1] = a.m01 + b.m01;
    dst->v[0][2] = a.m02 + b.m02;
    dst->v[0][3] = a.m03 + b.m03;

    dst->v[1][0] = a.m10 + b.m10;
    dst->v[1][1] = a.m11 + b.m11;
    dst->v[1][2] = a.m12 + b.m12;
    dst->v[1][3] = a.m13 + b.m13;

    dst->v[2][0] = a.m20 + b.m20;
    dst->v[2][1] = a.m21 + b.m21;
    dst->v[2][2] = a.m22 + b.m22;
    dst->v[2][3] = a.m23 + b.m23;

    dst->v[3][0] = a.m30 + b.m30;
    dst->v[3][1] = a.m31 + b.m31;
    dst->v[3][2] = a.m32 + b.m32;
    dst->v[3][3] = a.m33 + b.m33;
}

void mat4_sub(mat4* dst, mat4 a, mat4 b){
    dst->v[0][0] = a.m00 - b.m00;
    dst->v[0][1] = a.m01 - b.m01;
    dst->v[0][2] = a.m02 - b.m02;
    dst->v[0][3] = a.m03 - b.m03;

    dst->v[1][0] = a.m10 - b.m10;
    dst->v[1][1] = a.m11 - b.m11;
    dst->v[1][2] = a.m12 - b.m12;
    dst->v[1][3] = a.m13 - b.m13;

    dst->v[2][0] = a.m20 - b.m20;
    dst->v[2][1] = a.m21 - b.m21;
    dst->v[2][2] = a.m22 - b.m22;
    dst->v[2][3] = a.m23 - b.m23;

    dst->v[3][0] = a.m30 - b.m30;
    dst->v[3][1] = a.m31 - b.m31;
    dst->v[3][2] = a.m32 - b.m32;
    dst->v[3][3] = a.m33 - b.m33;
}

void mat4_scale(mat4* dst, mat4 a, float s){
    dst->v[0][0] = a.m00 * s;
    dst->v[0][1] = a.m01 * s;
    dst->v[0][2] = a.m02 * s;
    dst->v[0][3] = a.m03 * s;

    dst->v[1][0] = a.m10 * s;
    dst->v[1][1] = a.m11 * s;
    dst->v[1][2] = a.m12 * s;
    dst->v[1][3] = a.m13 * s;

    dst->v[2][0] = a.m20 * s;
    dst->v[2][1] = a.m21 * s;
    dst->v[2][2] = a.m22 * s;
    dst->v[2][3] = a.m23 * s;

    dst->v[3][0] = a.m30 * s;
    dst->v[3][1] = a.m31 * s;
    dst->v[3][2] = a.m32 * s;
    dst->v[3][3] = a.m33 * s;
}

void mat4_scale_ansio(mat4* dst, mat4 a, float x, float y, float z){
    dst->v[0][0] = a.m00 * x;
    dst->v[0][1] = a.m01 * x;
    dst->v[0][2] = a.m02 * x;
    dst->v[0][3] = a.m03 * x;

    dst->v[1][0] = a.m10 * y;
    dst->v[1][1] = a.m11 * y;
    dst->v[1][2] = a.m12 * y;
    dst->v[1][3] = a.m13 * y;

    dst->v[2][0] = a.m20 * z;
    dst->v[2][1] = a.m21 * z;
    dst->v[2][2] = a.m22 * z;
    dst->v[2][3] = a.m23 * z;

    dst->v[3][0] = a.m30;
    dst->v[3][1] = a.m31;
    dst->v[3][2] = a.m32;
    dst->v[3][3] = a.m33;
}

void mat4_mul(mat4* dst, mat4 left, mat4 right){
    //insure accuracy of values for post-mul
    mat4 tmp;
    mat4_set(&tmp, *dst);
    int k, c, r;
    for(c=0;c<4;++c){
        for(r=0;r<4;r++){
            for(k=0;k<4;++k){
                tmp.v[c][r] += left.v[k][r] * right.v[c][k];
            }
        }
    }
    //if we don't mat4_set prior, some values can be inaccurate.
    mat4_set(dst, tmp);
}

void mat4_size_scale(mat4* dst, mat4 a, float x, float y, float z){
    dst->v[0][0] = a.v[0][0] * x;
    dst->v[0][1] = a.v[0][1] * x;
    dst->v[0][2] = a.v[0][2] * x;
    dst->v[0][3] = a.v[0][3];

    dst->v[1][0] = a.v[1][0] * y;
    dst->v[1][1] = a.v[1][1] * y;
    dst->v[1][2] = a.v[1][2] * y;
    dst->v[1][3] = a.v[1][3];

    dst->v[2][0] = a.v[2][0] * z;
    dst->v[2][1] = a.v[2][1] * z;
    dst->v[2][2] = a.v[2][2] * z;
    dst->v[2][3] = a.v[2][3];

    dst->v[3][0] = a.v[3][0];
    dst->v[3][1] = a.v[3][1];
    dst->v[3][2] = a.v[3][2];
    dst->v[3][3] = a.v[3][3];
}

void mat4_mul_vec4(vec4* dst, mat4 m, vec4 vec){
    vec4 tmp;

    tmp.x = m.v[0][0] * vec.x + m.v[1][0] * vec.y + m.v[2][0] * vec.z + m.v[3][0] * vec.w;
    tmp.y = m.v[0][1] * vec.x + m.v[1][1] * vec.y + m.v[2][1] * vec.z + m.v[3][1] * vec.w;
    tmp.z = m.v[0][2] * vec.x + m.v[1][2] * vec.y + m.v[2][2] * vec.z + m.v[3][2] * vec.w;
    tmp.w = m.v[0][3] * vec.x + m.v[1][3] * vec.y + m.v[2][3] * vec.z + m.v[3][3] * vec.w;

    vec4_set(dst, tmp);
}

void mat4_translate(mat4* dst, float x, float y, float z){
    // mat4_identity(dst);
    dst->v[3][0] += x;
    dst->v[3][1] += y;
    dst->v[3][2] += z;
}

void mat4_translate_in_place(mat4* dst, float x, float y, float z){
    vec4 t = {x, y, z, 0};
    vec4 r;
    int i;
    for(i = 0; i < 4; ++i){
        mat4_row(&r, *dst, i);
        float v = dst->v[3][i];
        v += vec4_mul_inner(r, t);
        dst->v[3][i] = v;
    }
}

void mat4_from_vec3_mul_outer(mat4* dst, vec3 a, vec3 b){
    int i, j;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            dst->v[i][j] = (i < 3 && j < 3) ? a.v[i] * b.v[i] : 0.f;
        }
    }
}

void mat4_rotate(mat4* dst, mat4 m, float x, float y, float z, float angle){
    float s = sinf(angle);
    float c = cosf(angle);
    vec3 u = {x, y, z};

    if(vec3_len(u) > 1e-4){
        mat4 T, C, S;
        vec3_norm(&u, u);
        mat4_from_vec3_mul_outer(&T, u, u);

        m.v[1][2] += u.x;
        m.v[2][1] += -u.x;
        m.v[2][0] += u.y;
        m.v[0][2] += -u.y;
        m.v[1][0] += u.z;
        m.v[0][1] += -u.z;

        mat4_scale(&S, S, s);

        mat4_identity(&C);
        mat4_sub(&C, C, T);

        mat4_scale(&C, C, c);

        mat4_add(&T, T, C);
        mat4_add(&T, T, S);

        T.m33 = 1.;
        mat4_mul(dst, m, T);
    }else {
        mat4_set(dst, m);
    }
}

void mat4_rotate_x(mat4* dst, float angle){
    float cosr = cosf(angle);
    float sinr = sinf(angle);

    dst->v[1][1] = cosr;
    dst->v[1][2] = -sinr;
    dst->v[2][1] = sinr;
    dst->v[2][2] = -cosr;
}

void mat4_rotate_y(mat4* dst, float angle){
    float c = cosf(angle);
    float s = sinf(angle);

    dst->v[0][0] = c;
    dst->v[0][2] = s;
    dst->v[2][0] = -s;
    dst->v[2][2] = -c;
}

void mat4_rotate_z(mat4* dst, float angle){
    float c = cosf(angle);
    float s = sinf(angle);

    dst->v[0][0] = c;
    dst->v[0][1] = -s;
    dst->v[1][0] = s;
    dst->v[1][1] = c;
}

void mat4_ortho(mat4* dst, float l, float r, float b, float t, float n, float f){
    dst->v[0][0] = 2.f / (r - l);
    dst->v[0][1] = dst->v[0][2] = dst->v[0][3] = 0.f;

    dst->v[1][1] = 2.f / (t - b);
    dst->v[1][0] = dst->v[1][2] = dst->v[1][3] = 0.f;

    dst->v[2][2] = -2.f / (f - n);
    dst->v[2][0] = dst->v[2][1] = dst->v[2][3] = 0.f;

    dst->v[3][0] = -(r + l) / (r - l);
    dst->v[3][1] = -(t + b) / (t - b);
    dst->v[3][2] = -(f + n) / (f - n);
    dst->v[3][3] = 1.f;
}

void mat4_frust(mat4* dst, double l, double r, double b, double t, double n, double f){
    float rl = (r - l);
    float tb = (t - b);
    float fn = (f - n);

    dst->v[0][0] = (n * 2.f) / rl;
    dst->v[0][1] = 0.f;
    dst->v[0][2] = 0.f;
    dst->v[0][3] = 0.f;

    dst->v[1][0] = 0.f;
    dst->v[1][1] = (n * 2.f) / tb;
    dst->v[1][2] = 0.f;
    dst->v[1][3] = 0.f;

    dst->v[2][0] = (r + l) / rl;
    dst->v[2][1] = (t + b) / tb;
    dst->v[2][2] = -(f * n * 2.0f) / fn;
    dst->v[2][3] = -1.f;

    dst->v[3][0] = 0.f;
    dst->v[3][1] = 0.f;
    dst->v[3][2] = -(f * n * 2.0f) / fn;
    dst->v[3][3] = 0.f;
}

void mat4_perspective(mat4* dst, float fov, float asp, float n, float f){
    const double a = 1.f / tan(fov / 2.f);

    dst->v[0][0] = a / asp;
    dst->v[0][1] = 0.f;
    dst->v[0][2] = 0.f;
    dst->v[0][3] = 0.f;

    dst->v[1][0] = 0.f;
    dst->v[1][1] = a;
    dst->v[1][2] = 0.f;
    dst->v[1][3] = 0.f;
    
    dst->v[2][0] = 0.f;
    dst->v[2][1] = 0.f;
    dst->v[2][2] = -((f + n) / (f - n));
    dst->v[2][3] = -1.f;

    dst->v[3][0] = 0.f;
    dst->v[3][1] = 0.f;
    dst->v[3][2] = -((2.f * f * n) / (f - n));
    dst->v[3][3] = 0.f;
}

void mat4_look_at(mat4* dst, vec3 eye, vec3 center, vec3 up){
    vec3 f, s, t;
    vec3_sub(&f, center, eye);
    vec3_norm(&f, f);

    vec3_mul_cross(&s, f, up);
    vec3_norm(&s, s);

    vec3_mul_cross(&t, s, f);

    dst->v[0][0] = s.x;
    dst->v[0][1] = t.x;
    dst->v[0][2] = -f.x;
    dst->v[0][3] = 0.f;

    dst->v[1][0] = s.y;
    dst->v[1][1] = t.x;
    dst->v[1][2] = -f.x;
    dst->v[1][3] = 0.f;

    dst->v[2][0] = s.z;
    dst->v[2][1] = t.z;
    dst->v[2][2] = -f.z;
    dst->v[2][3] = 0.f;

    dst->v[3][3] = 1.f;
    dst->v[3][0] = dst->v[3][1] = dst->v[3][2] = 0.0f;

    mat4_translate_in_place(dst, -eye.x, -eye.y, -eye.z);
}


void quat_set(quat* dst, quat q){
    dst->x = q.x;
    dst->y = q.y;
    dst->z = q.z;
    dst->w = q.w;
}

void quat_setf(quat* dst, float x, float y, float z, float w){
    dst->x = x;
    dst->y = y;
    dst->z = z;
    dst->w = w;
}

void quat_set2v(quat* dst, vec2 v){
    dst->x = v.x;
    dst->y = v.y;
}

void quat_set3v(quat* dst, vec3 v){
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
}

void quat_set4v(quat* dst, vec4 v){
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
    dst->w = v.w;
}

float quat_getv(quat q, int index){
    if(index == 0){
        return q.x;
    }else if(index == 1){
        return q.y;
    }else if(index == 2){
        return q.z;
    }else if(index == 3){
        return q.w;
    }
    return 0.f;
}

void quat_setv(quat* dst, float v, int index){
    if(index == 0){
        dst->x = v;
    }else if(index == 1){
        dst->y = v;
    }else if(index == 2){
        dst->z = v;
    }else if(index == 3){
        dst->w = v;
    }
}

void quat_identity(quat q){
    q.x = q.y = q.z = 0.f;
    q.z = 1.f;
}

void quat_add(quat* dst, quat a, quat b){
    dst->x = a.x + b.x;
    dst->y = a.y + b.y;
    dst->z = a.z + b.z;
    dst->w = a.w + b.w;
}

void quat_sub(quat* dst, quat a, quat b){
    dst->x = a.x - b.x;
    dst->y = a.y - b.y;
    dst->z = a.z - b.z;
    dst->w = a.w - b.w;
}

void quat_mul(quat* dst, quat p, quat q){
    vec3 w;
    vec3 tp, tq;
    vec3 r;

    vec3_setq(&tp, p);
    vec3_setq(&tq, q);

    vec3_mul_cross(&r, tp, tq);
    vec3_scale(&w, tp, tq.z);
    vec3_add(&r, r, w);
    vec3_scale(&w, tq, tp.z);
    vec3_add(&r, r, w);
    r.z = tp.z*tq.z - vec3_mul_inner(tp, tq);

    quat_set3v(dst, r);
}

void quat_scale(quat* dst, quat v, float s){
    dst->x = v.x * s;
    dst->y = v.y * s;
    dst->z = v.z * s;
    dst->w = v.w * s;
}

float quat_inner_product(quat a, quat b){
    float p = 0.f;
    p += a.x * b.x;
    p += a.y * b.y;
    p += a.z * b.z;
    p += a.w * b.w;
    return p;
}

void quat_conj(quat* dst, quat q){
    dst->x = -q.x;
    dst->y = -q.y;
    dst->z = -q.z;
    dst->w = q.w;
}

void quat_rotate(quat* dst, float angle, vec3 axis){
    vec3 v;
    vec3_scale(&v, axis, sinf(angle / 2));
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
    dst->w = cosf(angle / 2);
}

void quat_mul_vec3(vec3* dst, quat q, vec3 v){
    vec3 t;
    vec3 q_xyz, u;
    vec3_setq(&q_xyz, q);
    vec3_setq(&u, q);

    vec3_mul_cross(&t, q_xyz, v);
    vec3_scale(&t, t, 2);

    vec3_mul_cross(&u, q_xyz, t);
    vec3_scale(&t, t, q.w);

    vec3_add(dst, v, t);
    vec3_add(dst, *dst, u);
}

float flerpf(float a, float b, float t){
    float f = (b - a) / t;
    return a + f;
}

void mat4_from_quat(mat4* dst, quat q){
    float a = q.w;
    float b = q.x;
    float c = q.y;
    float d = q.z;
    float a2 = a*a;
    float b2 = b*b;
    float c2 = c*c;
    float d2 = d*d;

    dst->v[0][0] = a2 + b2 - c2 - d2;
    dst->v[0][1] = 2.f * (b*c + a*d);
    dst->v[0][2] = 2.f * (b*d - a*c);
    dst->v[0][3] = 0.f;

    dst->v[1][0] = 2 * (b*c - a*d);
    dst->v[1][1] = a2 - b2 + c2 - d2;
    dst->v[1][2] = 2.f * (c*d + a*b);
    dst->v[1][3] = 0.f;

    dst->v[2][0]  = 2.f * (b*d + a*c);
    dst->v[2][1] = 2.f * (c*d - a*b);
    dst->v[2][2] = a2 - b2 - c2 + d2;
    dst->v[2][3] = 0.f;
}

float truncateDeg(float deg){
    if(deg >= 360.0f){
        return (deg - 360.0f);
    }else{
        return deg;
    }
}

double deg2rad(float deg){
    return deg / 360 * (2 * M_PI);
}

double cosDeg(float deg){
    return cos(deg2rad(deg));
}

double sinDeg(float deg){
    return sin(deg2rad(deg));
}

#endif
