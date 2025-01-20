#pragma once

#include <vector>

#include "common.h"
#include "math.h"
#include "texture.h"

namespace gl {

// Contains shader source code as a string and additional metadata.
//
// Don't instantiate by hand. Use shader_bundler to generate instance of this
// class from .glsl files.
struct ShaderSource {
	GLenum shader_type;
	const char *name;
	const char *file;
	const char *source;
};

struct VertexShaderSource : public ShaderSource { };
struct FragmentShaderSource : public ShaderSource { };

// Wraps an OpenGL shader object of type GL_VERTEX_SHADER.
struct VertexShader {
	const VertexShaderSource *source;
	GLuint shader_id;
};

// Wraps an OpenGL shader object of type GL_FRAGMENT_SHADER.
struct FragmentShader {
	const FragmentShaderSource *source;
	GLuint shader_id;
};

// Describes a uniform of a shader program.
struct Uniform {
	GLuint location;
	const char *name;
	GLenum type;

protected:
	Uniform(const char *name, GLenum type);
};

// Describes an attribute of a shader program.
struct Attribute {
	GLuint location;
	const char *name;
	GLenum type;

protected:
	Attribute(const char *name, GLenum type);
};

#define _Attribute(glsl_type, gl_type)                                     \
	Attribute_##glsl_type : public Attribute {                               \
		Attribute_##glsl_type(char const *name) : Attribute(name, gl_type) { } \
	}
#define _scalar_Uniform(glsl_type, gl_type, gl_setter_infix, cpp_component_type) \
	Uniform_##glsl_type : public Uniform {                                         \
		Uniform_##glsl_type(char const *name) : Uniform(name, gl_type) { }           \
		void operator=(cpp_component_type value) const {                             \
			glUniform1##gl_setter_infix(location, value);                              \
		}                                                                            \
	}
#define _vector_Uniform(component_count, glsl_type, gl_type, gl_setter_infix, cpp_component_type)  \
	Uniform_##glsl_type : public Uniform {                                                           \
		Uniform_##glsl_type(char const *name) : Uniform(name, gl_type) { }                             \
		void operator=(const glm::vec##component_count &u) const {                                     \
			glUniform##component_count##gl_setter_infix##v(location, 1, (const cpp_component_type *)&u); \
		}                                                                                              \
	}
#define _matrix_Uniform(csize, rsize)                                             \
	Uniform_mat##csize : public Uniform {                                           \
		Uniform_mat##csize(char const *name) : Uniform(name, GL_FLOAT_MAT##csize) { } \
		void operator=(const glm::mat##csize &M) const {                              \
			glUniformMatrix##csize##fv(location, 1, GL_FALSE, (const GLfloat *)&M);     \
		}                                                                             \
	}
#define _vector_Attributes_Uniforms(glsl_component_type, glsl_vec_prefix, gl_component_type, gl_setter_infix, cpp_component_type) \
	struct _Attribute(glsl_component_type, gl_component_type);                                                                      \
	struct _Attribute(glsl_vec_prefix##vec2, gl_component_type##_VEC2);                                                             \
	struct _Attribute(glsl_vec_prefix##vec3, gl_component_type##_VEC3);                                                             \
	struct _Attribute(glsl_vec_prefix##vec4, gl_component_type##_VEC4);                                                             \
	struct _scalar_Uniform(glsl_component_type, gl_component_type, gl_setter_infix, cpp_component_type);                            \
	struct _vector_Uniform(2, glsl_vec_prefix##vec2, gl_component_type##_VEC2, gl_setter_infix, cpp_component_type);                \
	struct _vector_Uniform(3, glsl_vec_prefix##vec3, gl_component_type##_VEC3, gl_setter_infix, cpp_component_type);                \
	struct _vector_Uniform(4, glsl_vec_prefix##vec4, gl_component_type##_VEC4, gl_setter_infix, cpp_component_type)

_vector_Attributes_Uniforms(float, , GL_FLOAT, f, GLfloat);
_vector_Attributes_Uniforms(int, i, GL_INT, i, GLint);
_vector_Attributes_Uniforms(uint, u, GL_UNSIGNED_INT, ui, GLuint);

struct _matrix_Uniform(2, 2);
struct _matrix_Uniform(2x3, 3x2);
struct _matrix_Uniform(2x4, 2x4);
struct _matrix_Uniform(3, 3);
struct _matrix_Uniform(3x2, 2x3);
struct _matrix_Uniform(3x4, 4x3);
struct _matrix_Uniform(4, 4);
struct _matrix_Uniform(4x2, 4x2);
struct _matrix_Uniform(4x3, 3x4);

#undef _vector_Attributes_Uniforms
#undef _matrix_Uniform
#undef _vector_Uniform
#undef _scalar_Uniform
#undef _Attribute

struct Uniform_sampler3D : public Uniform {
	Uniform_sampler3D(char const *name)
			: Uniform(name, GL_SAMPLER_3D) { }
	TextureUnit operator=(TextureUnit unit) const {
		glUniform1ui(location, unit);
		return unit;
	}
};

// Wraps an OpenGL shader program object.
struct Program {
	GLuint program_id;
	const char *name;
	const VertexShader *vertex_shader;
	const FragmentShader *fragment_shader;
	std::vector<const Uniform *> uniforms;
	std::vector<const Attribute *> attributes;

protected:
// Convenience aliases, so that the declarations resemble GLSL code.
#define _Attribute_typedef(suffix) \
	typedef Attribute_##suffix in_##suffix
#define _Uniform_typedef(suffix) \
	typedef Uniform_##suffix uniform_##suffix
#define _Attribute_Uniform_typedef(suffix) \
	_Attribute_typedef(suffix);              \
	_Uniform_typedef(suffix)
#define _vector_Attribute_Uniform_typedefs(scalar_type, vec_prefix) \
	_Attribute_Uniform_typedef(scalar_type);                          \
	_Attribute_Uniform_typedef(vec_prefix##vec2);                     \
	_Attribute_Uniform_typedef(vec_prefix##vec3);                     \
	_Attribute_Uniform_typedef(vec_prefix##vec4)
#define _matrix_Uniform_typedef(size) \
	_Uniform_typedef(mat##size)

	_vector_Attribute_Uniform_typedefs(float, );
	_vector_Attribute_Uniform_typedefs(int, i);
	_vector_Attribute_Uniform_typedefs(uint, u);

	_matrix_Uniform_typedef(2);
	_matrix_Uniform_typedef(3);
	_matrix_Uniform_typedef(4);
	_matrix_Uniform_typedef(2x3);
	_matrix_Uniform_typedef(3x2);
	_matrix_Uniform_typedef(2x4);
	_matrix_Uniform_typedef(4x2);
	_matrix_Uniform_typedef(3x4);
	_matrix_Uniform_typedef(4x3);

#undef _matrix_Uniform_typedef
#undef _vector_Attribute_Uniform_typedefs
#undef _Attribtue_Uniform_typedef
#undef _Uniform_typedef
#undef _Attribute_typedef

	typedef Uniform_sampler3D uniform_sampler3D;

	Program(const char *name, VertexShaderSource const &vertex_shader_source, FragmentShaderSource const &fragment_shader_source);
};

// Base class for declaring shader interfaces.
//
// The implementation of this class relies on C++ initialization order. To
// declare shaders for your application, one must extend this class and follow
// a specific pattern.
//
// class MyShaders : public gl::Shaders {
//   struct MyProgram : gl::Program {
//     gl::Attribute position = {"position"};
//     gl::Attribute position = {"color"};
//     MyProgram() : Program(my_vert, my_frag) {}
//   };
//   const MyProgram my_program;
//
//   struct MyOtherProgram : gl::Program {
//     ...
//   };
//   const MyOtherProgram my_other_program;
// };
//
// Upon instantiation, the Shaders base class registers a static global metadata
// builder that gathers structure information as it is being initialized. The
// fields are then initialized in a DFS-order, registering themselves into that
// builder. The gl::Program adds itself to the list of programs, each
// gl::Attribute adds itself to the list of attributes of the current program,
// and so on.
//
// If done correctly, you can do this:
//
//   MyShaders shaders;
//   shaders.compile_all();
//   shaders.my_program.program_id; // <- will be filled
//   shaders.my_program.position.location; // <- will be filled
//   // etc.
//
// As for the shader sources, use the shader_bundler to generate those from
// .glsl files on disk. Don't write the ShaderSource instances by hand.
class Shaders {
	std::vector<Program *> programs;
	std::vector<VertexShader *> vertex_shaders;
	std::vector<FragmentShader *> fragment_shaders;

public:
	// Compile all declared programs.
	void compile_all();

	friend class ShadersBuilder;

	// TODO: Utils for glValidateProgram?

protected:
	Shaders();
};

}  // namespace gl