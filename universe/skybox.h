#pragma once

#include "common.h"
#include "task.h"

class Shaders;

class SkyboxTask final : public ActiveTask<pb::SkyboxRequest, pb::SkyboxResponse> {
public:
	SkyboxTask(unique_ptr<Task> &&task)
			: ActiveTask(task->request.task().skybox(), *task->response.mutable_skybox(), std::move(task)) { }
};

class Skybox {
public:
	static unique_ptr<Skybox> init(const Shaders &shaders);
	virtual ~Skybox() { }

	virtual void draw_preview(const glm::mat4 &ProjectionView) = 0;
	virtual void process_task(unique_ptr<Task> task) = 0;
};