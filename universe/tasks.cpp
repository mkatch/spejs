#include "tasks.h"

void TaskQueue::emplace(Task &&task) {
	std::lock_guard<std::mutex> lock(mut);
	tasks.emplace(task);
}

bool TaskQueue::pop(Task &out) {
	std::lock_guard<std::mutex> lock(mut);
	if (tasks.empty()) {
		return false;
	} else {
		out = std::move(tasks.front());
		tasks.pop();
		return true;
	}
}