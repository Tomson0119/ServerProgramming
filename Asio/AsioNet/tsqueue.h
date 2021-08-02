#pragma once
#include "common.h"

namespace net
{
	template<typename T>
	class tsqueue
	{
	public:
		tsqueue() = default;
		tsqueue(const tsqueue<T>&) = delete;
		virtual ~tsQueue() { clear(); }
		
	public:
		// Returns and maintains item at front of Queue
		const T& front() const
		{
			std::scoped_lock lock(mtxQueue);
			return deq.front();
		}

		// Returns and maintains item at back of Queue
		const T& back() const
		{
			std::scoped_lock lock(mtxQueue);
			return deq.back();
		}

		// Adds an item to back of Queue
		void push_back(const T& item)
		{
			std::scoped_lock lock(mtxQueue);
			deq.emplace_back(std::move(item));
		}

		// Adds an item to front of Queue
		void push_front(const T& item)
		{
			std::scoped_lock lock(mtxQueue);
			deq.emplace_front(std::move(item));
		}

		// Removes and return item from front of Queue
		T pop_front()
		{
			std::scoped_lock lock(mtxQueue);
			auto t = std::move(deq.front());
			deq.pop_front();
			return t;
		}

		// Removes and returns item from back of Queue
		T pop_back()
		{
			std::scoped_lock lock(mtxQueue);
			auto t = std::move(deq.back());
			deq.pop_back();
			return t;
		}

		size_t count() const
		{
			std::scoped_lock lock(mtxQueue);
			return deq.size();
		}

		bool empty() const
		{
			std::scoped_lock lock(mtxQueue);
			return deq.empty();
		}

		void clear()
		{
			std::scoped_lock lock(mtxQueue);
			deq.clear();
		}

	protected:
		std::mutex mtxQueue;
		std::deque<T> deq;
	};
}