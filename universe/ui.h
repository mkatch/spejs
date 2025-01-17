#pragma once

#include "common.h"

#include <gl_cpp/gl.h>

#include "math.h"
#include "shaders.h"

class RpcServer;
class GLFWwindow;
class TaskQueue;
class SkyboxTask;

struct SolidVertex {
	glm::vec3 position;
	glm::vec3 normal;
};

struct CubeInstance {
	glm::vec3 position;
	glm::vec3 color;
	float phase;
	float scale;
	glm::mat4 Model;
	glm::mat3 Normal_model;
};

class UI {
	static constexpr int SKYBOX_SIZE = 512;

	GLFWwindow *window = nullptr;
	TaskQueue &tasks;
	uint8_t *skybox_pixels = nullptr;
	GLuint default_frmaebuffer = 0;
	GLuint skybox_framebuffer = 0;
	Shaders shaders;
	GLuint vertex_array;
	GLuint cube_vertex_array;
	gl::VertexBuffer<SolidVertex> cube_vertices;
	std::vector<CubeInstance> cubes;

public:
	UI(TaskQueue &tasks);
	~UI();

	void event_loop(const RpcServer *rpc_server);

private:
	void on_key(int key, int scancode, int action, int mods);
	void process_tasks();
	void process_skybox_task(SkyboxTask &task);

	static void create_cube_vertices(gl::VertexBuffer<SolidVertex> &vertex_buffer);
};