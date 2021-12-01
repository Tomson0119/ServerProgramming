#pragma once

#include <stdexcept>

class NetException : public std::exception
{
public:
	NetException(const std::string& info);
	virtual ~NetException();

	virtual const char* what() const override;

private:
	std::string mErrorString;
};