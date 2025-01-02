#pragma once

#include "common.h"

#include <universe/proto/skybox.grpc.pb.h>

class TaskQueue;

class UniverseSkyboxServiceServer final : public UniverseSkyboxService::Service {
	TaskQueue &tasks;

public:
	UniverseSkyboxServiceServer(TaskQueue &tasks);

	grpc::Status Render(grpc::ServerContext *ctx, const UniverseSkyboxRenderRequest *req, UniverseSkyboxRenderResponse *rsp) override;
};