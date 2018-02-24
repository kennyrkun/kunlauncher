#include "ThreadManager.hpp"

#include <iostream>
#include <vector>

ThreadedOperation::ThreadedOperation(std::function<void (void)> function)
{
}

ThreadedOperation::~ThreadedOperation()
{
}

// THREADMANAGER

ThreadManager::ThreadManager()
{
	std::cout << "threadmanager constructed" << std::endl;
}

ThreadManager::~ThreadManager()
{
	std::cout << "threadmanager deconstructed" << std::endl;
}

void ThreadManager::newOperation(std::function<void (void)> function)
{

}

void ThreadManager::newThread()
{
}
