#pragma once

class ClientSocket : public Socket
{
public:
	ClientSocket(Protocol type);
	virtual ~ClientSocket();

	void Disconnect();
};