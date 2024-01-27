#pragma once

#include <grpcpp/grpcpp.h>
#include <functional>
#include <mutex>

#include "proto/job.grpc.pb.h"

class JobServiceImpl final : public JobService::Service {
	std::mutex mut;
	bool quit_requested = false;
	std::function<void()> on_quit;

public:
	void set_on_quit(const std::function<void()> &callback);

	grpc::Status Status(grpc::ServerContext *c, Empty const *req, JobStatusResponse *rsp) override;
	grpc::Status Quit(grpc::ServerContext *c, Empty const *req, Empty *rsp) override;
};