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

bool DBHandler::FindPlayerID(const std::string& player_id)
{
	std::wstring query = L"EXEC find_player_id ";
	query.insert(query.end(), player_id.begin(), player_id.end());

	RETCODE ret = SQLExecDirect(m_hStmt, (SQLWCHAR*)query.c_str() , SQL_NTS);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	SQLVARCHAR retID[20]{};
	SQLLEN cbID = 0;
	ret = SQLBindCol(m_hStmt, 1, SQL_C_CHAR, &retID, 20, &cbID);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	for (int i = 0;; i++)
	{
		ret = SQLFetch(m_hStmt);
		PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret);

		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) 
			return true;
		else break;
	}

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		SQLCancel(m_hStmt);

	return false;
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
