#pragma once

#include "common.h"

#include <proto/skybox.grpc.pb.h>

#include "math.h"
#include "task.h"

class SkyboxServiceServer final : public SkyboxService::Service {
	TaskQueue &tasks;

public:
	SkyboxServiceServer(TaskQueue &tasks);

	grpc::Status Render(grpc::ServerContext *ctx, const SkyboxRenderRequest *req, SkyboxRenderResponse *rsp) override;
};

class SkyboxRenderTask : public Task {
public:
	const glm::vec3 position;
	SkyboxRenderResult result;

	SkyboxRenderTask(glm::vec3 position);

	void write_result(TaskResult *result) override;
};