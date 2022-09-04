#include "DBHandler.h"
#include <iostream>

DBHandler::DBHandler()
	: m_hEnv{}, m_hDbc{}, m_hStmt{}
{
	std::wcout.imbue(std::locale("korean"));	
}

DBHandler::~DBHandler()
{
	if (m_hEnv) SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
	if (m_hDbc) SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
	if (m_hStmt) SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
}

bool DBHandler::ConnectToDB(const std::wstring& sourcename)
{
	RETCODE ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);
	if (PrintIfError(m_hEnv, SQL_HANDLE_ENV, ret)) return false;

	ret = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	if (PrintIfError(m_hEnv, SQL_HANDLE_ENV, ret)) return false;

	ret = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);
	if (PrintIfError(m_hDbc, SQL_HANDLE_DBC, ret)) return false;

	ret = SQLSetConnectAttr(m_hDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	if (PrintIfError(m_hDbc, SQL_HANDLE_DBC, ret)) return false;

	ret = SQLConnect(m_hDbc, (SQLWCHAR*)sourcename.c_str(), SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
	if (PrintIfError(m_hDbc, SQL_HANDLE_DBC, ret)) return false;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &m_hStmt);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	return true;
}

std::pair<int, PlayerInfo> DBHandler::ConnectWithID(const std::string& player_id)
{
	std::wstring query = L"EXEC connect_with_id ";
	query.insert(query.end(), player_id.begin(), player_id.end());

	RETCODE ret = SQLExecDirect(m_hStmt, (SQLWCHAR*)query.c_str() , SQL_NTS);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) return { -1, {} };

	SQLSMALLINT retLevel = 0, retHP = 0, retMAXHP = 0;
	SQLSMALLINT retX = 0, retY = 0;
	SQLINTEGER retEXP = 0;
	SQLCHAR retConnected = 0;
	SQLLEN cb[7]{};

	SQLBindCol(m_hStmt, 1, SQL_C_SHORT, &retLevel, 2, &cb[0]);
	SQLBindCol(m_hStmt, 2, SQL_C_SHORT, &retX, 2, &cb[1]);
	SQLBindCol(m_hStmt, 3, SQL_C_SHORT, &retY, 2, &cb[2]);
	SQLBindCol(m_hStmt, 4, SQL_C_SHORT, &retHP, 2, &cb[3]);
	SQLBindCol(m_hStmt, 5, SQL_C_SHORT, &retMAXHP, 2, &cb[4]);
	SQLBindCol(m_hStmt, 6, SQL_C_LONG, &retEXP, 4, &cb[5]);
	SQLBindCol(m_hStmt, 7, SQL_C_BIT, &retConnected, 1, &cb[6]);
	
	ret = SQLFetch(m_hStmt);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) {
		SQLCloseCursor(m_hStmt);
		return { -1, {} };
	}

	SQLCancel(m_hStmt);
	SQLCloseCursor(m_hStmt);

	if ((bool)retConnected == true) {
		return { 0, {} };
	}

	PlayerInfo info{};
	strncpy_s(info.name, player_id.c_str(), player_id.size());
	info.level = (short)retLevel;
	info.x = (short)retX;
	info.y = (short)retY;
	info.hp = (short)retHP;
	info.max_hp = (short)retMAXHP;
	info.exp = (int)retEXP;
	return { 1, info };
}

bool DBHandler::DisconnectAndUpdate(PlayerInfo& info)
{
	std::wstring query = L"EXEC disconnect ";
	query.insert(query.end(), std::begin(info.name), std::begin(info.name) + strlen(info.name));
	query += L", " + std::to_wstring(info.level)
		+ L", " + std::to_wstring(info.x)
		+ L", " + std::to_wstring(info.y)
		+ L", " + std::to_wstring(info.hp)
		+ L", " + std::to_wstring(info.max_hp)
		+ L", " + std::to_wstring(info.exp);

	RETCODE ret = SQLExecDirect(m_hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(m_hStmt);
	}

	SQLCloseCursor(m_hStmt);
	return true;
}

bool DBHandler::PrintIfError(SQLHANDLE handle, SQLSMALLINT type, RETCODE retCode)
{
	if(retCode != SQL_ERROR) {
		if (retCode == SQL_INVALID_HANDLE) {
			std::cerr << "Invalid Handle\n";
			return true;
		}
		return false;
	}

	SQLSMALLINT record = 0;
	SQLINTEGER error = 0;
	WCHAR errorMsg[1000];
	WCHAR state[SQL_SQLSTATE_SIZE + 1]{};

	while (SQLGetDiagRec(type, handle, ++record, state, &error, errorMsg,
		(SQLSMALLINT)(sizeof(errorMsg) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS)
	{
		if (wcsncmp(errorMsg, L"01004", 5))
		{
			std::wcout << "[" << state << "] " << errorMsg << " (" << error << ")\n";
		}
	}
	return true;
}
