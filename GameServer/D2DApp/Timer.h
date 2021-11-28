#pragma once

class Timer
{
public:
	Timer();
	~Timer() { }

	void Start();
	void Stop();
	void Reset();
	void Tick();

	float TotalTime() const;
	float ElapsedTime() const;

private:
	double mSecondsPerCount = 0.0;
	double mElapsedTime     = 0.0;

	__int64 mBaseTime   = 0;
	__int64 mPausedTime = 0;
	__int64 mStopTime   = 0;
	__int64 mCurrTime   = 0;
	__int64 mPrevTime   = 0;

	bool mStopped = false;
};