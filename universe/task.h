#pragma once

#include "common.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>

#include <proto/task.grpc.pb.h>

typedef uint32_t TaskId;
typedef TaskResult::ResultCase TaskType;

class Task {
public:
	const TaskType type;

	Task(TaskType type)
			: type(type) { }
	virtual ~Task() = default;

	virtual void write_result(TaskResult *result) = 0;
};

class TaskQueue {
	std::mutex pending_mut;
	std::queue<std::unique_ptr<Task>> pending;
	std::mutex results_mut;
	google::protobuf::RepeatedPtrField<TaskResult> results;

public:
	void add(std::unique_ptr<Task> &&task);

	template <class T, class... Args>
	void add(Args &&...args) {
		add(std::make_unique<T>(std::forward<Args>(args)...));
	}

	std::unique_ptr<Task> pop();

	void done(std::unique_ptr<Task> &&task);

private:
	friend class TaskServiceServer;
};

// TODO: Use the async API (callback or queues?)
class TaskServiceServer final : public TaskService::Service {
	TaskQueue &tasks;

public:
	TaskServiceServer(TaskQueue &tasks)
			: tasks(tasks) { }

	grpc::Status Poll(grpc::ServerContext *ctx, const TaskPollRequest *req, TaskPollResponse *rsp) override;
};