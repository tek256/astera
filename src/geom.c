#ifndef MATH_C
#define MATH_C

#include "geom.h"

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

void v2_zero(v2* dst){
    dst->x = 0;
    dst->y = 0;
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

void v2_scale(v2* dst, v2 a, M_SCALAR s){
    *dst = (v2){a.x * s, a.y * s};
}

void v2_div(v2* dst, v2 a, v2 b){
    *dst = (v2){a.x / b.x, a.y / b.y};
}

M_SCALAR v2_mul_inner(v2 a, v2 b){
    M_SCALAR p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    return p;
}

M_SCALAR v2_len(v2 a){
    return (M_SCALAR)sqrt(v2_mul_inner(a, a));
}

void  v2_norm(v2* dst, v2 v){
    M_SCALAR k = 1/ v2_len(v);
    v2_scale(dst, v, k);
}

M_SCALAR v2_dist(v2 a, v2 b){
    M_SCALAR x2 = a.x - b.x;
    M_SCALAR y2 = a.y - b.y;
    return (x2*x2)+(y2*y2);
}

void v2_reflect(v2* dst, v2 v, v2 n){
    M_SCALAR p = 2 * v2_mul_inner(v, n);
    dst->x = v.x - p * n.x;
    dst->y = v.y - p * n.y;
}

void v3_zero(v3* vec){
    vec->x = 0;
    vec->y = 0;
	vec->z = 0;
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

void v3_scale(v3* dst, v3 a, M_SCALAR s){
    *dst = (v3){a.x * s, a.y * s, a.z * s};
}

void v3_div(v3* dst, v3 a, v3 b){
    *dst = (v3){a.x / b.x, a.y / b.y, a.z / b.z};
}

M_SCALAR v3_mul_inner(v3 a, v3 b){
    M_SCALAR p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    p += a.z * b.z;
    return p;
}

M_SCALAR v3_len( v3 a){
    return (M_SCALAR)sqrt(v3_mul_inner(a, a));
}

void  v3_norm(v3* dst, v3 v){
    M_SCALAR k = 1/ v3_len(v);
    v3_scale(dst, v, k);
}

M_SCALAR v3_dist(v3 a, v3 b){
    M_SCALAR x2 = a.x - b.x;
    M_SCALAR y2 = a.y - b.y;
    M_SCALAR z2 = a.z - b.z;
    return (x2*x2) + (y2*y2) + (z2*z2);
}

void v3_set(v3* dst, v3 src){
    dst->x = src.x;
    dst->y = src.y;
    dst->z = src.z;
}

void v3_setf(v3* dst, M_SCALAR x, M_SCALAR y, M_SCALAR z){
    dst->x = x;
    dst->y = y;
    dst->z = z;
}

void v3_setv2(v3* dst, v2 v){
    dst->x = v.x;
    dst->y = v.y;
}

void v3_setv4(v3* dst, v4 v){
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
}

void v3_mul_cross(v3* dst, v3 a, v3 b){
    dst->x = a.y*b.z - a.z*b.y;
    dst->y = a.z*b.x - a.x*b.z;
    dst->z = a.x*b.y - a.y*b.x;
}

void v3_reflect(v3* dst, v3 v, v3 n){
    M_SCALAR p = 2 * v3_mul_inner(v, n);
    dst->x = v.x - p * n.x;
    dst->y = v.y - p * n.y;
    dst->z = v.z - p * n.z;
}

void v4_zero(v4* dst){
    dst->x = 0;
    dst->y = 0;
    dst->z = 0;
    dst->w = 0;
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

void v4_scale(v4* dst, v4 a, M_SCALAR s){
    *dst = (v4){a.x * s, a.y * s, a.z * s};
}

void v4_div(v4* dst, v4 a, v4 b){
    *dst = (v4){a.x / b.x, a.y / b.y, a.z / b.z};
}

M_SCALAR v4_mul_inner(v4 a, v4 b){
    M_SCALAR p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    p += a.z * b.z;
    return p;
}

M_SCALAR v4_len(v4 a){
    return (M_SCALAR)sqrt(v4_mul_inner(a, a));
}

void  v4_norm(v4* dst, v4 v){
    M_SCALAR k = 1/ v4_len(v);
    v4_scale(dst, v, k);
}

void v4_set(v4* dst, v4 src){
    dst->x = src.x;
    dst->y = src.y;
    dst->z = src.z;
    dst->w = src.w;
}

void v4_setf(v4* dst, M_SCALAR x, M_SCALAR y, M_SCALAR z, M_SCALAR w){
    dst->x = x;
    dst->y = y;
    dst->z = z;
    dst->w = w;
}

void v4_setv3(v4* dst, v3 v){
    dst->x = v.x;
    dst->y = v.y;
    dst->z = v.z;
}

void v4_setv2(v4* dst, v2 v){
    dst->x = v.x;
    dst->y = v.y;
}

M_SCALAR v4_dist(v4 a, v4 b){
    M_SCALAR x2 = a.x - b.x;
    M_SCALAR y2 = a.y - b.y;
    M_SCALAR z2 = a.z - b.z;
    return (x2*x2) + (y2*y2) + (z2*z2);
}

void v4_reflect(v4* dst, v4 v, v4 n){
    M_SCALAR p = 2 * v4_mul_inner(v, n);
    dst->x = v.x - p * n.x;
    dst->y = v.y - p * n.y;
    dst->z = v.z - p * n.z;
}

void v4_mul_cross(v4* dst, v4 a, v4 b){
    dst->x = a.y * b.z - a.z * b.y;
    dst->y = a.z * b.x - a.x * b.z;
    dst->z = a.x * b.y - a.y * b.x;
    dst->w = 1;
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
	for(int i=0;i<4;++i){
		for(int j=0;j<4;++j){
			dst->v[i][j] = src.v[i][j];
		}
	}
}

void m4_row(v4* r, m4 m, int i){
	r->v[0] = m.v[i][0];
	r->v[1] = m.v[i][1];
	r->v[2] = m.v[i][2];
	r->v[3] = m.v[i][3];
}

void m4_column(v4* r, m4 m, int i){
	r->v[0] = m.v[0][i];
	r->v[1] = m.v[1][i];
	r->v[2] = m.v[2][i];
	r->v[3] = m.v[3][i];
}

void m4_transpose(m4* dst, m4 src){
	for(int i=0;i<4;++i){
		for(int j=0;j<4;++j){
			dst->v[i][j] = src.v[j][i];
		}
	}
}

void m4_add(m4* dst, m4 a, m4 b){
	for(int i=0;i<4;++i){
		for(int j=0;j<4;++j){
			dst->v[i][j] = a.v[i][j] + b.v[i][j];
		}
	}
}

void m4_sub(m4* dst, m4 a, m4 b){
	for(int i=0;i<4;++i){
		for(int j=0;j<4;++j){
			dst->v[i][j] = a.v[i][j] - b.v[i][j];
		}
	}
}

void m4_scale(m4* dst, m4 a, M_SCALAR s){
	for(int i=0;i<4;++i){
		for(int j=0;j<4;++j){
			dst->v[i][j] = a.v[i][j] * s;
		}
	}
}

void m4_scale_ansio(m4* dst, m4 a, M_SCALAR x, M_SCALAR y, M_SCALAR z){
	for(int i=0;i<4;++i){
		for(int j=0;j<4;++j){
			M_SCALAR scale;

			if(i == 0){
				scale = x;
			}else if(i == 1){
				scale = y;
			}else if(i == 2){
				scale = z;
			}else if(i == 3){
				scale = 1.0;
			}

			dst->v[i][j] = a.v[i][j] * scale;
		}
	}
}

void m4_mul(m4* dst, m4 left, m4 right){
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

void m4_size_scale(m4* dst, m4 a, M_SCALAR x, M_SCALAR y, M_SCALAR z){
	for(int i=0;i<4;++i){
		for(int j=0;j<4;++j){
			M_SCALAR s;
			
			if(i == 0){
				s = x;
			}else if(i == 1){
				s = y;
			}else if(i == 2){
				s = z;
			}else if(i == 3){
				s = 1.0;
			}	

			dst->v[i][j] = a.v[i][j] * s;
		}
	}
}

void m4_mul_v4(v4* dst, m4 m, v4 vec){
    v4 tmp;

    tmp.x = m.v[0][0] * vec.x + m.v[1][0] * vec.y + m.v[2][0] * vec.z + m.v[3][0] * vec.w;
    tmp.y = m.v[0][1] * vec.x + m.v[1][1] * vec.y + m.v[2][1] * vec.z + m.v[3][1] * vec.w;
    tmp.z = m.v[0][2] * vec.x + m.v[1][2] * vec.y + m.v[2][2] * vec.z + m.v[3][2] * vec.w;
    tmp.w = m.v[0][3] * vec.x + m.v[1][3] * vec.y + m.v[2][3] * vec.z + m.v[3][3] * vec.w;

    v4_set(dst, tmp);
}

void m4_translate(m4* dst, M_SCALAR x, M_SCALAR y, M_SCALAR z){
    // m4_identity(dst);
    dst->v[3][0] += x;
    dst->v[3][1] += y;
    dst->v[3][2] += z;
}

void m4_translatev3(m4* dst, v3 v){
	dst->v[3][0] += v.x;
	dst->v[3][1] += v.y;
	dst->v[3][2] += v.z;
}

void m4_translate_in_place(m4* dst, M_SCALAR x, M_SCALAR y, M_SCALAR z){
    v4 t = {x, y, z, 0};
    v4 r;
    int i;
    for(i = 0; i < 4; ++i){
        m4_row(&r, *dst, i);
        M_SCALAR v = dst->v[3][i];
        v += v4_mul_inner(r, t);
        dst->v[3][i] = v;
    }
}

void m4_from_v3_mul_outer(m4* dst, v3 a, v3 b){
    int i, j;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            dst->v[i][j] = (i < 3 && j < 3) ? a.v[i] * b.v[i] : 0;
        }
    }
}

void m4_rotate(m4* dst, m4 m, M_SCALAR x, M_SCALAR y, M_SCALAR z, M_SCALAR angle){
    M_SCALAR s = sinf(angle);
    M_SCALAR c = cosf(angle);
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

void m4_rotate_x(m4* dst, M_SCALAR angle){
    M_SCALAR cosr = cosf(angle);
    M_SCALAR sinr = sinf(angle);

    dst->v[1][1] = cosr;
    dst->v[1][2] = -sinr;
    dst->v[2][1] = sinr;
    dst->v[2][2] = -cosr;
}

void m4_rotate_y(m4* dst, M_SCALAR angle){
    M_SCALAR c = cosf(angle);
    M_SCALAR s = sinf(angle);

    dst->v[0][0] = c;
    dst->v[0][2] = s;
    dst->v[2][0] = -s;
    dst->v[2][2] = -c;
}

void m4_rotate_z(m4* dst, M_SCALAR angle){
    M_SCALAR c = cosf(angle);
    M_SCALAR s = sinf(angle);

    dst->v[0][0] = c;
    dst->v[0][1] = -s;
    dst->v[1][0] = s;
    dst->v[1][1] = c;
}

void m4_ortho(m4* dst, M_SCALAR l, M_SCALAR r, M_SCALAR b, M_SCALAR t, M_SCALAR n, M_SCALAR f){
	m4_identity(dst);

	dst->v[0][0] =  2.0 / (r - l);
	dst->v[1][1] =  2.0 / (t - b);
   	dst->v[2][2] = -2.0 / (f - n);

	dst->v[3][0] = -(r+l)/(r-l);
	dst->v[3][1] = -(t+b)/(t-b);
	dst->v[3][2] = -(f+n)/(f-n);
	dst->v[3][3] = 1.0;
}

int m4_is_ortho(m4 a, M_SCALAR l, M_SCALAR r, M_SCALAR b, M_SCALAR t, M_SCALAR n, M_SCALAR f){
	if(a.v[0][0] !=  2.0 / (r - l)){
		return 0;
	}

	if(a.v[1][1] !=  2.0 / (t - b)){
		return 0;
	}

   	if(a.v[2][2] != -2.0 / (f - n)){
		return 0;
	}

	if(a.v[3][0] != -(r+l)/(r-l)){
		return 0;
	}

	if(a.v[3][1] != -(t+b)/(t-b)){
		return 0;
	}

	if(a.v[3][2] != -(f+n)/(f-n)){
		return 0;
	}

	if(a.v[3][3] != 1.0){
		return 0;
	}

	return 1;
}

M_SCALAR flerpf(M_SCALAR a, M_SCALAR b, M_SCALAR t){
    M_SCALAR f = (b - a) / t;
    return a + f;
}

M_SCALAR truncateDeg(M_SCALAR deg){
    if(deg >= 360.0f){
        return (deg - 360.0f);
    }else{
        return deg;
    }
}

M_SCALAR deg2rad(M_SCALAR deg){
    return deg / 360 * (2 * M_PI);
}

M_SCALAR cosDeg(M_SCALAR deg){
    return cos(deg2rad(deg));
}

M_SCALAR sinDeg(M_SCALAR deg){
    return sin(deg2rad(deg));
}

int m4_is_ident(m4* m){
	for(int i=0;i<4;++i){
		for(int j=0;j<4;++j){
			if(i == j){
				if(m->v[i][j] != 1.0){
					return 0;
				}
			}else if(m->v[i][j] != 0.0){
				return 0; 
			}
		}
	}

	return 1;
}

#endif
