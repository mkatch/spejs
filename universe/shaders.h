#pragma once

#include <gl_cpp/gl.h>
#include <universe/shaders/shader_sources.h>

struct Shaders : public gl::Shaders {
	typedef ShaderSources Src;

	struct BasicProgram : gl::Program {
		in_vec4 position = {"position"};
		in_vec4 color = {"color"};

		BasicProgram()
				: Program("BasicProgram", Src::basic_v, Src::basic_f) { }
	};
	const BasicProgram basic_program;

	struct SolidProgram : gl::Program {
		uniform_mat4 Projection = {"Projection"};
		uniform_mat4 Model = {"Model"};
		uniform_mat3 Normal_model = {"_Normal_model"};
		uniform_vec4 color = {"color"};

		uniform_vec3 ambient_color = {"ambient_color"};
		uniform_vec3 light0_position = {"light0_position"};
		uniform_vec3 light0_color = {"light0_color"};
		uniform_vec3 light1_position = {"light1_position"};
		uniform_vec3 light1_color = {"light1_color"};

		in_vec3 position = {"position"};
		in_vec3 normal = {"normal"};

		SolidProgram()
				: Program("SolidProgram", Src::solid_v, Src::solid_f) { }
	};
	const SolidProgram solid_program;
};