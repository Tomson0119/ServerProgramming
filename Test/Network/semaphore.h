#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
	Semaphore(int cnt=0) : count(cnt){ }

	inline void Notify()
	{
		// unique_lock�� �ɼ����� ����ڰ�
		// ���� �����ϰ� �ٽ� ���� �� ���� �ִ�.
		std::unique_lock<std::mutex> lock(mtx);
		count++;
		// ��� ���� �ϳ��� �����忡�� �˸�
		cv.notify_one();
		// ��� ���� ��� �����忡�� �˸�
		// cv.notify_all();
	}

	inline void Wait()
	{
		std::unique_lock<std::mutex> lock(mtx);

		while (count == 0) {
			// �˸��� �ޱ� ������ ���� �����带 ����Ų��.
			cv.wait(lock);
			// ������ �ð����� ��ٸ���.
			//cv.wait_for(lock, 1s);
			// ������ �ð����� ��ٸ���.
			//cv.wait_until(lock, now + 1s);
		}
		count--;
	}

private:
	int count;
	std::mutex mtx;
	std::condition_variable cv;
};