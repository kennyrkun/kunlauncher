#include "ThreadManager.hpp"

#include <iostream>
#include <vector>

ThreadManager::ThreadManager()
{
	std::cout << "threadmanager constructed" << std::endl;
}

ThreadManager::~ThreadManager()
{
	std::cout << "threadmanager deconstructed" << std::endl;
}
