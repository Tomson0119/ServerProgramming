#pragma once

#include <chrono>
#include <queue>

enum class EventType : char
{
	NPC_MOVE,
	NPC_REVIVE
};

struct TimerEvent
{
	int ObjectID;
	int TargetID;

	int Move_direction;
	EventType EvntType;
	std::chrono::system_clock::time_point StartTime;

	constexpr bool operator<(const TimerEvent& other) const
	{
		return (StartTime > other.StartTime);
	}
};

class IOCPServer;

class Timer
{
private:
	typedef std::chrono::high_resolution_clock clock;

public:
	Timer();
	~Timer();
	
	void Start(IOCPServer* ptr);
	void Tick();

	float GetElapsedTime() const;
	float GetTotalTime() const;

public:
	void AddTimerEvent(const TimerEvent& event);
	bool IsQueueEmpty();
	
	static void TimerThreadFunc(Timer& timer);

private:
	clock::time_point mStart;
	clock::time_point mPrev;
	clock::time_point mCurr;
	float mElapsed;

private:
	IOCPServer* mServerPtr;

	std::thread mTimerThread;
	std::atomic_bool mLoop;
	std::mutex mQueueMut;
	std::priority_queue<TimerEvent> mTimerQueue;
};