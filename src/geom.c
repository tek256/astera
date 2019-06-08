#ifndef MATH_C
#define MATH_C

#include "geom.h"

#if defined(_MSC_VER)
//Force M_PI definition on MS platforms
#define _USE_MATH_DEFINES
#endif

#include <math.h>

void v2_zero(v2* dst){
    dst->x = 0.f;
    dst->y = 0.f;
}

void v2_add(v2* dst, v2 a, v2 b){
    *dst = (v2){a.x + b.x, a.y + b.y};
}

void v2_sub(v2* dst, v2 a, v2 b){
    *dst = (v2){a.x - b.x, a.y - b.y};
}

void v2_mul(v2* dst, v2 a, v2 b){
    *dst = (v2){a.x * b.x, a.y * b.y};
}

void v2_scale(v2* dst, v2 a, float s){
    *dst = (v2){a.x * s, a.y * s};
}

void v2_div(v2* dst, v2 a, v2 b){
    *dst = (v2){a.x / b.x, a.y / b.y};
}

float v2_mul_inner(v2 a, v2 b){
    float p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    return p;
}

float v2_len(v2 a){
    return (float)sqrt(v2_mul_inner(a, a));
}

void  v2_norm(v2* dst, v2 v){
    float k = 1/ v2_len(v);
    v2_scale(dst, v, k);
}

float v2_dist(v2 a, v2 b){
    float x2 = a.x - b.x;
    float y2 = a.y - b.y;
    return (x2*x2)+(y2*y2);
}

void v2_set(v2* dst, v2 src){
    dst->x = src.x;
    dst->y = src.y;
}

void v2_setfv(v2* dst, float v[], int len){
    if(len < 2){
        return;
    }
    dst->x = v[0];
    dst->y = v[1];
}

void v2_setf(v2* dst, float x, float y){
    dst->x = x;
    dst->y = y;
}

void v2_setv(v2* dst, float val, int index){
    dst->v[index] = val;
}

void v2_set_array(v2* dst[], float v[], int len, int* set){
    int dstv = 0;
    for(int i=0;i<len;i++){
        dst[dstv]->v[i%2] = v[i];
        dstv ++;
    }
    *set = dstv;
}

void v2_set3v(v2* dst, v3 v){
    dst->x = v.x;
    dst->y = v.y;
}

void v2_set4v(v2* dst, v4 v){
    dst->x = v.x;
    dst->y = v.y;
}

void v2_reflect(v2* dst, v2 v, v2 n){
    float p = 2.f * v2_mul_inner(v, n);
    dst->x = v.x - p * n.x;
    dst->y = v.y - p * n.y;
}

void v3_zero(v3* vec){
    vec->x = 0.f;
    vec->y = 0.f;
}

void v3_add(v3* dst, v3 a, v3 b){
    *dst = (v3){a.x + b.x, a.y + b.y, a.z + b.z};
}

void v3_sub(v3* dst, v3 a, v3 b){
    *dst = (v3){a.x - b.x, a.y - b.y, a.z - b.z};
}

void v3_mul(v3* dst, v3 a, v3 b){
    *dst = (v3){a.x * b.x, a.y * b.y, a.z * b.z};
}

void v3_scale(v3* dst, v3 a, float s){
    *dst = (v3){a.x * s, a.y * s, a.z * s};
}

void v3_div(v3* dst, v3 a, v3 b){
    *dst = (v3){a.x / b.x, a.y / b.y, a.z / b.z};
}

float v3_mul_inner(v3 a, v3 b){
    float p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    p += a.z * b.z;
    return p;
}

float v3_len( v3 a){
    return (float)sqrt(v3_mul_inner(a, a));
}

void  v3_norm(v3* dst, v3 v){
    float k = 1/ v3_len(v);
    v3_scale(dst, v, k);
}

float v3_dist(v3 a, v3 b){
    float x2 = a.x - b.x;
    float y2 = a.y - b.y;
    float z2 = a.z - b.z;
    return (x2*x2) + (y2*y2) + (z2*z2);
}

void v3_set(v3* dst, v3 src){
    dst->x = src.x;
    dst->y = src.y;
    dst->z = src.z;
}

void v3_setf(v3* dst, float x, float y, float z){
    dst->x = x;
    dst->y = y;
    dst->z = z;
}

void v3_setfv(v3* dst, float v[], int len){
    if(len < 3){
        return;
    }
    dst->x = v[0];
    dst->y = v[1];
    dst->z = v[2];
}

void v3_set_array(v3* dst[], float v[], int len, int* set){
    int dstv = 0;
    for(int i=0;i<len;i+=3){
        dst[dstv]->x = v[i];
        dst[dstv]->y = v[i+1];
        dst[dstv]->z = v[i+2];
        dstv ++;
    }
    *set = dstv;
}

void v3_setv(v3* dst, float val, int index){
    if(index == 0){
        dst->x = val;
    }else if(index == 1){
        dst->y = val;
    }else if(index == 2){
        dst->z = val;
    }
}

void v3_setq(v3* dst, quat q){
    dst->x = q.x;
    dst->y = q.y;
    dst->z = q.z;
}

void v3_set2v(v3* dst, v2 v){
    dst->x = v.x;
    dst->y = v.y;
}

void v3_set4v(v3* dst, v4 v){
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
}

float v3_getv(v3 src, int index){
    return src.v[index];
}

void v3_mul_cross(v3* dst, v3 a, v3 b){
    dst->x = a.y*b.z - a.z*b.y;
    dst->y = a.z*b.x - a.x*b.z;
    dst->z = a.x*b.y - a.y*b.x;
}

void v3_reflect(v3* dst, v3 v, v3 n){
    float p = 2.f * v3_mul_inner(v, n);
    dst->x = v.x - p * n.x;
    dst->y = v.y - p * n.y;
    dst->z = v.z - p * n.z;
}

void v4_zero(v4* dst){
    dst->x = 0.f;
    dst->y = 0.f;
    dst->z = 0.f;
    dst->w = 0.f;
}

void v4_add(v4* dst, v4 a, v4 b){
    *dst = (v4){a.x + b.x, a.y + b.y, a.z + b.z};
}

void v4_sub(v4* dst, v4 a, v4 b){
    *dst = (v4){a.x - b.x, a.y - b.y, a.z - b.z};
}

void v4_mul(v4* dst, v4 a, v4 b){
    *dst = (v4){a.x * b.x, a.y * b.y, a.z * b.z};
}

void v4_scale(v4* dst, v4 a, float s){
    *dst = (v4){a.x * s, a.y * s, a.z * s};
}

void v4_div(v4* dst, v4 a, v4 b){
    *dst = (v4){a.x / b.x, a.y / b.y, a.z / b.z};
}

float v4_mul_inner(v4 a, v4 b){
    float p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    p += a.z * b.z;
    return p;
}

float v4_len(v4 a){
    return (float)sqrt(v4_mul_inner(a, a));
}

void  v4_norm(v4* dst, v4 v){
    float k = 1/ v4_len(v);
    v4_scale(dst, v, k);
}

void v4_set(v4* dst, v4 src){
    dst->x = src.x;
    dst->y = src.y;
    dst->z = src.z;
    dst->w = src.w;
}

void v4_setf(v4* dst, float x, float y, float z, float w){
    dst->x = x;
    dst->y = y;
    dst->z = z;
    dst->w = w;
}

void v4_setfv(v4* dst, float v[], int len){
    if(len < 3){
        return;
    }
    dst->x = v[0];
    dst->y = v[1];
    dst->z = v[2];
}

void v4_set_array(v4* dst[], float v[], int len, int* set){
    int dstv = 0;
    for(int i=0;i<len;i+=3){
        dst[dstv]->x = v[i];
        dst[dstv]->y = v[i+1];
        dst[dstv]->z = v[i+2];
        dstv ++;
    }
    *set = dstv;
}

void v4_set3v(v4* dst, v3 v){
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
}

void v4_set2v(v4* dst, v2 v){
    dst->x = v.x;
    dst->y = v.y;
}

void v4_setv(v4* dst, float v, int index){
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

float v4_dist(v4 a, v4 b){
    float x2 = a.x - b.x;
    float y2 = a.y - b.y;
    float z2 = a.z - b.z;
    return (x2*x2) + (y2*y2) + (z2*z2);
}

float v4_getv(v4 src, int index){
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

void v4_reflect(v4* dst, v4 v, v4 n){
    float p = 2.f * v4_mul_inner(v, n);
    dst->x = v.x - p * n.x;
    dst->y = v.y - p * n.y;
    dst->z = v.z - p * n.z;
}

void v4_mul_cross(v4* dst, v4 a, v4 b){
    dst->x = a.y * b.z - a.z * b.y;
    dst->y = a.z * b.x - a.x * b.z;
    dst->z = a.x * b.y - a.y * b.x;
    dst->w = 1.f;
}

void m4_identity(m4* dst){
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

void m4_set(m4* dst, m4 src){
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

void m4_row(v4* r, m4 m, int i){
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

void m4_column(v4* r, m4 m, int i){
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

void m4_transpose(m4* dst, m4 src){
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

void m4_add(m4* dst, m4 a, m4 b){
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

void m4_sub(m4* dst, m4 a, m4 b){
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

void m4_scale(m4* dst, m4 a, float s){
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

void m4_scale_ansio(m4* dst, m4 a, float x, float y, float z){
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

void m4_mul(m4* dst, m4 left, m4 right){
    //insure accuracy of values for post-mul
    m4 tmp;
    m4_set(&tmp, *dst);
    int k, c, r;
    for(c=0;c<4;++c){
        for(r=0;r<4;r++){
            for(k=0;k<4;++k){
                tmp.v[c][r] += left.v[k][r] * right.v[c][k];
            }
        }
    }
    //if we don't m4_set prior, some values can be inaccurate.
    m4_set(dst, tmp);
}

void m4_size_scale(m4* dst, m4 a, float x, float y, float z){
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

void m4_mul_v4(v4* dst, m4 m, v4 vec){
    v4 tmp;

    tmp.x = m.v[0][0] * vec.x + m.v[1][0] * vec.y + m.v[2][0] * vec.z + m.v[3][0] * vec.w;
    tmp.y = m.v[0][1] * vec.x + m.v[1][1] * vec.y + m.v[2][1] * vec.z + m.v[3][1] * vec.w;
    tmp.z = m.v[0][2] * vec.x + m.v[1][2] * vec.y + m.v[2][2] * vec.z + m.v[3][2] * vec.w;
    tmp.w = m.v[0][3] * vec.x + m.v[1][3] * vec.y + m.v[2][3] * vec.z + m.v[3][3] * vec.w;

    v4_set(dst, tmp);
}

void m4_translate(m4* dst, float x, float y, float z){
    // m4_identity(dst);
    dst->v[3][0] += x;
    dst->v[3][1] += y;
    dst->v[3][2] += z;
}

void m4_translate_in_place(m4* dst, float x, float y, float z){
    v4 t = {x, y, z, 0};
    v4 r;
    int i;
    for(i = 0; i < 4; ++i){
        m4_row(&r, *dst, i);
        float v = dst->v[3][i];
        v += v4_mul_inner(r, t);
        dst->v[3][i] = v;
    }
}

void m4_from_v3_mul_outer(m4* dst, v3 a, v3 b){
    int i, j;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            dst->v[i][j] = (i < 3 && j < 3) ? a.v[i] * b.v[i] : 0.f;
        }
    }
}

void m4_rotate(m4* dst, m4 m, float x, float y, float z, float angle){
    float s = sinf(angle);
    float c = cosf(angle);
    v3 u = {x, y, z};

    if(v3_len(u) > 1e-4){
        m4 T, C, S;
        v3_norm(&u, u);
        m4_from_v3_mul_outer(&T, u, u);

        m.v[1][2] += u.x;
        m.v[2][1] += -u.x;
        m.v[2][0] += u.y;
        m.v[0][2] += -u.y;
        m.v[1][0] += u.z;
        m.v[0][1] += -u.z;

        m4_scale(&S, S, s);

        m4_identity(&C);
        m4_sub(&C, C, T);

        m4_scale(&C, C, c);

        m4_add(&T, T, C);
        m4_add(&T, T, S);

        T.m33 = 1.;
        m4_mul(dst, m, T);
    }else {
        m4_set(dst, m);
    }
}

void m4_rotate_x(m4* dst, float angle){
    float cosr = cosf(angle);
    float sinr = sinf(angle);

    dst->v[1][1] = cosr;
    dst->v[1][2] = -sinr;
    dst->v[2][1] = sinr;
    dst->v[2][2] = -cosr;
}

void m4_rotate_y(m4* dst, float angle){
    float c = cosf(angle);
    float s = sinf(angle);

    dst->v[0][0] = c;
    dst->v[0][2] = s;
    dst->v[2][0] = -s;
    dst->v[2][2] = -c;
}

void m4_rotate_z(m4* dst, float angle){
    float c = cosf(angle);
    float s = sinf(angle);

    dst->v[0][0] = c;
    dst->v[0][1] = -s;
    dst->v[1][0] = s;
    dst->v[1][1] = c;
}

void m4_ortho(m4* dst, float l, float r, float b, float t, float n, float f){
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

void m4_frust(m4* dst, double l, double r, double b, double t, double n, double f){
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

void m4_perspective(m4* dst, float fov, float asp, float n, float f){
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

void m4_look_at(m4* dst, v3 eye, v3 center, v3 up){
    v3 f, s, t;
    v3_sub(&f, center, eye);
    v3_norm(&f, f);

    v3_mul_cross(&s, f, up);
    v3_norm(&s, s);

    v3_mul_cross(&t, s, f);

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

    m4_translate_in_place(dst, -eye.x, -eye.y, -eye.z);
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

void quat_set2v(quat* dst, v2 v){
    dst->x = v.x;
    dst->y = v.y;
}

void quat_set3v(quat* dst, v3 v){
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
}

void quat_set4v(quat* dst, v4 v){
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
    v3 w;
    v3 tp, tq;
    v3 r;

    v3_setq(&tp, p);
    v3_setq(&tq, q);

    v3_mul_cross(&r, tp, tq);
    v3_scale(&w, tp, tq.z);
    v3_add(&r, r, w);
    v3_scale(&w, tq, tp.z);
    v3_add(&r, r, w);
    r.z = tp.z*tq.z - v3_mul_inner(tp, tq);

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

void quat_rotate(quat* dst, float angle, v3 axis){
    v3 v;
    v3_scale(&v, axis, sinf(angle / 2));
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
    dst->w = cosf(angle / 2);
}

void quat_mul_v3(v3* dst, quat q, v3 v){
    v3 t;
    v3 q_xyz, u;
    v3_setq(&q_xyz, q);
    v3_setq(&u, q);

    v3_mul_cross(&t, q_xyz, v);
    v3_scale(&t, t, 2);

    v3_mul_cross(&u, q_xyz, t);
    v3_scale(&t, t, q.w);

    v3_add(dst, v, t);
    v3_add(dst, *dst, u);
}

float flerpf(float a, float b, float t){
    float f = (b - a) / t;
    return a + f;
}

void m4_from_quat(m4* dst, quat q){
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
