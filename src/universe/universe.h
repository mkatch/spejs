#pragma once

#include <grpcpp/grpcpp.h>

#include <proto/universe.grpc.pb.h>

class TaskQueue;

class UniverseServiceImpl final : public UniverseService::Service {
	TaskQueue &tasks;

public:
	UniverseServiceImpl(TaskQueue &tasks)
			: tasks(tasks) { }

	grpc::Status Ping(grpc::ServerContext *c, PingRequest const *req, PingResponse *rsp) override;
	grpc::Status OpticalSample(grpc::ServerContext *c, OpticalSampleRequest const *req, OpticalSampleResponse *rsp) override;
	grpc::Status Skybox(grpc::ServerContext *c, const SkyboxRequest *req, SkyboxResponse *rsp) override;
};