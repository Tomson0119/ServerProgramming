#pragma once

#include <exception>
#include <string>

class Exception : public std::exception
{
public:
	Exception(const char* msg);

	std::string GetLog() const { return errorLog; }

private:
	std::string errorLog;
};

std::string GetLastErrorLog();