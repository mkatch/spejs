#pragma once

#include "common.h"

namespace gl {

// Represents a vector of 2, 3 or 4 elements of type T.
//
// To be used as a transfer data type when passing vectors to OpenGL functions.
// You should define conversions from your own vector types.
template <typename T, GLsizei size>
struct vec {
	const T *data;
};

typedef vec<GLfloat, 2> vec2;
typedef vec<GLfloat, 3> vec3;
typedef vec<GLfloat, 4> vec4;
typedef vec<GLint, 2> ivec2;
typedef vec<GLint, 3> ivec3;
typedef vec<GLint, 4> ivec4;
typedef vec<GLuint, 2> uvec2;
typedef vec<GLuint, 3> uvec3;
typedef vec<GLuint, 4> uvec4;

enum MatrixOrder {
	COLUMN_MAJOR,
	ROW_MAJOR,
};

// Represents a matrix with elements of type T.
//
// To be used as a transfer data type when passing matrices to OpenGL functions.
// You should define conversions from your own matrix types.
template <typename T, GLsizei columns, GLsizei rows, MatrixOrder order>
struct mat {
	const T *data;
};

// Represents a matrix with elements of type T in a column-major order.
//
// To be used as a transfer data type when passing matrices to OpenGL functions.
// You should define conversions from your own matrix types.
template <typename T, GLsizei columns, GLsizei rows>
using cmat = mat<T, columns, rows, COLUMN_MAJOR>;

// Represents a matrix with elements of type T in a row-major order.
//
// To be used as a transfer data type when passing matrices to OpenGL functions.
// You should define conversions from your own matrix types.
template <typename T, GLsizei rows, GLsizei columns>
using rmat = mat<T, columns, rows, ROW_MAJOR>;

typedef cmat<GLfloat, 2, 2> cmat2;
typedef cmat<GLfloat, 2, 3> cmat2x3;
typedef cmat<GLfloat, 2, 4> cmat2x4;
typedef cmat<GLfloat, 3, 2> cmat3x2;
typedef cmat<GLfloat, 3, 3> cmat3;
typedef cmat<GLfloat, 3, 4> cmat3x4;
typedef cmat<GLfloat, 4, 2> cmat4x2;
typedef cmat<GLfloat, 4, 3> cmat4x3;
typedef cmat<GLfloat, 4, 4> cmat4;

typedef rmat<GLfloat, 2, 2> rmat2;
typedef rmat<GLfloat, 2, 3> rmat2x3;
typedef rmat<GLfloat, 2, 4> rmat2x4;
typedef rmat<GLfloat, 3, 2> rmat3x2;
typedef rmat<GLfloat, 3, 3> rmat3;
typedef rmat<GLfloat, 3, 4> rmat3x4;
typedef rmat<GLfloat, 4, 2> rmat4x2;
typedef rmat<GLfloat, 4, 3> rmat4x3;
typedef rmat<GLfloat, 4, 4> rmat4;

}  // namespace gl