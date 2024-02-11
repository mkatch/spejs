#pragma once

#include <vector>

#include "common.h"

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

// Describes an attribute of a shader program.
struct Attribute {
	const char *name;
	GLuint location;
	GLenum data_type;

protected:
	Attribute(const char *name, GLenum data_type);
};

#define _Attribute(suffix, gl_data_type)                                     \
	Attribute_##suffix : public Attribute {                                    \
		Attribute_##suffix(char const *name) : Attribute(name, gl_data_type) { } \
	}
#define _Attribute_structs(scalar_suffix, vec_prefix, gl_component_type) \
	struct _Attribute(scalar_suffix, gl_component_type);                   \
	struct _Attribute(vec_prefix##vec2, gl_component_type##_VEC2);         \
	struct _Attribute(vec_prefix##vec3, gl_component_type##_VEC3);         \
	struct _Attribute(vec_prefix##vec4, gl_component_type##_VEC4)

_Attribute_structs(float, , GL_FLOAT);
_Attribute_structs(int, i, GL_INT);
_Attribute_structs(uint, u, GL_UNSIGNED_INT);
_Attribute_structs(double, d, GL_DOUBLE);

#undef _Attribute_structs
#undef _Attribute

// Wraps an OpenGL shader program object.
struct Program {
	GLuint program_id;
	const char *name;
	const VertexShader *vertex_shader;
	const FragmentShader *fragment_shader;
	std::vector<const Attribute *> attributes;

protected:
// Convenience aliases, so that the declarations resemble GLSL code.
#define _Attribute_typedefs(scalar_type, vec_prefix)          \
	typedef Attribute_##scalar_type in_##scalar_type;           \
	typedef Attribute_##vec_prefix##vec2 in_##vec_prefix##vec2; \
	typedef Attribute_##vec_prefix##vec3 in_##vec_prefix##vec3; \
	typedef Attribute_##vec_prefix##vec4 in_##vec_prefix##vec4

	_Attribute_typedefs(float, );
	_Attribute_typedefs(int, i);
	_Attribute_typedefs(uint, u);
	_Attribute_typedefs(double, d);

#undef _Attribute_typedefs

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