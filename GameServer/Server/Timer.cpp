#include "common.h"
#include "Timer.h"
#include "IOCPServer.h"


Timer::Timer()
	: mStart{}, mPrev{}, mCurr{}, mElapsed{}, mServerPtr{}
{
}

Timer::~Timer()
{
	mLoop = false;
	mTimerThread.join();
}

void Timer::Start(IOCPServer* ptr)
{
	mStart = clock::now();
	mCurr = mStart;
	mPrev = mStart;

	mLoop = true;
	mServerPtr = ptr;
	mTimerThread = std::thread{ TimerThreadFunc, std::ref(*this) };
}

void Timer::Tick()
{
	auto curr_time = clock::now();
	mCurr = curr_time;
	mElapsed = std::chrono::duration<float>(mCurr - mPrev).count();
	mPrev = curr_time;
}

float Timer::GetElapsedTime() const
{
	return mElapsed;
}

float Timer::GetTotalTime() const
{
	return std::chrono::duration<float>(mCurr - mStart).count();
}

void Timer::AddTimerEvent(const TimerEvent& event)
{
	std::unique_lock<std::mutex> lock{ mQueueMut };
	mTimerQueue.push(event);
}

bool Timer::IsQueueEmpty()
{
	std::unique_lock<std::mutex> lock{ mQueueMut };
	return mTimerQueue.empty();
}

void Timer::TimerThreadFunc(Timer& timer)
{
	try {
		TimerEvent evnt{};
		while (timer.mLoop)
		{
			while (timer.IsQueueEmpty() == false)
			{
				{
					std::unique_lock<std::mutex> lock{ timer.mQueueMut };
					evnt = timer.mTimerQueue.top();

					auto now = std::chrono::system_clock::now();
					if (evnt.StartTime <= now)
					{
						timer.mTimerQueue.pop();
					}
					else
					{
						continue;
					}
				}
				
				switch (evnt.EvntType)
				{
				case EventType::NPC_MOVE:
				{
					timer.mServerPtr->PostNPCMoveEvent(
						evnt.ObjectID, 
						evnt.TargetID, 
						evnt.Move_direction);
					break;
				}
				case EventType::NPC_REVIVE:
				{
					timer.mServerPtr->HandleRevivedPlayer(evnt.ObjectID);
					break;
				}}
			}
			std::this_thread::sleep_for(10ms);
		}
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}
