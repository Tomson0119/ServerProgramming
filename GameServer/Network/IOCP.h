#pragma once

class Socket;

class IOCP
{
public:
	IOCP();
	~IOCP();

	void RegisterDevice(const Socket& sck, int key);

private:
	HANDLE mIOCP;
};