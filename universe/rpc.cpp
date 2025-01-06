#include "rpc.h"

#include <grpcpp/grpcpp.h>
#include <iostream>

RpcServer::RpcServer(int argc, char **argv, TaskQueue &tasks)
		: job_service(argc, argv)
		, skybox_service(tasks)
		, universe_service(tasks)
		, task_service(tasks) { }

void RpcServer::start(const string &addr) {
	grpc::ServerBuilder builder;
	builder.AddListeningPort(addr, grpc::InsecureServerCredentials(), &_port);
	builder.RegisterService(&job_service);
	builder.RegisterService(&universe_service);
	builder.RegisterService(&skybox_service);
	builder.RegisterService(&task_service);

	server = builder.BuildAndStart();
	if (!server) {
		throw std::exception("Failed to start server.");
	}

	job_service.set_on_quit(std::bind(&RpcServer::ensure_quit, this));

	waiting_thread = std::thread([=] { server->Wait(); });
}

RpcServer::~RpcServer() {
	ensure_quit();
	waiting_thread.join();
}

void RpcServer::ensure_quit() {
	if (_is_running.exchange(false)) {
		std::thread([=] { server->Shutdown(); }).detach();
	}
}