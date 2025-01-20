#pragma once

#include "common.h"
#include "math.h"

namespace gl {

class TextureUnit {
	GLuint value;

public:
	explicit TextureUnit(GLuint value)
			: value(value) { }
	operator GLuint() const { return value; }
};

struct TextureImageFormat {
	const GLenum format;
	const GLenum type;

	constexpr TextureImageFormat(GLenum format, GLenum type)
			: format(format), type(type) { }
};

constexpr TextureImageFormat RED8 = {GL_RED, GL_UNSIGNED_BYTE};

class Texture {
	GLuint _texture_id = 0;

public:
	const GLenum target;
	const GLenum internal_format;

	GLuint texture_id() const { return _texture_id; }

	void bind(TextureUnit unit) const {
		assert_created();
		glBindTextureUnit(unit, _texture_id);
	}

	void assert_created() const {
		assert(_texture_id != 0);
	}

	void ensure_created() {
		if (_texture_id == 0) {
			gl_error_guard(glCreateTextures(target, 1, &_texture_id));
		}
	}

	void set(GLenum pname, GLint param) {
		glTextureParameteri(_texture_id, pname, param);
	}

protected:
	Texture(GLenum target, GLenum internal_format)
			: target(target), internal_format(internal_format) { }
	Texture(const Texture &) = delete;
	Texture(Texture &&other)
			: target(other.target), internal_format(other.internal_format) {
		_texture_id = other._texture_id;
		other._texture_id = 0;
	}
	virtual ~Texture() {
		if (_texture_id != 0) {
			glDeleteTextures(1, &_texture_id);
		}
	}
};

struct BufferView {
	const void *data;
	size_t size_bytes;

	BufferView(const BufferView &other) = delete;
	BufferView(BufferView &&other) = default;
	template <typename T>
	BufferView(const std::vector<T> &data)
			: data(data.data()), size_bytes(data.size() * sizeof(T)) { }
};

class Texture3D : public Texture {
	glm::uvec3 _size = glm::uvec3(0);
	int _levels = 1;

public:
	Texture3D(GLenum internal_format = GL_RGBA8)
			: Texture(GL_TEXTURE_3D, internal_format) { }

	void resize(int sizex, int sizey, int sizez, int levels = 1) {
		ensure_created();
		gl_error_guard(glTextureStorage3D(texture_id(), levels, internal_format, sizex, sizey, sizez));
		_size = glm::uvec3(sizex, sizey, sizez);
		_levels = levels;
	}

	const glm::uvec3 &size() const { return _size; }

	glm::uvec3 size(int level) const {
		return glm::uvec3(
				_size.x > 0 ? std::max(1u, _size.x >> level) : 0,
				_size.y > 0 ? std::max(1u, _size.y >> level) : 0,
				_size.z > 0 ? std::max(1u, _size.z >> level) : 0);
	}

	void sub_image(int level, int x, int y, int z, int sizex, int sizey, int sizez, TextureImageFormat format, BufferView data) {
		assert_created();
		assert(0 <= level && level < _levels);
		assert(data.size_bytes == sizex * sizey * sizez * gl::size_of(data.type));
		gl_error_guard(glTextureSubImage3D(texture_id(), level, x, y, z, sizex, sizey, sizez, format.format, format.type, data.data));
	}

	void image(int level, TextureImageFormat format, BufferView data) {
		glm::uvec3 s = size(level);
		sub_image(level, 0, 0, 0, s.x, s.y, s.z, format, std::move(data));
	}

	void image(TextureImageFormat format, BufferView data) {
		return image(0, format, std::move(data));
	}
};

}  // namespace gl