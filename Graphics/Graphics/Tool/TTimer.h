#pragma once 

class TTimer
{
public:
	TTimer();

	void Reset(); // Call before message loop
	void Start(); //启动
	void Stop();  //暂停
	void Tick();  //每帧调用刷新当前时间

	//距离上一次Reset（）的时间，秒为单位
	float TotalTime()const; 
	//距离上一帧的时间，秒为单位
	float DeltaTime()const; 



private:
	__int64 Basetime;//起始时间
	__int64 Pausedtime;//总停止时间
	__int64 Stoptime;//上一次停顿时间
	__int64 Prevtime;//上一帧时间
	__int64 Currtime;//当前帧时间

	double Secondspercount;//cpu刷新时间（秒为单位）
	double Deltatime;//距离上一帧的时间，秒为单位

	bool Stopped;
};
