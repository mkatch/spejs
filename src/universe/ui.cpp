#include <glpp/gl.h>
#include <glm/glm.hpp>

#include "rpc.h"
#include "shaders.h"
#include "ui.h"

// GLFW must be after OpenGL
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
	glm::vec2 position;
	glm::vec3 color;
};

struct SolidVertex {
	glm::vec3 position;
	glm::vec3 normal;
};

gl::VertexBuffer<SolidVertex> create_cube_vertices();

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

	const gl::VertexBuffer<SolidVertex> cube_vertices = create_cube_vertices();
	GLuint cube_vertex_array;
	gl_error_guard(glCreateVertexArrays(1, &cube_vertex_array));
	glBindVertexArray(cube_vertex_array);

	const auto &s = shaders.solid_program;

	cube_vertices.bind([&](auto builder, auto base) {
		builder.enable_attribute(s.position, base->position);
		builder.enable_attribute(s.normal, base->normal);
	});

	while (!glfwWindowShouldClose(window) && rpc_server->is_running()) {
		glClear(GL_COLOR_BUFFER_BIT);

		gl_error_guard(glUseProgram(shaders.basic_program.program_id));
		glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUseProgram(s.program_id);
		s.color = {1.0f, 1.0f, 1.0f, 1.0f};
		s.projection = {
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1},
		};
		s.model = {
			{0.2,   0,   0, 0},
			{  0, 0.2,   0, 0},
			{  0,   0, 0.2, 0},
			{  0,   0,   0, 1},
		};

		glBindVertexArray(cube_vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, cube_vertices.vertex_count());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

gl::VertexBuffer<SolidVertex> create_cube_vertices() {
	std::vector<SolidVertex> vertices;
	auto tagv = [](int tag, glm::vec3 &v) -> void {
		v[0] = (tag & 4) ? 1.0f : -1.0f;
		v[1] = (tag & 2) ? 1.0f : -1.0f;
		v[2] = (tag & 1) ? 1.0f : -1.0f;
	};
	auto push_vertex = [&](int p, int n) -> void {
		SolidVertex v;
		tagv(p, v.position);
		tagv(n, v.normal);
		vertices.push_back(v);
	};
	auto push_face = [&](int p, int ux, int uy, int n) {
		push_vertex(p, n);
		push_vertex(p + ux, n);
		push_vertex(p + ux + uy, n);
		push_vertex(p, n);
		push_vertex(p + ux + uy, n);
		push_vertex(p + uy, n);
	};
	const int ux = 0b100, uy = 0b010, uz = 0b001;
	push_face(0b000, +uz, +uy, ~ux);
	push_face(0b111, -uz, -uy, +ux);
	push_face(0b000, +ux, +uz, ~uy);
	push_face(0b111, -ux, -uz, +uy);
	push_face(0b000, +uy, +ux, ~uz);
	push_face(0b111, -uy, -ux, +uz);
	gl::VertexBuffer<SolidVertex> vertex_buffer;
	vertex_buffer.buffer_data(vertices.data(), vertices.size());
	return vertex_buffer;
}