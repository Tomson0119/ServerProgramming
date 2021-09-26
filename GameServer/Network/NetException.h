#pragma once

class NetException : public std::exception
{
public:
	NetException();
	virtual ~NetException();

	virtual const char* what() const;

private:
	std::wstring errorInfo;
};