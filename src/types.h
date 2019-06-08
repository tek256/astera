#ifndef TYPES_H
#define TYPES_H

#define R_SHADER_NAME_SIZE 8

typedef union {
    float v[4];
    struct {
        float x, y, z, w;
    };
    struct {
        float r, g, b, a;
    };
} v4;

typedef union {
    float v[3];
    struct {
        float x, y, z;
    };
    struct {
        float r, g, b;
    };
} v3;

typedef union {
    float v[2];
    struct {
        float x, y;
    };
    struct {
        float s, t;
    };
} v2;

typedef union {
    float v[4][4];
    struct {
        float m00, m10, m20, m30;
        float m01, m11, m21, m31;
        float m02, m12, m22, m32;
        float m03, m13, m23, m33;
    };
} m4;

typedef union {
    float v[0];
    struct {
        float x, y, z, w;
    };
} quat;

#endif
