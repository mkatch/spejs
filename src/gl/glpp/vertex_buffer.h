#pragma once

#include <functional>
#include <glm/glm.hpp>

#include "common.h"
#include "shaders.h"

namespace gl {

class VertexArrayBuilder;

// Wraps an OpenGL buffer object of type GL_ARRAY_BUFFER and offers
// functionality to help build vertex arrays with it.
template <typename T>
class VertexBuffer {
	GLuint _buffer_id;
	GLsizei _vertex_count;

public:
	VertexBuffer() {
		gl_error_guard(glCreateBuffers(1, &_buffer_id));
	}
	VertexBuffer(const VertexBuffer &) = delete;
	VertexBuffer(VertexBuffer &&other) {
		_buffer_id = other._buffer_id;
		other._buffer_id = 0;
	}
	~VertexBuffer() {
		glDeleteBuffers(1, &_buffer_id);
	}

	GLuint buffer_id() const { return _buffer_id; }

	GLsizei vertex_count() const { return _vertex_count; }

	void buffer_data(const T *data, GLsizei vertex_count, GLenum usage = GL_STATIC_DRAW) {
		_vertex_count = vertex_count;
		glNamedBufferData(_buffer_id, vertex_count * sizeof(T), data, usage);
	}

	// Uploads fixed-size array data to the buffer.
	template <size_t N>
	void buffer_data(const T (&data)[N], GLenum usage = GL_STATIC_DRAW) {
		buffer_data(data, sizeof(data) / sizeof(T), usage);
	}

	// Binds the buffer for building a vertex array.
	void bind(std::function<void(VertexArrayBuilder, const T *)> build) const {
		glBindBuffer(GL_ARRAY_BUFFER, _buffer_id);
		build(VertexArrayBuilder(sizeof(T)), nullptr);
	}
};

// Helper class to build vertex arrays in a type safe way.
//
// Use VertexBuffer::bind to get an instance of this class.
class VertexArrayBuilder {
	GLsizei stride;

public:
#define _enable_attribute_scalar(glsl_type, gl_component_type, cpp_component_type)                 \
	void enable_attribute(const Attribute_##glsl_type &attribute, const cpp_component_type &value) { \
		enable_attribute(attribute.location, 1, gl_component_type, &value);                            \
	}
#define _enable_attribute_vector(value_size, attribute_size, glsl_vec_prefix, gl_component_type)                                                 \
	void enable_attribute(const Attribute_##glsl_vec_prefix##vec##attribute_size &attribute, const glm::glsl_vec_prefix##vec##value_size &value) { \
		enable_attribute(attribute.location, value_size, gl_component_type, &value);                                                                 \
	}
#define _enable_attribute_overloads(glsl_component_type, glsl_vec_prefix, gl_component_type, cpp_component_type) \
	_enable_attribute_scalar(glsl_component_type, gl_component_type, cpp_component_type);                          \
	_enable_attribute_scalar(glsl_vec_prefix##vec2, gl_component_type, cpp_component_type);                        \
	_enable_attribute_scalar(glsl_vec_prefix##vec3, gl_component_type, cpp_component_type);                        \
	_enable_attribute_scalar(glsl_vec_prefix##vec4, gl_component_type, cpp_component_type);                        \
	_enable_attribute_vector(2, 2, glsl_vec_prefix, gl_component_type);                                            \
	_enable_attribute_vector(2, 3, glsl_vec_prefix, gl_component_type);                                            \
	_enable_attribute_vector(2, 4, glsl_vec_prefix, gl_component_type);                                            \
	_enable_attribute_vector(3, 3, glsl_vec_prefix, gl_component_type);                                            \
	_enable_attribute_vector(3, 4, glsl_vec_prefix, gl_component_type);                                            \
	_enable_attribute_vector(4, 4, glsl_vec_prefix, gl_component_type)

	_enable_attribute_overloads(float, , GL_FLOAT, GLfloat);
	_enable_attribute_overloads(int, i, GL_INT, GLint);
	_enable_attribute_overloads(uint, u, GL_UNSIGNED_INT, GLuint);

#undef _enable_attribute_overloads
#undef _enable_attribute_vector
#undef _enable_attribute_scalar

private:
	VertexArrayBuilder(GLsizei stride)
			: stride(stride) { }

	void enable_attribute(GLuint location, GLint component_count, GLenum component_type, const void *offset) {
		glVertexAttribPointer(location, component_count, component_type, /* normalized */ GL_FALSE, stride, offset);
		glEnableVertexAttribArray(location);
	}

	template <typename T>
	friend struct VertexBuffer;
};

}  // namespace gl