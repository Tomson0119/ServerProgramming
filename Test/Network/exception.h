#pragma once

#include <exception>
#include <string>

class Exception : public std::exception
{
public:
	Exception(const char* msg);

	const char* ErrorLog() const { return errorLog; }

private:
	const char* errorLog;
};

std::string GetLastErrorMessage();