#pragma once

#include "common.h"
#include "task.h"

class SkyboxTask final : public ActiveTask<pb::SkyboxRequest, pb::SkyboxResponse> {
public:
	SkyboxTask(unique_ptr<Task> &&task)
			: ActiveTask(task->request.task().skybox(), *task->response.mutable_skybox(), std::move(task)) { }
};