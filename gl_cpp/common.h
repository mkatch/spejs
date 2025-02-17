#pragma once

#include <common_cpp/common.h>
#include <glad/gl.h>

namespace gl {

// Returns a string representation of the given OpenGL enum.
const string &enum_string(GLenum value);

// Size of a type referenced by a GLenum, in bytes.
inline size_t size_of(GLenum type) {
	switch (type) {
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return sizeof(GLbyte);
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			return sizeof(GLshort);
		case GL_INT:
		case GL_UNSIGNED_INT:
			return sizeof(GLint);
		case GL_FLOAT:
			return sizeof(GLfloat);
		case GL_DOUBLE:
			return sizeof(GLdouble);
		default:
			assert(false);
			return 0;
	}
}

// An exception that is thrown when an error related to the usage of OpenGL
// occurs.
class exception : public std::runtime_error {
public:
	exception(string const &message);
	exception(string const &message, GLenum error_code);

	GLenum error_code() const { return _error_code; }
	string const &error_string() const { return enum_string(error_code()); }

private:
	GLenum _error_code = GL_NO_ERROR;
};

#define gl_if_error(call) \
	glGetError();           \
	call;                   \
	for (GLenum error = glGetError(); error != GL_NO_ERROR; error = GL_NO_ERROR)

#define gl_error_guard(call) \
	gl_if_error(call) throw ::gl::exception("Error during " #call, error);

}  // namespace gl