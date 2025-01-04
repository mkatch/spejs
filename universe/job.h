#pragma once

#include "common.h"

#include <grpcpp/grpcpp.h>
#include <functional>
#include <mutex>

#include <proto/job.grpc.pb.h>

class JobServiceImpl final : public JobService::Service {
	const string command;
	const int pid;
	std::mutex mut;
	bool quit_requested = false;
	std::function<void()> on_quit;

public:
	JobServiceImpl(int argc, char **argv);

	void set_on_quit(const std::function<void()> &callback);

	grpc::Status Attach(grpc::ServerContext *ctx, const google::protobuf::Empty *req, JobAttachResponse *rsp) override;
	grpc::Status Status(grpc::ServerContext *ctx, const google::protobuf::Empty *req, JobStatusResponse *rsp) override;
	grpc::Status Quit(grpc::ServerContext *ctx, const google::protobuf::Empty *req, google::protobuf::Empty *rsp) override;

private:
	static string assemble_command(int argc, char **argv);
};