#include "job.h"

#include <process.h>
#include <iostream>

JobServiceServer::JobServiceServer(int argc, char **argv)
		: command(assemble_command(argc, argv)), pid(_getpid()) { }

void JobServiceServer::set_on_quit(const std::function<void()> &callback) {
	const std::lock_guard lock(mut);
	on_quit = callback;
	if (quit_requested) {
		on_quit();
	}
}

grpc::Status JobServiceServer::Attach(grpc::ServerContext *ctx, const google::protobuf::Empty *req, pb::JobAttachResponse *rsp) {
	std::cout << "JobServiceServer::Attach" << std::endl;
	rsp->set_command(command);
	rsp->set_pid(pid);
	return grpc::Status::OK;
}

grpc::Status JobServiceServer::Status(grpc::ServerContext *ctx, const google::protobuf::Empty *req, pb::JobStatusResponse *rsp) {
	rsp->set_is_ready(true);
	return grpc::Status::OK;
}

grpc::Status JobServiceServer::Quit(grpc::ServerContext *ctx, const google::protobuf::Empty *req, google::protobuf::Empty *rsp) {
	const std::lock_guard lock(mut);
	if (!quit_requested) {
		quit_requested = true;
		if (on_quit) {
			on_quit();
		}
	}
	return grpc::Status::OK;
}

string JobServiceServer::assemble_command(int argc, char **argv) {
	string command;
	for (int i = 0; i < argc; i++) {
		if (i > 0) {
			command += " ";
		}
		command += argv[i];
	}
	return command;
}