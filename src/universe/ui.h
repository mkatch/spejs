#pragma once

class RpcServer;

class UI {
public:
	UI();
	~UI();

	void event_loop(const RpcServer *rpc_server);
};