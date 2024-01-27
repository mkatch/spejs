#include "job.h"

void JobServiceImpl::set_server(const std::shared_ptr<grpc::Server> &server) {
	const std::lock_guard lock(mut);
	this->server = server;
	if (!_is_running) {
		server->Shutdown();
	}
}

grpc::Status JobServiceImpl::Status(grpc::ServerContext *c, Empty const *req, JobStatusResponse *rsp) {
	const std::lock_guard lock(mut);
	rsp->set_is_ready(true);
	return grpc::Status::OK;
}

grpc::Status JobServiceImpl::Quit(grpc::ServerContext *context, Empty const *request, Empty *response) {
	const std::lock_guard lock(mut);
	if (is_running()) {
		_is_running = false;
		if (server) {
			server->Shutdown();
		}
	}
	return grpc::Status::OK;
}