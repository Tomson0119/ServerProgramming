#pragma once

class WSAInit
{
public:
	WSAInit();
	~WSAInit();

	bool Init();

private:
	WSAData wsaData;
	int initResult;
};