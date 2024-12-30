#pragma once

#include "common.h"

#include <memory>
#include <mutex>
#include <queue>
#include <variant>

struct SkyboxTask {
	string asset_id;
};

typedef std::variant<std::monostate, SkyboxTask> Task;

enum TaskType : size_t {
	TASK_NONE = 0,
	TASK_SKYBOX = 1,
};

class TaskQueue {
	std::mutex mut;
	std::queue<Task> tasks;

public:
	void emplace(Task &&task);
	bool pop(Task &out);
};