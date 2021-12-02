#pragma once

#include <Windows.h>
#include <sqlext.h>
#include <string>

class DBHandler
{
public:
	DBHandler();
	~DBHandler();

	bool ConnectToDB(const std::wstring& sourcename);
	std::tuple<bool, short, short> FindPlayerInfo(const std::string& player_id);
	bool UpdatePlayerPosition(const std::string& player_id, short x, short y);

private:
	bool PrintIfError(SQLHANDLE handle, SQLSMALLINT type, RETCODE retCode);

private:
	SQLHENV m_hEnv;
	SQLHDBC m_hDbc;
	SQLHSTMT m_hStmt;
};