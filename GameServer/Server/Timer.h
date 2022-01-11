#pragma once

#include <chrono>

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

class Timer
{
private:
	typedef std::chrono::high_resolution_clock clock;

public:
	Timer();
	~Timer();
	
	void Start();
	void Tick();

	float GetElapsedTime() const;
	float GetTotalTime() const;	

private:
	clock::time_point m_start;
	clock::time_point m_prev;
	clock::time_point m_curr;

	float m_elapsed;
};