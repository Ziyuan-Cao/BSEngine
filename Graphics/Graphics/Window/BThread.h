#include <thread>

class BThread
{
private:
	BThread() = delete;
public:
	BThread(void * Function)
	{
		//std::thread t(Function);
	}

	~BThread()
	{

	}
};