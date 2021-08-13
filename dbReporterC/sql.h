#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <fstream>
#include <iostream>

#include "dbConn.h"

#ifdef UNICODE
#define STRING(x) L ##x
#define TCOUT std::wcout
#define printf wprintf
#define ttoi _wtoi
typedef std::wifstream tifstream;
typedef std::wofstream tofstream;
#else
#define STRING(x) x
#define TCOUT std::cout
#define printf printf
#define ttoi atoi
typedef std::ifstream tifstream;
typedef std::ofstream tofstream;
#endif // UNICODE

#define SQLFUNCEXEX(x, type, hstmt) {SQLRETURN res = x;\
						if (res != SQL_SUCCESS) \
							GetExtraInfo(res, type, hstmt);}

class sql {
private:
	SQLHENV hEnv;
	SQLHDBC hdb;
	SQLHSTMT hStmt;
	void GetExtraInfo(SQLRETURN ret, SQLSMALLINT hType, SQLHANDLE handle);
public:
	inline void CloseCursor() { SQLCloseCursor(hStmt); }
	SQLRETURN Fetch();
	void fetchNext();
	void closeSqlConn();
	void AllocSqlConn(SQLTCHAR* connStr);
	void bindCols(dbConn* row, TCHAR* hName, SQLLEN* hNameLen, TCHAR* pName, SQLLEN* pNameLen, TCHAR* status);
	void loadConfig(TCHAR*& connStr, int* updateRate);
};
