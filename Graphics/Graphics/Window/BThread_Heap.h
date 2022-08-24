#pragma once
#include "BThread.h"
#include <string>
#include <map>

using namespace std;

static class BThread_Heap
{
public:
	static void CreateThread(string IName, void* IFunction);
	static void Clear()
	{
		for (auto it : ThreadPool)
		{
			it.second->join();
		}
	}

	~BThread_Heap()
	{
		Clear();
	}

private:

	static map<string, std::thread *> ThreadPool;

};