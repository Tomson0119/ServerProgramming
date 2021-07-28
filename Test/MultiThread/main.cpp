#include "getPrimeNumber.h"

template<typename T>
void print(const vector<T>& vec)
{
	for (const T& e : vec)
		cout << e << " ";
	cout << '\n';
}

int main()
{
	int max = 250000;
	int threads = 4;

	/*cout << "Single Thread : \n";
	auto result1 = GetPrimeNumbersWithSingleThread(max);
	cout << "Duration : " << result1.first << '\n';*/
	//print(result1.second);

	cout << "Omp Schedule : \n";
	auto result2 = GetPrimeNumbersWithOmpSchedule(max);
	cout << "Duration : " << result2.first << '\n';

	//cout << "\nMultithread(no mutex) : \n";
	//// 에러 발생 : primes가 재할당하면서 다른 스레드가 엉뚱한 메모리 위치에 write함. 
	//auto result2 = GetPrimeNumbersWithNoMutex(max, threads);
	//cout << "Duration : " << result2.first << '\n';
	//print(result2.second);

	/*cout << "\nMultithread(with mutex) : \n";
	auto result3 = GetPrimeNumbersWithMutex(max, threads);
	cout << "Duration : " << result3.first << '\n';*/
	//print(result3.second);
}