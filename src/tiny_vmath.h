/* Copyright (C) 2018 Yosshin(@yosshin4004) */


#ifndef _TINY_VMATH_H_
#define _TINY_VMATH_H_


void Vec4Copy(
	float vec4Dst[4],
	const float vec4Src[4]
);

void Vec4MulScalar(
	float vec4A[4],
	const float vec4B[4],
	float scalar
);

void Vec4MacScalar(
	float vec4A[4],
	const float vec4B[4],
	const float vec4C[4],
	float scalar
);

void Vec4Transform(
	float vec4A[4],
	const float mat4x4B[4][4],
	const float vec4C[4]
);

void Mat4x4Copy(
	float mat4x4Dst[4][4],
	const float mat4x4Src[4][4]
);

void Mat4x4SetUnit(
	float mat4x4[4][4]
);

void Mat4x4Mul(
	float mat4x4A[4][4],
	float mat4x4B[4][4],
	float mat4x4C[4][4]
);

void Mat4x4SetAffineRotX(
	float mat4x4[4][4],
	float xAng
);

void Mat4x4SetAffineRotY(
	float mat4x4[4][4],
	float yAng
);

void Mat4x4SetAffineRotZ(
	float mat4x4[4][4],
	float zAng
);


#endif
