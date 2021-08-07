#include <iostream>
#include <future>
#include <string>
#include <deque>
#include <conio.h>
#include <Windows.h>

using namespace std;

vector<string> allInputs;

bool dirty_flag = true;
mutex mtxDirty;

string inputData = "";
mutex mtxString;

deque<string> printQue;
mutex mtxPrint;

atomic_int atomData;

bool loop = true;
mutex mtxLoop;

bool check_loop()
{	
	lock_guard<mutex> lock(mtxLoop);
	bool isLoop = loop;
	return isLoop;
}

void input_task()
{
	bool isLoop = true;
	while (isLoop) {
		int c = _getch();
		scoped_lock<mutex, mutex> lock(mtxDirty, mtxString);
		if ((int)c == 13)
		{			
			if (inputData == "exit")
			{
				lock_guard<mutex> loopLock(mtxLoop);
				loop = false;
			}
			allInputs.push_back(inputData);
			inputData.clear();
		}
		else if ((int)c == 8 && !inputData.empty())
			inputData.erase(inputData.end() - 1);
		else if ((int)c != 27)
			inputData += c;

		dirty_flag = true;

		isLoop = check_loop();
	}
}

void output_task()
{	
	bool isLoop = true;
	while (isLoop) {
		this_thread::sleep_for(1000ms);
		atomData.fetch_add(1, memory_order_relaxed);
		int cnt = atomData.load(memory_order_acquire);
		if (cnt > 1000) break;
		
		scoped_lock<mutex, mutex> lock(mtxDirty, mtxPrint);
		printQue.push_back("Count: " + to_string(cnt));
		if (printQue.size() > 5)
			printQue.pop_front();
		dirty_flag = true;

		isLoop = check_loop();
	}
}

void print_task()
{
	bool isLoop = true;
	while (isLoop) {
		scoped_lock<mutex, mutex, mutex> lock(mtxDirty, mtxString, mtxPrint);
		if (dirty_flag) {
			system("cls");
			cout << "Input: " << inputData << '\n';

			for (auto iter = printQue.begin(); iter != printQue.end(); ++iter)
			{
				cout << *iter << endl;
			}
			dirty_flag = false;
		}

		isLoop = check_loop();
	}
}

int main()
{
	// input thread controls input.
	// ouput thread returns output.
	// main thread handles streams.
	
	thread inputThread(input_task);
	thread outputThread(output_task);
	thread printThread(print_task);

	inputThread.join();
	outputThread.join();
	printThread.join();

	for (const string& s : allInputs)
		cout << s << endl;
}