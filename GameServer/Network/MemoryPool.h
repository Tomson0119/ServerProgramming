#pragma once
#include <concurrent_queue.h>
#include <queue>

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
		mQueMut.lock();
		if (mMemAddrs.empty())
		{
			mQueMut.unlock();
			return nullptr;
		}

		void* ptr = mMemAddrs.front();
		mMemAddrs.pop();
		mQueMut.unlock();

		return ptr;
	}

	void Dealloc(void* p)
	{
		if (p)
		{
			auto ptr = reinterpret_cast<void*>(p);
			p = nullptr;
			mQueMut.lock();
			mMemAddrs.push(ptr);
			mQueMut.unlock();
		}
	}

	size_t GetTotalSize()
	{
		return mPoolSize;
	}

private:
	T* mPool = nullptr;
	//concurrency::concurrent_queue<void*> mMemAddrs;

	std::mutex mQueMut;
	std::queue<void*> mMemAddrs;

	size_t mBlockCount = 0;
	size_t mPoolSize = 0;
};