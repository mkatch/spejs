#include "task.h"

#include <unordered_set>

void Task::done(unique_ptr<Task> task) {
	shared_ptr<TaskReactor> reactor = task->reactor.lock();
	if (!reactor) {
		return;
	}
	reactor->done(std::move(task));
}

ActiveTaskBase::~ActiveTaskBase() {
	if (is_done) {
		Task::done(std::move(task));
	}
	// TODO: What should happen if it was not done?
}

void TaskQueue::add(std::unique_ptr<Task> &&task) {
	std::lock_guard<std::mutex> lock(mut);
	pending_tasks.emplace(std::move(task));
}

unique_ptr<Task> TaskQueue::pop() {
	std::lock_guard<std::mutex> lock(mut);
	if (pending_tasks.empty()) {
		return nullptr;
	} else {
		unique_ptr<Task> task = std::move(pending_tasks.front());
		pending_tasks.pop();
		return task;
	}
}

grpc::ServerBidiReactor<Task::Request, Task::Response> *TaskQueue::Stream(grpc::CallbackServerContext *ctx) {
	std::cout << "Streaming man!" << std::endl;
	return new TaskReactor(*this);
}

TaskReactor::TaskReactor(TaskQueue &tasks)
		: shared_this(this), tasks(tasks) {
	read_next();
}

void TaskReactor::done(unique_ptr<Task> task) {
	std::lock_guard<std::mutex> lock(mut);
	write_queue.push(std::move(task));
	write_next();
}

void TaskReactor::read_next() {
	read_target = make_unique<Task>(shared_this);
	// const_cast is fine, because we own the task that we just created and it
	// will not be accessed by anyone before the read finishes.
	StartRead(const_cast<Task::Request *>(&read_target->request));
}

void TaskReactor::OnReadDone(bool ok) {
	std::cout << "OnReadDone" << std::endl;
	if (!ok) {
		return;
	}
	std::lock_guard<std::mutex> lock(mut);
	tasks.add(std::move(read_target));
	read_next();
}

void TaskReactor::write_next() {
	if (write_queue.empty()) {
		return;
	}
	StartWrite(&write_queue.front()->response);
}

void TaskReactor::OnWriteDone(bool ok) {
	std::cout << "OnWriteDone" << std::endl;
	if (!ok) {
		return;
	}
	std::lock_guard<std::mutex> lock(mut);
	write_queue.pop();
	write_next();
}

void TaskReactor::OnDone() {
	shared_this.reset();
}
