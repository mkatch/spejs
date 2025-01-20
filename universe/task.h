#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>

#include <universe/proto/task.grpc.pb.h>

#include "common.h"

typedef uint64_t TaskId;
class TaskReactor;

class Task final {
	weak_ptr<TaskReactor> reactor;

public:
	typedef universepb::TaskRequest Request;
	typedef pb::TaskResponse Response;
	typedef pb::TaskRequest::VariantCase VariantCase;

	const Request request;
	Response response;

	Task(const shared_ptr<TaskReactor> &reactor)
			: reactor(reactor) { }

	VariantCase variant_case() const { return request.task().variant_case(); }

	static void done(unique_ptr<Task> task);
};

class ActiveTaskBase {
public:
	virtual ~ActiveTaskBase();

	void done() { is_done = true; }

protected:
	unique_ptr<Task> task;
	bool is_done = false;

	ActiveTaskBase(unique_ptr<Task> &&task)
			: task(std::move(task)) { }
};

template <class RequestType, class ResponseType>
class ActiveTask : public ActiveTaskBase {
public:
	typedef RequestType Request;
	typedef ResponseType Response;

	const Request &request;
	Response &response;

protected:
	ActiveTask(const Request &request, Response &response, unique_ptr<Task> &&task)
			: ActiveTaskBase(std::move(task))
			, request(request)
			, response(response) {
		// Not a perfect check, but can detect some mistakes.
		assert(task->request.task().variant_case() == task->response.variant_case());
	}
};

class TaskQueue final : public universepb::TaskService::CallbackService {
	std::mutex mut;
	std::queue<unique_ptr<Task>> pending_tasks;

public:
	void add(unique_ptr<Task> &&task);

	unique_ptr<Task> pop();

	grpc::ServerBidiReactor<Task::Request, Task::Response> *Stream(grpc::CallbackServerContext *ctx) override;
};

class TaskReactor final : public grpc::ServerBidiReactor<Task::Request, Task::Response> {
	std::mutex mut;
	shared_ptr<TaskReactor> shared_this;
	TaskQueue &tasks;
	std::queue<unique_ptr<Task>> write_queue;
	unique_ptr<Task> read_target;

public:
	TaskReactor(TaskQueue &tasks);

	const shared_ptr<TaskReactor> &shared() const { return shared_this; }

	void done(unique_ptr<Task> task);

	void OnReadDone(bool ok) override;
	void OnWriteDone(bool ok) override;
	void OnDone() override;

private:
	void read_next();
	void write_next();
};