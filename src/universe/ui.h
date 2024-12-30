#pragma once

class RpcServer;
class GLFWwindow;
class TaskQueue;
class SkyboxTask;

class UI {
	GLFWwindow *window = nullptr;
	uint8_t *pixels = nullptr;
	TaskQueue &tasks;

public:
	UI(TaskQueue &tasks);
	~UI();

	void event_loop(const RpcServer *rpc_server);

private:
	void on_key(int key, int scancode, int action, int mods);
	void process_tasks();
	void process_task(const SkyboxTask &task);
};