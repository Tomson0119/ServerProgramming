#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <Windows.h>

#include <omp.h>

using namespace std;

struct Time
{
	chrono::steady_clock::time_point begin;
	chrono::steady_clock::time_point end;
	long long duration;

	void start()
	{
		begin = chrono::steady_clock::now();
	}

	void stop()
	{
		end = chrono::steady_clock::now();
		duration = chrono::duration_cast<chrono::milliseconds>(end - begin).count();
	}

	long long GetDuration() const { return duration; }
};

bool isPrimeNumber(int num)
{
	if (num == 1)
		return false;
	if (num == 2 || num == 3)
		return true;
	for (int i = 2; i < num - 1; ++i)
		if (num % i == 0)
			return false;
	return true;
}

pair<long long, vector<int>> GetPrimeNumbersWithSingleThread(int maxCount)
{
	int num = 1;
	vector<int> primes;

	Time timer;
	timer.start();

	while(true) {
		int n = num;
		num++;
		if (n >= maxCount) break;
		if (isPrimeNumber(n)) primes.push_back(n);
	}

	timer.stop();
	return { timer.GetDuration(), primes };
}

pair<long long, vector<int>> GetPrimeNumbersWithOmpSchedule(int maxCount)
{
	vector<int> primes;
	// recursive_mutex : 중복 lock을 허용한다.
	recursive_mutex primes_mutex;

	Time timer;
	timer.start();

	#pragma omp parallel for ordered schedule(dynamic) 
	for (int i = 1; i < maxCount; ++i) {
		if (isPrimeNumber(i)) {
			lock_guard<recursive_mutex> lock(primes_mutex);
			primes.push_back(i);
		}
	}

	timer.stop();
	return { timer.GetDuration(), primes };
}

pair<long long, vector<int>> GetPrimeNumbersWithNoMutex(int maxCount, int threadCount)
{
	int num = 1;
	vector<int> primes;
	recursive_mutex num_mutex;
	recursive_mutex prime_mutex;

	Time timer;
	timer.start();

	vector<shared_ptr<thread>> threads;

	for (int i = 0; i < threadCount; ++i)
	{
		shared_ptr<thread> th(new thread([&]() {
			while (true)
			{
				int n;
				n = num;
				num++;
				if (n >= maxCount) break;
				if (isPrimeNumber(n)) {
					primes.push_back(n);
				}
			}
			}));
		threads.push_back(th);
	}

	for (auto& t : threads)
		t->join();

	timer.stop();

	return { timer.GetDuration(), primes };
}

pair<long long, vector<int>> GetPrimeNumbersWithMutex(int maxCount, int threadCount)
{
	int num = 1;
	vector<int> primes;
	recursive_mutex num_mutex;
	recursive_mutex prime_mutex;

	Time timer;
	timer.start();

	vector<shared_ptr<thread>> threads;

	for (int i = 0; i < threadCount; ++i)
	{
		shared_ptr<thread> th(new thread([&]() {
			while (true)
			{
				int n;
				{
					lock_guard<recursive_mutex> lock(num_mutex);
					n = num;
					num++;
				}
				if (n >= maxCount) break;
				if (isPrimeNumber(n)) {
					lock_guard<recursive_mutex> lock(prime_mutex);
					primes.push_back(n);
				}
			}
			}));
		threads.push_back(th);
	}

	for (auto& t : threads)
		t->join();

	timer.stop();

	return { timer.GetDuration(), primes };
}