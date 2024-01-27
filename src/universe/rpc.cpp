#include "rpc.h"

#include <grpcpp/grpcpp.h>

RpcServer::RpcServer(const std::string &addr) {
	grpc::ServerBuilder builder;
	builder.AddListeningPort(addr, grpc::InsecureServerCredentials(), &_port);
	builder.RegisterService(&job_service);
	builder.RegisterService(&universe_service);

	server = builder.BuildAndStart();
	if (!server) {
		throw std::exception("Failed to start server.");
	}

	job_service.set_server(server);

	// TODO: Idk why, but having server->Wait() in the main thread, and calling
	// server->Shutdown() from another thread, doesn't work and kills the process
	// abnormally. Investigate.
	waiting_thread = std::thread([&] { server->Wait(); });
}

void RpcServer::wait() {
	waiting_thread.join();
}