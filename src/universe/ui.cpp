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

	GLuint vertex_buffer;
	gl_error_guard(glCreateBuffers(1, &vertex_buffer));
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	const float vertices[] = {
			// position 0
			-0.5f,
			-0.5f,
			// color 0
			1.0f,
			0.0f,
			0.0f,
			// position 1
			0.5f,
			-0.5f,
			// color 1
			0.0f,
			1.0f,
			0.0f,
			// position 2
			0.0f,
			0.5f,
			// color 2
			0.0f,
			0.0f,
			1.0f,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	const GLsizei stride = 5 * sizeof(float);
	glVertexAttribPointer(p.position.location, 2, GL_FLOAT, GL_FALSE, stride, nullptr);
	glEnableVertexAttribArray(p.position.location);
	glVertexAttribPointer(p.color.location, 3, GL_FLOAT, GL_FALSE, stride, (void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(p.color.location);

	while (!glfwWindowShouldClose(window) && rpc_server->is_running()) {
		glClear(GL_COLOR_BUFFER_BIT);

		gl_error_guard(glUseProgram(shaders.basic_program.program_id));
		glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}