#include <windows.h>
#include "BGraphics.h"

TTimer::TTimer()
	: Secondspercount(0.0), Deltatime(-1.0), Basetime(0),
	Pausedtime(0), Prevtime(0), Currtime(0), Stopped(false)
{
	//cpu刷新时间（秒为单位）= 1 / 每秒刷新cpu次数（频率）
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	Secondspercount = 1.0 / (double)countsPerSec;
}

// 返回Reset()之后的总时间，不包含暂停时间
float TTimer::TotalTime()const
{
	//当前还在暂停的话
	if (Stopped)
	{
		return (float)(((Stoptime - Pausedtime) - Basetime) * Secondspercount);
	}
	//否则就当前帧时间减去总暂停时间
	else
	{
		return (float)(((Currtime - Pausedtime) - Basetime) * Secondspercount);
	}
}

float TTimer::DeltaTime()const
{
	return (float)Deltatime;
}

void TTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	Basetime = currTime;
	Prevtime = currTime;
	Stoptime = 0;
	Stopped = false;
}

void TTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (Stopped)
	{
		//记录总暂停时间
		Pausedtime += (startTime - Stoptime);

		Prevtime = startTime;
		Stoptime = 0;
		Stopped = false;
	}
}

void TTimer::Stop()
{
	if (!Stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		Stoptime = currTime;
		Stopped = true;
	}
}

void TTimer::Tick()
{
	//正在暂停则不刷新
	if (Stopped)
	{
		Deltatime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	Currtime = currTime;
	//与前一帧相间隔时间
	Deltatime = (Currtime - Prevtime) * Secondspercount;

	Prevtime = Currtime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then Deltatime can be negative.
	if (Deltatime < 0.0)
	{
		Deltatime = 0.0;
	}
}
