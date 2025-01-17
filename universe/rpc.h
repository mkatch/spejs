#pragma once
#include <memory>
#include <string>

#include "job.h"

class TaskQueue;

class RpcServer {
	int _port;
	TaskQueue &tasks;
	JobServiceServer job_service;
	std::shared_ptr<grpc::Server> server;
	std::thread waiting_thread;
	std::atomic<bool> _is_running = true;

public:
	RpcServer(int argc, char **argv, TaskQueue &tasks);
	~RpcServer();

	void start(const string &addr);

	int port() const { return _port; }

	bool is_running() const { return _is_running; }

private:
	void ensure_quit();
};