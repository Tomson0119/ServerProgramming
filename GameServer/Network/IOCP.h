#pragma once

class Socket;

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
	CompletionInfo GetCompletionInfo() const;

private:
	HANDLE mIOCP;
};