#pragma once
#include "MemoryPool.h"
#include <vector>
#include <cassert>

template<typename T>
class MemoryPoolManager
{
public:
	static MemoryPoolManager& GetInstance()
	{
		static MemoryPoolManager instance;
		return instance;
	}

private:
	MemoryPoolManager() = default;
	MemoryPoolManager(const MemoryPoolManager&) = delete;
	MemoryPoolManager& operator=(const MemoryPoolManager&) = delete;
	MemoryPoolManager(MemoryPoolManager&&) = delete;
	MemoryPoolManager& operator=(MemoryPoolManager&&) = delete;

public:
	void Init(size_t poolSize)
	{
		assert(poolSize > 0);
		mMemPool.Init(poolSize);
	}

	void* Allocate()
	{
		void* ptr = mMemPool.Alloc();
		return ptr;
	}

	void Deallocate(void* ptr)
	{
		mMemPool.Dealloc(ptr);
	}

	size_t GetPoolSize()
	{
		return mMemPool.GetTotalSize();
	}

private:
	MemoryPool<T> mMemPool;
};