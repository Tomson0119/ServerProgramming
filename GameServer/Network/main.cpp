#include "stdafx.h"
#include "Socket.h"

using namespace std;

int main()
{
	try {
		Socket socket(Protocol::TCP);
		return 0;
	}
	catch (NetException& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_OK);
		return -1;
	}

}