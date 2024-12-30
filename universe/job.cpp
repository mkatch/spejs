#include "job.h"

#include <process.h>
#include <iostream>

JobServiceImpl::JobServiceImpl(int argc, char **argv)
		: command(assemble_command(argc, argv)), pid(_getpid()) { }

void JobServiceImpl::set_on_quit(const std::function<void()> &callback) {
	const std::lock_guard lock(mut);
	on_quit = callback;
	if (quit_requested) {
		on_quit();
	}
}

grpc::Status JobServiceImpl::Attach(grpc::ServerContext *c, Empty const *req, JobAttachResponse *rsp) {
	rsp->set_command(command);
	rsp->set_pid(pid);
	return grpc::Status::OK;
}

grpc::Status JobServiceImpl::Status(grpc::ServerContext *c, Empty const *req, JobStatusResponse *rsp) {
	rsp->set_is_ready(true);
	return grpc::Status::OK;
}

grpc::Status JobServiceImpl::Quit(grpc::ServerContext *context, Empty const *request, Empty *response) {
	const std::lock_guard lock(mut);
	if (!quit_requested) {
		quit_requested = true;
		if (on_quit) {
			on_quit();
		}
	}
	return grpc::Status::OK;
}

string JobServiceImpl::assemble_command(int argc, char **argv) {
	string command;
	for (int i = 0; i < argc; i++) {
		if (i > 0) {
			command += " ";
		}
		command += argv[i];
	}
	return command;
}