#pragma once
#include <Windows.h>

class CriticalSection 
{
private:
	CRITICAL_SECTION mCritSec;
public:
	CriticalSection() 
	{
		InitializeCriticalSectionEx(&mCritSec, 0, 0);
	}
	~CriticalSection() 
	{
		DeleteCriticalSection(&mCritSec);
	}

	void Lock() { EnterCriticalSection(&mCritSec); }
	void Unlock() { LeaveCriticalSection(&mCritSec); }
};

class CriticalSectionLock 
{
private:
	CriticalSection* mCritSec;
public:
	CriticalSectionLock(CriticalSection& critSec) 
	{
		mCritSec = &critSec;
		mCritSec->Lock();
	}
	~CriticalSectionLock()
	{
		mCritSec->Unlock();
	}
};