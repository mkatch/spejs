#include "task.h"

#include <unordered_set>

void TaskQueue::add(std::unique_ptr<Task> &&task) {
	std::lock_guard<std::mutex> lock(pending_mut);
	pending.emplace(std::move(task));
}

std::unique_ptr<Task> TaskQueue::pop() {
	std::lock_guard<std::mutex> lock(pending_mut);
	if (pending.empty()) {
		return nullptr;
	} else {
		std::unique_ptr<Task> task = std::move(pending.front());
		pending.pop();
		return task;
	}
}

void TaskQueue::done(std::unique_ptr<Task> &&task) {
	TaskResult *result = new TaskResult();
	task->write_result(result);
	std::lock_guard<std::mutex> lock(results_mut);
	results.AddAllocated(result);
}

grpc::Status TaskServiceServer::Poll(grpc::ServerContext *ctx, const TaskPollRequest *req, TaskPollResponse *rsp) {
	std::lock_guard<std::mutex> lock(tasks.results_mut);
	rsp->mutable_results()->Swap(&tasks.results);
	return grpc::Status::OK;
}