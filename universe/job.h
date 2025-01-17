#pragma once

#include <grpcpp/grpcpp.h>
#include <functional>
#include <mutex>

#include <proto/job.grpc.pb.h>

#include "common.h"

class JobServiceServer final : public pb::JobService::Service {
	const string command;
	const int pid;
	std::mutex mut;
	bool quit_requested = false;
	std::function<void()> on_quit;

public:
	JobServiceServer(int argc, char **argv);

	void set_on_quit(const std::function<void()> &callback);

	grpc::Status Attach(grpc::ServerContext *ctx, const google::protobuf::Empty *req, pb::JobAttachResponse *rsp) override;
	grpc::Status Status(grpc::ServerContext *ctx, const google::protobuf::Empty *req, pb::JobStatusResponse *rsp) override;
	grpc::Status Quit(grpc::ServerContext *ctx, const google::protobuf::Empty *req, google::protobuf::Empty *rsp) override;

private:
	static string assemble_command(int argc, char **argv);
};