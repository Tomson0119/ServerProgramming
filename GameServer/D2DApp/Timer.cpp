#include "stdafx.h"
#include "Timer.h"

Timer::Timer()
{
	__int64 frequencyPerSec;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequencyPerSec));
	mSecondsPerCount = 1.0 / static_cast<double>(frequencyPerSec);
}

void Timer::Start()
{
	__int64 currTime;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

	if (mStopped)
	{
		mPausedTime += (currTime - mStopTime);
		mPrevTime = currTime;
		mStopTime = 0;
		mStopped = false;
	}
}

void Timer::Stop()
{
	if (!mStopped)
	{
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&mStopTime));
		mStopped = true;
	}
}

void Timer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped = false;
}

void Timer::Tick()
{
	if (mStopped)
	{
		mElapsedTime = 0;
		return;
	}

	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&mCurrTime));

	mElapsedTime = (mCurrTime - mPrevTime) * mSecondsPerCount;
	mPrevTime = mCurrTime;

	// Prevent not to be negative
	if (mElapsedTime < 0.0)
		mElapsedTime = 0.0;
}

float Timer::TotalTime() const
{
	if (mStopped)
		return static_cast<float>((mStopTime - mPausedTime - mBaseTime) * mSecondsPerCount);
	else
		return static_cast<float>((mCurrTime - mPausedTime - mBaseTime) * mSecondsPerCount);
}

float Timer::ElapsedTime() const
{
	return static_cast<float>(mElapsedTime);
}


