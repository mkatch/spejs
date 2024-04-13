#include <glpp/gl.h>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/ext.hpp>
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
	glUseProgram(s.program_id);
	s.Projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	glm::mat4 m = glm::identity<glm::mat4>();
	m = glm::translate(m, {0, 0, -10});
	std::cout << glm::to_string(m) << std::endl;
	s.ambient_color = {0.2, 0.2, 0.2};
	s.light0_color = {0.9, 0.9, 0.3};
	s.light0_position = {-5, 5, 0};
	s.light1_color = {0.4, 0.4, 0.8};
	s.light1_position = {10, -10, -5};

	cube_vertices.bind([&](auto builder, auto base) {
		builder.enable_attribute(s.position, base->position);
		builder.enable_attribute(s.normal, base->normal);
	});

	glEnable(GL_CULL_FACE);

	while (!glfwWindowShouldClose(window) && rpc_server->is_running()) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gl_error_guard(glUseProgram(shaders.basic_program.program_id));
		glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUseProgram(s.program_id);
		s.color = {1.0f, 0.8f, 0.8f, 1.0f};
		glm::mat4 Model = m * glm::eulerAngleYXZ((float)glfwGetTime() * 2.0f, (float)glfwGetTime() * 3.0f, 0.0f);
		s.Model = Model;
		s.Normal_model = glm::inverseTranspose(glm::mat3(Model));

		glBindVertexArray(cube_vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, cube_vertices.vertex_count());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

gl::VertexBuffer<SolidVertex> create_cube_vertices() {
	std::vector<SolidVertex> vertices;
	auto push_face = [&](const glm::vec3 &p, const glm::vec3 &ux, const glm::vec3 &uy) {
		const glm::vec3 n = glm::cross(ux, uy);
		vertices.push_back({p, n});
		vertices.push_back({p + ux, n});
		vertices.push_back({p + ux + uy, n});
		vertices.push_back({p, n});
		vertices.push_back({p + ux + uy, n});
		vertices.push_back({p + uy, n});
	};
	const glm::vec3 v0 = {-1, -1, -1};
	const glm::vec3 v1 = {1, 1, 1};
	const glm::vec3 ux = {2, 0, 0};
	const glm::vec3 uy = {0, 2, 0};
	const glm::vec3 uz = {0, 0, 2};
	push_face(v0, +uz, +uy);
	push_face(v1, -uy, -uz);
	push_face(v0, +ux, +uz);
	push_face(v1, -uz, -ux);
	push_face(v0, +uy, +ux);
	push_face(v1, -ux, -uy);
	gl::VertexBuffer<SolidVertex> vertex_buffer;
	vertex_buffer.buffer_data(vertices.data(), vertices.size());
	return vertex_buffer;
}