#include <glpp/gl.h>
// GLFW must be after OpenGL

#include "rpc.h"
#include "shaders.h"
#include "ui.h"

#include <GLFW/glfw3.h>
UI::UI() {
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
	}
}

UI::~UI() {
	glfwTerminate();
}

struct BasicVertex {
	GLfloat position[2];
	GLfloat color[3];
};

void UI::event_loop(const RpcServer *rpc_server) {
	GLFWwindow *window = glfwCreateWindow(800, 600, "Universe server", nullptr, nullptr);
	if (!window) {
		throw std::exception("Failed to create GLFW window");
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize OpenGL API");
	}

	Shaders shaders;
	shaders.compile_all();
	auto &p = shaders.basic_program;

	glClearColor(0.5f, 0.5f, 0.1f, 0.0f);

	GLuint vertex_array;
	gl_error_guard(glCreateVertexArrays(1, &vertex_array));
	glBindVertexArray(vertex_array);

	const BasicVertex vertices[] = {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{ {0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{  {0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	};
	gl::VertexBuffer<BasicVertex> vertex_buffer;
	vertex_buffer.buffer_data(vertices);

	vertex_buffer.bind([&](auto builder, auto base) {
		builder.enable_attribute(p.position, base->position);
		builder.enable_attribute(p.color, base->color);
	});

	while (!glfwWindowShouldClose(window) && rpc_server->is_running()) {
		glClear(GL_COLOR_BUFFER_BIT);

		gl_error_guard(glUseProgram(shaders.basic_program.program_id));
		glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}