#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <vector>
#include <future>
#include <functional>

enum class TaskPriority
{
	Anywhere,
	ThisState,
	Now
};

class AsyncTask
{
public:
	~AsyncTask();

	bool finished();

	std::future<void> future;
};

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void addTask(AsyncTask* task);

	// checks for finished tasks, and clears them
	void update();

	// number of tasks currently active
	size_t tasks();
	// whether or not the task pool is empty
	bool empty();

	// TODO: add action to queue
	// TODO: remove action from queue

private:
	std::vector<AsyncTask*> threadQueue;
};

#endif // !THREAD_MANAGER_HPP
