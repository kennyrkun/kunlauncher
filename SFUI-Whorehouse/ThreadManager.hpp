#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <vector>
#include <thread>
#include <functional>

// HACK: std::function<void (void)> make sure at least one variable is specified or compile errors will occur

class ThreadedOperation
{
	ThreadedOperation(std::function<void (void)> function);
	~ThreadedOperation();

	int percent;
	int id;
	bool done;

	std::thread *thread;
	std::function<void (void)> *function;
};

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void newOperation(std::function<void (void)> function);
	void newThread();

	// add action to queue
	// remove action from queue
	// get action status

private:
	std::vector<ThreadedOperation*> threadQueue;
//	std::vector<std::thread*> threadQueue;
};

#endif
