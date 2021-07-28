#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
	Semaphore(int cnt=0) : count(cnt){ }

	inline void Notify()
	{
		// unique_lock은 옵션으로 사용자가
		// 락을 해제하고 다시 락을 걸 수도 있다.
		std::unique_lock<std::mutex> lock(mtx);
		count++;
		// 대기 중인 하나의 스레드에게 알림
		cv.notify_one();
		// 대기 중인 모든 스레드에게 알림
		// cv.notify_all();
	}

	inline void Wait()
	{
		std::unique_lock<std::mutex> lock(mtx);

		while (count == 0) {
			// 알림을 받기 전까지 현재 스레드를 대기시킨다.
			cv.wait(lock);
			// 지정된 시간동안 기다린다.
			//cv.wait_for(lock, 1s);
			// 지정된 시간까지 기다린다.
			//cv.wait_until(lock, now + 1s);
		}
		count--;
	}

private:
	int count;
	std::mutex mtx;
	std::condition_variable cv;
};