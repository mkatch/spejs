#pragma once

#include <functional>
#include "common.h"

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
#define _enable_attribute(suffix, cpp_component_type, gl_component_type, size)                          \
	void enable_attribute(const Attribute_##suffix &attribute, const cpp_component_type(&offset)[size]) { \
		enable_attribute(attribute.location, size, gl_component_type, offset);                              \
	}
#define _enable_attribute_overloads(scalar_suffix, vec_prefix, cpp_component_type, gl_component_type) \
	_enable_attribute(scalar_suffix, cpp_component_type, gl_component_type, 1);                         \
	_enable_attribute(vec_prefix##vec2, cpp_component_type, gl_component_type, 1);                      \
	_enable_attribute(vec_prefix##vec2, cpp_component_type, gl_component_type, 2);                      \
	_enable_attribute(vec_prefix##vec3, cpp_component_type, gl_component_type, 1);                      \
	_enable_attribute(vec_prefix##vec3, cpp_component_type, gl_component_type, 2);                      \
	_enable_attribute(vec_prefix##vec3, cpp_component_type, gl_component_type, 3);                      \
	_enable_attribute(vec_prefix##vec4, cpp_component_type, gl_component_type, 1);                      \
	_enable_attribute(vec_prefix##vec4, cpp_component_type, gl_component_type, 2);                      \
	_enable_attribute(vec_prefix##vec4, cpp_component_type, gl_component_type, 3);                      \
	_enable_attribute(vec_prefix##vec4, cpp_component_type, gl_component_type, 4);

	_enable_attribute_overloads(float, , GLfloat, GL_FLOAT);
	_enable_attribute_overloads(int, i, GLint, GL_INT);
	_enable_attribute_overloads(uint, u, GLuint, GL_UNSIGNED_INT);

#undef _enable_attribute_overloads
#undef _enable_attribute

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