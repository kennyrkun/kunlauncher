#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <vector>
#include <thread>

typedef void(*callback_function)(void); // type for conciseness

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	// add action to queue
	// remove action from queue
	// get action status

private:
	std::vector<std::thread*> threadQueue;
	std::thread *thread;
};

#endif
