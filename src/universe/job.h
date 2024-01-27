#pragma once

#include <grpcpp/grpcpp.h>
#include <mutex>

#include "proto/job.grpc.pb.h"

class JobServiceImpl final : public JobService::Service {
	std::mutex mut;
	std::atomic<bool> _is_running = true;  // Accessed in the event loop, so better keep it atomic.
	std::shared_ptr<grpc::Server> server;

public:
	void set_server(const std::shared_ptr<grpc::Server> &server);

	bool is_running() { return _is_running; }

	grpc::Status Status(grpc::ServerContext *c, Empty const *req, JobStatusResponse *rsp) override;
	grpc::Status Quit(grpc::ServerContext *c, Empty const *req, Empty *rsp) override;
};