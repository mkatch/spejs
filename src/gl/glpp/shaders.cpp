#include "shaders.h"

#include <iostream>

namespace gl {

struct ShadersBuilder {
	static Shaders *shaders;

	static void push_program(Program *program, VertexShaderSource const *vertex_shader_source, FragmentShaderSource const *fragment_shader_source) {
		program->vertex_shader = get_shader(shaders->vertex_shaders, vertex_shader_source);
		program->fragment_shader = get_shader(shaders->fragment_shaders, fragment_shader_source);
		shaders->programs.push_back(program);
	}

	static Program *current_program() {
		return const_cast<Program *>(shaders->programs.back());
	}

	template <typename Shader, typename Source>
	static const Shader *get_shader(std::vector<Shader *> &shaders, const Source *source) {
		for (auto shader : shaders) {
			if (shader->source == source) {
				return shader;
			}
		}
		// This leaks. But nvm, since the Shader classes are supposed to be used as
		// singletons.
		return shaders.emplace_back(new Shader{source});
	}
};

Shaders *ShadersBuilder::shaders = nullptr;

Shaders::Shaders() {
	ShadersBuilder::shaders = this;
}

Uniform::Uniform(const char *name, const GLenum data_type)
		: name(name), data_type(data_type) {
	ShadersBuilder::current_program()->uniforms.push_back(this);
}

Attribute::Attribute(const char *name, const GLenum data_type)
		: name(name), data_type(data_type) {
	ShadersBuilder::current_program()->attributes.push_back(this);
}

Program::Program(const char *name, VertexShaderSource const &vertex_shader_source, FragmentShaderSource const &fragment_shader_source)
		: name(name) {
	ShadersBuilder::push_program(this, &vertex_shader_source, &fragment_shader_source);
}

GLuint compile_shader(const ShaderSource *source) {
	GLuint shader_id = glCreateShader(source->shader_type);
	if (shader_id == 0)
		throw gl::exception("Unable to create new shader", glGetError());

	const GLint source_size = strlen(source->source);
	glShaderSource(shader_id, 1, &source->source, &source_size);
	glCompileShader(shader_id);

	int compile_status;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE) {
		const int max_log_length = 255;
		char log[max_log_length + 1];
		glGetShaderInfoLog(shader_id, max_log_length, nullptr, (char *)&log);
		glDeleteShader(shader_id);
		throw gl::exception("Shader " + squote(source->name) + ", compilation error: " + log);
	}

	return shader_id;
}

GLuint link_program(const VertexShader *vertex_shader, const FragmentShader *fragment_shader) {
	GLuint program_id = glCreateProgram();
	if (program_id == 0)
		throw gl::exception("Unable to create new program");

	gl_if_error(
			glAttachShader(program_id, vertex_shader->shader_id);
			glAttachShader(program_id, fragment_shader->shader_id);) {
		glDeleteProgram(program_id);
		throw gl::exception("Unable to attach shaders " + squote(vertex_shader->source->name) + ", " + squote(fragment_shader->source->name) + ".", error);
	}

	glLinkProgram(program_id);
	GLint link_status;
	glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE) {
		const int max_log_length = 255;
		char log[max_log_length + 1];
		glGetProgramInfoLog(program_id, max_log_length, nullptr, log);
		glDeleteProgram(program_id);
		throw gl::exception("Unable to link shaders " + squote(vertex_shader->source->name) + ", " + squote(fragment_shader->source->name) + ": " + log);
	}

	return program_id;
}

void validate_uniforms(GLuint program_id, const char *program_name, const std::vector<const Uniform *> &uniforms) {
	GLint max_uniform_name_length = 0;
	glGetProgramiv(program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length);
	string uniform_name_storage((char)0, max_uniform_name_length + 1);
	char *uniform_name = uniform_name_storage.data();

	GLint active_uniform_count = 0;
	glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &active_uniform_count);

	for (int i = 0; i < active_uniform_count; ++i) {
		GLint size;
		GLenum data_type;
		glGetActiveUniform(program_id, i, uniform_name_storage.capacity(), nullptr, &size, &data_type, uniform_name);
		for (auto &uniform : uniforms) {
			if (strcmp(uniform->name, uniform_name) == 0) {
				if (uniform->data_type != data_type) {
					throw gl::exception("Uniform " + string(program_name) + "." + uniform->name + " has wrong data type; expected " + enum_string(data_type) + ", got " + enum_string(uniform->data_type) + ".");
				}
				break;
			}
		}
	}

	for (auto &uniform : uniforms) {
		if (uniform->location == -1) {
			if (uniform->name[0] == '_') {
				std::cout << "WARNING: Uniform " << squote(uniform->name) << " is not used in program " << program_name << std::endl;
				continue;
			}
			throw gl::exception("Uniform " + squote(uniform->name) + " not found in program " + string(program_name) + ".");
		}
	}
}

void validate_attributes(GLuint program_id, const char *program_name, const std::vector<const Attribute *> &attributes) {
	GLint max_attribute_name_length = 0;
	glGetProgramiv(program_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_attribute_name_length);
	string attribute_name_storage((char)0, max_attribute_name_length + 1);
	char *attribute_name = attribute_name_storage.data();

	GLint active_attribute_count = 0;
	glGetProgramiv(program_id, GL_ACTIVE_ATTRIBUTES, &active_attribute_count);

	for (int i = 0; i < active_attribute_count; ++i) {
		GLint size;
		GLenum data_type;
		glGetActiveAttrib(program_id, i, attribute_name_storage.capacity(), nullptr, &size, &data_type, attribute_name);
		for (auto &attribute : attributes) {
			if (strcmp(attribute->name, attribute_name) == 0) {
				if (attribute->data_type != data_type) {
					throw gl::exception("Attribute " + string(program_name) + "." + attribute->name + " has wrong data type; expected " + enum_string(data_type) + ", got " + enum_string(attribute->data_type) + ".");
				}
				break;
			}
		}
	}

	for (auto &attribute : attributes) {
		if (attribute->location == -1) {
			if (attribute->name[0] == '_') {
				std::cout << "WARNING: Attribute " << squote(attribute->name) << " is not used in program " << program_name << std::endl;
				continue;
			}
			throw gl::exception("Attribute " + squote(attribute->name) + " not found in program " + string(program_name) + ".");
		}
	}
}

void Shaders::compile_all() {
	for (auto vertex_shader : vertex_shaders) {
		vertex_shader->shader_id = compile_shader(vertex_shader->source);
	}
	for (auto fragment_shader : fragment_shaders) {
		fragment_shader->shader_id = compile_shader(fragment_shader->source);
	}
	for (auto program : programs) {
		program->program_id = link_program(program->vertex_shader, program->fragment_shader);
		for (auto &uniform : program->uniforms) {
			// const_cast is ok, because program owns the uniforms.
			const_cast<Uniform *>(uniform)->location = glGetUniformLocation(program->program_id, uniform->name);
		}
		validate_uniforms(program->program_id, program->name, program->uniforms);
		for (auto &attribute : program->attributes) {
			// const_cast is ok, because program owns the attributes.
			const_cast<Attribute *>(attribute)->location = glGetAttribLocation(program->program_id, attribute->name);
		}
		validate_attributes(program->program_id, program->name, program->attributes);
	}
}

}  // namespace gl