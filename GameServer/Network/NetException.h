#pragma once

class NetException : public std::exception
{
public:
	NetException(const std::string& info);
	virtual ~NetException();

	virtual const char* what() const;

private:
	std::string mErrorString;
};