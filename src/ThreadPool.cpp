#include "ThreadPool.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <tuple>
#include <condition_variable>
#include <queue>
#include <functional>

class ThreadPool::Impl
{
	std::atomic<bool> workFlag;
	std::vector<std::thread> workers;
	std::queue<std::function<TaskCallback>> workQueue;
	std::mutex workMutex; //prevents data races in workStack
	std::condition_variable workAvailable, noWork;
	unsigned busy;

	void workerProcedure();
public:
	Impl(std::size_t);
	~Impl();

	void addTask(const std::function<TaskCallback>&);
	void waitForTasks();
};

void ThreadPool::Impl::workerProcedure()
{
	while (workFlag.load())
	{
		std::unique_lock<std::mutex> lck(workMutex);

		if (!workQueue.empty())
		{
			auto task = workQueue.front();
			workQueue.pop();

			++busy;
			lck.unlock();
			task();
			lck.lock();
			--busy;

			if(workQueue.empty())
				noWork.notify_all();
		}
		else
			workAvailable.wait(lck, [this]() { return !workQueue.empty() || !workFlag.load(); });
	}
}

ThreadPool::Impl::Impl(std::size_t workerCount)
	:workFlag(true)
	,busy(0)
{
	for (size_t i = 0; i < workerCount; ++i)
	{
		auto newElem = workers.emplace(workers.end(), std::thread(&Impl::workerProcedure, this));
	}
}

ThreadPool::Impl::~Impl()
{
	workFlag.store(false);
	workAvailable.notify_all();
	for (auto &workData : workers) {
		workData.join();
	}
}

void ThreadPool::Impl::addTask(const std::function<TaskCallback> &task)
{
	std::unique_lock<std::mutex>(workMutex);
	workQueue.emplace(task);
	workAvailable.notify_one();
}

void ThreadPool::Impl::waitForTasks()
{
	std::unique_lock<std::mutex> lck(workMutex);
	noWork.wait(lck, [this]() { return workQueue.empty() && !busy; }); //TODO: this returns when the work queue is empty, there could still be threads working
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ThreadPool::ThreadPool(size_t workerCount)
	:mThis(new Impl(workerCount))
{}

ThreadPool::~ThreadPool() noexcept = default;

ThreadPool::ThreadPool(ThreadPool&&) noexcept = default;

ThreadPool& ThreadPool::operator=(ThreadPool&&) noexcept = default;

void ThreadPool::addTask(const std::function<TaskCallback> &callback)
{
	mThis->addTask(callback);
}

void ThreadPool::waitForTasks()
{
	mThis->waitForTasks();
}