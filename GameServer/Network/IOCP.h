#pragma once

class Socket;
struct WSAOVERLAPPEDEX;

struct CompletionInfo
{
	LONG64 key;
	DWORD bytes;
	BOOL success;
	WSAOVERLAPPED* overEx;
};

class IOCP
{
public:
	IOCP();
	~IOCP();

	void RegisterDevice(SOCKET sck, int key);
	void PostToCompletionQueue(WSAOVERLAPPEDEX* over, int key);
	CompletionInfo GetCompletionInfo() const;

private:
	HANDLE mIOCPHandle;
};