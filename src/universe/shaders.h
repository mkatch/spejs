#pragma once

#include <glpp/gl.h>
#include <universe/shaders/shader_sources.h>

struct Shaders : public gl::Shaders {
	typedef ShaderSources Src;
	typedef gl::Attribute Attribute;

	struct BasicProgram : gl::Program {
		Attribute position = {"position"};
		Attribute color = {"color"};

		BasicProgram()
				: Program(Src::basic_v, Src::basic_f) {}
	};
	const BasicProgram basic_program;
};