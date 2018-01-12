#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <vector>
#include <thread>

class ThreadedOperation
{
	int percent;
	int id;
	bool done;

	std::thread *thread;
};

typedef void(*callback_function)(void); // type for conciseness

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void newOperation(std::function function);

	// add action to queue
	// remove action from queue
	// get action status

private:
	std::vector<ThreadedOperation*> threadQueue;
//	std::vector<std::thread*> threadQueue;
	std::thread *thread;
};

#endif
