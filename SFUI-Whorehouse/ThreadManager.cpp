#include "ThreadManager.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

AsyncTask::~AsyncTask()
{
	std::cout << "AsyncTask destroyed." << std::endl;
}

bool AsyncTask::finished()
{
	return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

// THREADMANAGER

ThreadManager::ThreadManager()
{
	std::cout << "threadmanager constructed" << std::endl;
}

ThreadManager::~ThreadManager()
{
	std::cout << "threadmanager deconstructed" << std::endl;

	for (size_t i = 0; i < threadQueue.size(); i++)
		delete threadQueue[i];

	threadQueue.clear();
}

void ThreadManager::addTask(AsyncTask* task)
{
	threadQueue.push_back(task);
}

void ThreadManager::update()
{
	for (size_t i = 0; i < threadQueue.size(); i++)
	{
		if (threadQueue[i]->finished())
		{
			AsyncTask* task = threadQueue[i];
			threadQueue.erase(std::remove(threadQueue.begin(), threadQueue.end(), threadQueue[i]), threadQueue.end());
			delete task;
		}
	}
}

size_t ThreadManager::tasks()
{
	return threadQueue.size();
}

bool ThreadManager::empty()
{
	return threadQueue.empty();
}
