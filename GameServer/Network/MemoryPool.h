#pragma once
#include <concurrent_queue.h>

template <typename T>
class MemoryPool
{
public:
	MemoryPool() = default;
	MemoryPool(const MemoryPool&) = delete;
	MemoryPool& operator=(const MemoryPool&) = delete;
	MemoryPool(MemoryPool&&) = delete;
	MemoryPool& operator=(MemoryPool&&) = delete;

	~MemoryPool()
	{
		if (mPool) delete[] mPool;
	}

	void Init(size_t count)
	{
		mBlockCount = count;
		mPoolSize = count * sizeof(T);
		mPool = new T[count];

		for (int i = 0; i < (int)mBlockCount; i++)
		{
			void* ptr = reinterpret_cast<void*>(mPool + i);
			mMemAddrs.push(ptr);
		}
	}

	void* Alloc()
	{
		if (mMemAddrs.empty())
			return nullptr;

		void* ptr = nullptr;
		while (mMemAddrs.try_pop(ptr) == false);

		return ptr;
	}

	void Dealloc(void* p)
	{
		if (p)
		{
			auto ptr = reinterpret_cast<void*>(p);
			p = nullptr;
			mMemAddrs.push(ptr);
		}
	}

	size_t GetTotalSize()
	{
		return mPoolSize;
	}

private:
	T* mPool = nullptr;
	concurrency::concurrent_queue<void*> mMemAddrs;

	size_t mBlockCount = 0;
	std::atomic_size_t mPoolSize = 0;
};