#ifndef __THREADPOOL__
#define __THREADPOOL__
#include <memory>
#include <functional>

class ThreadPool
{
	class Impl;
	std::unique_ptr<Impl> mThis;
public:
	using TaskCallback = void();

	ThreadPool(size_t workerCount);
	~ThreadPool() noexcept;
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) noexcept;
	ThreadPool& operator=(ThreadPool&&) noexcept;

	void addTask(const std::function<TaskCallback> &callback);
	//blocks until there are no more tasks
	void waitForTasks();
};

#endif