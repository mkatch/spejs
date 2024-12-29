#pragma once

class RpcServer;
class GLFWwindow;

class UI {
	GLFWwindow *window = nullptr;
	bool save_next_frame = false;
	uint8_t *pixels = nullptr;

public:
	UI();
	~UI();

	void event_loop(const RpcServer *rpc_server);

private:
	void on_key(int key, int scancode, int action, int mods);
};