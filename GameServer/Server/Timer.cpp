#include "Timer.h"

Timer::Timer()
	: m_start{}, m_prev{}, m_curr{}, m_elapsed{}
{
}

Timer::~Timer()
{
}

void Timer::Start()
{
	m_start = clock::now();
	m_curr = m_start;
	m_prev = m_start;
}

void Timer::Tick()
{
	auto curr_time = clock::now();
	m_curr = curr_time;
	m_elapsed = std::chrono::duration<float>(m_curr - m_prev).count();
	m_prev = curr_time;
}

float Timer::GetElapsedTime() const
{
	return m_elapsed;
}

float Timer::GetTotalTime() const
{
	return std::chrono::duration<float>(m_curr - m_start).count();
}
