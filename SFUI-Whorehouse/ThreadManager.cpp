#include "ThreadManager.hpp"

#include <iostream>
#include <vector>

ThreadManager::ThreadManager()
{
	std::cout << "threadmanager constructed" << "\n";
}

ThreadManager::~ThreadManager()
{
	std::cout << "threadmanager deconstructed" << "\n";
}
