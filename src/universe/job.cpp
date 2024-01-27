#include "job.h"

#include <iostream>

void JobServiceImpl::set_on_quit(const std::function<void()> &callback) {
	const std::lock_guard lock(mut);
	on_quit = callback;
	if (quit_requested) {
		on_quit();
	}
}

grpc::Status JobServiceImpl::Status(grpc::ServerContext *c, Empty const *req, JobStatusResponse *rsp) {
	const std::lock_guard lock(mut);
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