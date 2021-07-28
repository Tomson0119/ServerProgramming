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
	//// ���� �߻� : primes�� ���Ҵ��ϸ鼭 �ٸ� �����尡 ������ �޸� ��ġ�� write��. 
	//auto result2 = GetPrimeNumbersWithNoMutex(max, threads);
	//cout << "Duration : " << result2.first << '\n';
	//print(result2.second);

	/*cout << "\nMultithread(with mutex) : \n";
	auto result3 = GetPrimeNumbersWithMutex(max, threads);
	cout << "Duration : " << result3.first << '\n';*/
	//print(result3.second);
}