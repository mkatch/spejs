#pragma once
#include <memory>
#include <string>

#include "job.h"
#include "universe.h"

class RpcServer {
	int _port;
	JobServiceImpl job_service;
	UniverseServiceImpl universe_service;
	std::shared_ptr<grpc::Server> server;
	std::thread waiting_thread;

	RpcServer(const std::string &addr);

public:
	static std::unique_ptr<RpcServer> build_and_start(const std::string &addr) {
    return std::unique_ptr<RpcServer>(new RpcServer(addr));
  }

	int port() { return _port; }

	bool is_running() { return job_service.is_running(); }

	void wait();
};