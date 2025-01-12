#include <cstdlib>

#include <qoi.h>

#include "rpc.h"
#include "shaders.h"
#include "task.h"
#include "ui.h"

// GLFW must be after OpenGL
#include <GLFW/glfw3.h>

glm::vec3 light0_offset = {0, 0, -1};
glm::vec3 light1_offset = {5, -5, -5};

UI::UI(TaskQueue &tasks)
		: tasks(tasks) {
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
	}
}

UI::~UI() {
	if (skybox_pixels) {
		delete[] skybox_pixels;
	}
	glfwTerminate();
}

struct BasicVertex {
	glm::vec2 position;
	glm::vec3 color;
};

UI *ui(GLFWwindow *window) {
	return static_cast<UI *>(glfwGetWindowUserPointer(window));
}

float randf(float min, float max) {
	float t = (float)std::rand() / RAND_MAX;
	return (1 - t) * min + t * max;
}

float randf() {
	return randf(0, 1);
}

float randsgn() {
	return std::rand() % 2 == 0 ? -1.0f : 1.0f;
}

void UI::event_loop(const RpcServer *rpc_server) {
	if (window) {
		throw std::runtime_error("Event loop already running");
	}

	const int width = 1600;
	const int height = 1200;
	const float aspect = (float)width / (float)height;
	window = glfwCreateWindow(width, height, "Universe server", nullptr, nullptr);
	if (!window) {
		throw std::exception("Failed to create GLFW window");
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize OpenGL API");
	}

	shaders.compile_all();
	auto &p = shaders.basic_program;

	glClearColor(0.9f, 0.9f, 0.7f, 0.0f);

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

	create_cube_vertices(cube_vertices);
	gl_error_guard(glCreateVertexArrays(1, &cube_vertex_array));
	glBindVertexArray(cube_vertex_array);

	const auto &s = shaders.solid_program;
	glUseProgram(s.program_id);
	glm::mat4 m = glm::identity<glm::mat4>();
	m = glm::translate(m, {0, 0, -10});
	std::cout << glm::to_string(m) << std::endl;
	s.ambient_color = {0.2, 0.2, 0.2};
	s.light0_color = {0.9, 0.9, 0.3};
	s.light0_position = light0_offset;
	s.light1_color = {0.4, 0.4, 0.8};
	s.light1_position = light1_offset;

	cube_vertices.bind([&](auto builder, auto base) {
		builder.enable_attribute(s.position, base->position);
		builder.enable_attribute(s.normal, base->normal);
	});

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
		ui(window)->on_key(key, scancode, action, mods);
	});

	int skybox_size = 512;
	gl_error_guard(glCreateFramebuffers(1, &skybox_framebuffer));
	GLuint skybox_renderbuffers[2];
	gl_error_guard(glCreateRenderbuffers(2, skybox_renderbuffers));
	GLuint skybox_color_renderbuffer = skybox_renderbuffers[0];
	gl_error_guard(glNamedRenderbufferStorage(skybox_color_renderbuffer, GL_RGB8, skybox_size, skybox_size));
	GLuint skybox_depth_renderbuffer = skybox_renderbuffers[1];
	gl_error_guard(glNamedRenderbufferStorage(skybox_depth_renderbuffer, GL_DEPTH_COMPONENT24, skybox_size, skybox_size));
	gl_error_guard(glNamedFramebufferRenderbuffer(skybox_framebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, skybox_color_renderbuffer));
	gl_error_guard(glNamedFramebufferRenderbuffer(skybox_framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, skybox_depth_renderbuffer));
	GLenum status = glCheckNamedFramebufferStatus(skybox_framebuffer, GL_FRAMEBUFFER);
	std::cout << "Skybox framebuffer status: " << gl::enum_string(status) << std::endl;

	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint *)&default_frmaebuffer);
	std::cout << "Default framebuffer: " << default_frmaebuffer << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, skybox_framebuffer);

	skybox_pixels = new uint8_t[6 * skybox_size * skybox_size * 3];

	CubeInstance cube;
	cube.phase = 0.0f;
	cube.scale = 1.0f;

	cube.position = {1.0f, 0.0f, 0.0f};
	cube.color = {1.0f, 0.0f, 0.0f};
	cubes.push_back(cube);
	cube.position = {-1.0f, 0.0f, 0.0f};
	cube.color = {0.5f, 0.5f, 0.0f};
	cubes.push_back(cube);
	cube.position = {0.0f, 1.0f, 0.0f};
	cube.color = {0.0f, 1.0f, 0.0f};
	cubes.push_back(cube);
	cube.position = {0.0f, -1.0f, 0.0f};
	cube.color = {0.0f, 0.5f, 0.5f};
	cubes.push_back(cube);
	cube.position = {0.0f, 0.0f, 1.0f};
	cube.color = {0.0f, 0.0f, 1.0f};
	cubes.push_back(cube);
	cube.position = {0.0f, 0.0f, -1.0f},
	cube.color = {0.5f, 0.0f, 0.5f};
	cubes.push_back(cube);

	cube.phase = 1.0f;
	cube.scale = 0.2f;
	for (int i = 0; i < 6; ++i) {
		glm::vec3 p0 = cubes[i].position;
		for (int j = 0; j < 6; ++j) {
			auto const &c1 = cubes[j];
			cube.position = p0 + 0.15f * c1.position;
			cube.color = 0.9f * c1.color;
			cubes.push_back(cube);
		}
	}

	for (auto &cube : cubes) {
		cube.position *= 10.0f;
	}

	for (int i = 0; i < 200; ++i) {
		glm::vec3 p = {randf(-10, 10), randf(-10, 10), randf(-10, 10)};
		glm::normalize(p);
		p *= randf(3, 10);
		cubes.push_back({
			p, // position
			{randf(), randf(), randf()}, // color
			randf(-10, 10), // phase
			randf(1.0, 4.0), // scale
		});
	}

	cube.position = {0.0f, 0.0f, 0.0f};
	cube.color = {0.0f, 0.0f, 0.0f};
	cube.scale = 0.2f;
	cube.phase = 0.0;
	cubes.push_back(cube);

	while (!glfwWindowShouldClose(window) && rpc_server->is_running()) {
		glBindFramebuffer(GL_FRAMEBUFFER, default_frmaebuffer);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// gl_error_guard(glUseProgram(shaders.basic_program.program_id));
		// glBindVertexArray(vertex_array);
		// glDrawArrays(GL_TRIANGLES, 0, 3);

		glUseProgram(s.program_id);
		glBindVertexArray(cube_vertex_array);
		glm::mat4 tr = glm::translate(glm::identity<glm::mat4>(), {0, -5, -20});
		tr = glm::rotate(tr, 0.2f * (float)glfwGetTime(), {0, 1, 0});
		s.Projection = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 100.0f) * tr;
		s.light0_position = cubes.back().position + light0_offset;
		s.light1_position = cubes.back().position + light1_offset;
		for (auto &cube : cubes) {
			float t = cube.phase;  //(float)glfwGetTime() + cube.phase;
			cube.Model = glm::identity<glm::mat4>();
			cube.Model = glm::translate(cube.Model, cube.position);
			cube.Model = glm::scale(cube.Model, {cube.scale, cube.scale, cube.scale});
			cube.Model *= glm::eulerAngleYXZ(t * 2.0f, t * 3.0f, 0.0f);
			cube.Normal_model = glm::inverseTranspose(glm::mat3(cube.Model));

			s.Model = cube.Model;
			s.Normal_model = cube.Normal_model;
			s.color = glm::vec4(cube.color, 1.0f);

			glDrawArrays(GL_TRIANGLES, 0, cube_vertices.vertex_count());
		}

		process_tasks();

		glfwSwapBuffers(window);
		glfwPollEvents();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

void UI::on_key(int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void UI::process_tasks() {
	std::unique_ptr<Task> task = tasks.pop();
	if (!task) {
		return;
	}
	switch (task->type) {
		case TaskType::kSkyboxRender:
			process_skybox_render_task(*static_cast<SkyboxRenderTask *>(task.get()));
			break;

		default:
			std::cerr << "Unimplemented task type: " << task->type << std::endl;
			break;
	}
	tasks.done(std::move(task));
}

// TODO: The order is by trial & error. I have no idea why it is in this
// particular way. Probably something is wrong and I just made an even number of
// mistakes. Revisit and clean up.
const glm::mat4 LOOKATS[] = {
	glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(+1, 0, 0), glm::vec3(0, +1, 0)),
	glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, +1, 0)),
	glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, +1, 0), glm::vec3(0, 0, -1)),
	glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, +1)),
	glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, +1), glm::vec3(0, +1, 0)),
	glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, +1, 0)),
};

void UI::process_skybox_render_task(SkyboxRenderTask &task) {
	glBindFramebuffer(GL_FRAMEBUFFER, skybox_framebuffer);
	glViewport(0, 0, SKYBOX_SIZE, SKYBOX_SIZE);

	const auto &s = shaders.solid_program;
	glUseProgram(s.program_id);
	glBindVertexArray(cube_vertex_array);

	cubes.back().position = task.position;
	glm::mat tr = glm::translate(glm::identity<glm::mat4>(), -task.position);
	glm::mat4 p = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 flip = glm::scale(glm::identity<glm::mat4>(), glm::vec3(-1.0f, 1.0f, 1.0f));

	int i = 0;
	for (int i = 0; i < 6; ++i) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		s.Projection = p * LOOKATS[i] * tr;
		s.light0_position = cubes.back().position + light0_offset;
		s.light1_position = cubes.back().position + light1_offset;

		for (auto &cube : cubes) {
			s.Model = cube.Model;
			s.Normal_model = cube.Normal_model;
			s.color = glm::vec4(cube.color, 1.0f);
			glDrawArrays(GL_TRIANGLES, 0, cube_vertices.vertex_count());
		}

		glReadPixels(
				0, 0, SKYBOX_SIZE, SKYBOX_SIZE, GL_RGB, GL_UNSIGNED_BYTE,
				skybox_pixels + i * SKYBOX_SIZE * SKYBOX_SIZE * 3);
	}

	std::string path = "skybox.qoi";
	qoi_desc desc = {(unsigned int)SKYBOX_SIZE, 6 * (unsigned int)SKYBOX_SIZE, 3, QOI_LINEAR};
	qoi_write(path.c_str(), skybox_pixels, &desc);
	task.result.set_path(path);
}

void UI::create_cube_vertices(gl::VertexBuffer<SolidVertex> &vertex_buffer) {
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
	vertex_buffer.buffer_data(vertices.data(), vertices.size());
}