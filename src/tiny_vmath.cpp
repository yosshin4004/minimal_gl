/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <math.h>
#include "tiny_vmath.h"

void Vec4Copy(
	float vec4Dst[4],
	const float vec4Src[4]
){
	for (int i = 0; i < 4; ++i) {
		vec4Dst[i] = vec4Src[i];
	}
}

void Vec4MulScalar(
	float vec4A[4],
	const float vec4B[4],
	float scalar
){
	for (int i = 0; i < 4; ++i) {
		vec4A[i] = vec4B[i] * scalar;
	}
}

void Vec4MacScalar(
	float vec4A[4],
	const float vec4B[4],
	const float vec4C[4],
	float scalar
){
	for (int i = 0; i < 4; ++i) {
		vec4A[i] = vec4B[i] + vec4C[i] * scalar;
	}
}

void Vec4Transform(
	float vec4A[4],
	const float mat4x4B[4][4],
	const float vec4C[4]
){
	float vec4Tmp[4];
	Vec4MulScalar(vec4Tmp, mat4x4B[0], vec4C[0]);
	Vec4MacScalar(vec4Tmp, vec4Tmp, mat4x4B[1], vec4C[1]);
	Vec4MacScalar(vec4Tmp, vec4Tmp, mat4x4B[2], vec4C[2]);
	Vec4MacScalar(vec4Tmp, vec4Tmp, mat4x4B[3], vec4C[3]);
	Vec4Copy(vec4A, vec4Tmp);
}

void Mat4x4Copy(
	float mat4x4Dst[4][4],
	const float mat4x4Src[4][4]
){
	for (int i = 0; i < 4; ++i) {
		Vec4Copy(mat4x4Dst[i], mat4x4Src[i]);
	}
}

void Mat4x4SetUnit(
	float mat4x4[4][4]
){
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			mat4x4[i][j] = (i == j)? 1.0f : 0.0f;
		}
	}
}

void Mat4x4Mul(
	float mat4x4A[4][4],
	float mat4x4B[4][4],
	float mat4x4C[4][4]
){
	float mat4x4Tmp[4][4];
	for (int i = 0; i < 4; ++i) {
		Vec4Transform(mat4x4Tmp[i], mat4x4B, mat4x4C[i]);
	}
	Mat4x4Copy(mat4x4A, mat4x4Tmp);
}

void Mat4x4SetAffineRotX(
	float mat4x4[4][4],
	float xAng
){
	const float s = sinf(xAng);
	const float c = cosf(xAng);
	Mat4x4SetUnit(mat4x4);
	mat4x4[1][1] = c;
	mat4x4[1][2] = s;
	mat4x4[2][1] = -s;
	mat4x4[2][2] = c;
}

void Mat4x4SetAffineRotY(
	float mat4x4[4][4],
	float yAng
){
	const float s = sinf(yAng);
	const float c = cosf(yAng);
	Mat4x4SetUnit(mat4x4);
	mat4x4[0][0] = c;
	mat4x4[0][2] = -s;
	mat4x4[2][0] = s;
	mat4x4[2][2] = c;
}

void Mat4x4SetAffineRotZ(
	float mat4x4[4][4],
	float zAng
){
	const float s = sinf(zAng);
	const float c = cosf(zAng);
	Mat4x4SetUnit(mat4x4);
	mat4x4[0][0] = c;
	mat4x4[0][1] = s;
	mat4x4[1][0] = -s;
	mat4x4[1][1] = c;
}

