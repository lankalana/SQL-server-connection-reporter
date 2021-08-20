#include "sql.h"

SQLRETURN sql::Fetch() {
	SQLRETURN res = SQLFetch(hStmt);
	if (res != SQL_SUCCESS)
		GetExtraInfo(res, SQL_HANDLE_STMT, hStmt);
	return res;
}

void sql::fetchNext() {
	SQLTCHAR query[] = STRING("SELECT session_id, status, host_name, program_name, cpu_time, memory_usage, open_transaction_count, logical_reads, reads, row_count, writes, CAST(login_time as time(0)) FROM sys.dm_exec_sessions eS WHERE is_user_process = 1 AND database_id = DB_ID(db_name()) ORDER BY login_time");
	SQLFUNCEXEX(SQLExecDirect(hStmt, query, SQL_NTS), SQL_HANDLE_STMT, hStmt);
}

void sql::loadConfig(TCHAR*& connStr, int* updateRate) {
	tifstream is("config.cfg", std::ifstream::binary);
	if (is) {
		is.seekg(0, is.end);
		if (is.tellg() > UINT_MAX)
			throw;
		UINT length = (UINT)is.tellg();
		is.seekg(0, is.beg);

		TCHAR* buff = new TCHAR[length];
		is.read(buff, length);

		if (!is)
			length = (UINT)is.gcount();
		is.close();

		for (unsigned int i = 0; i < length; i++) {
			if (i < length - 16 && Connected::isSame(&buff[i], STRING("connectionString")))
			{
				i += 16;
				while (i + 1 < length && buff[++i] != '"') {}
				int len = ++i;
				while (i + 1 < length && buff[i] != '\\' && buff[++i] != '"') {}
				len = i - len;
				connStr = new TCHAR[len + 1]{ '\0' };
				memcpy(connStr, &buff[i - len], len * sizeof(TCHAR));
			}
			if (i < length - 10 && Connected::isSame(&buff[i], STRING("updateRate")))
			{
				i += 10;
				while (i + 1 < length && buff[++i] != '"') {}
				int len = ++i;
				while (i + 1 < length && buff[i] != '\\' && buff[++i] != '"') {}
				len = i - len;
				TCHAR* arr = new TCHAR[len + 1]{ '\0' };
				memcpy(arr, &buff[i - len], len * sizeof(TCHAR));
				*updateRate = ttoi(arr);
				delete[] arr;
			}
		}
		delete[] buff;
	}
}

void sql::GetExtraInfo(SQLRETURN ret, SQLSMALLINT hType, SQLHANDLE handle)
{
	if ((ret == SQL_SUCCESS_WITH_INFO) || (ret == SQL_ERROR)) {
		SQLTCHAR SqlState[6], Msg[2048];
		SQLINTEGER NativeError;
		SQLLEN numRecs = 0;
		SQLSMALLINT i, MsgLen;
		SQLRETURN ret2;
		SQLGetDiagField(hType, handle, 0, SQL_DIAG_NUMBER, &numRecs, 0, 0);
		// Get the status records.
		i = 1;
		while (i <= numRecs && (ret2 = SQLGetDiagRec(hType, handle, i, SqlState, &NativeError, Msg, 2048, &MsgLen)) != SQL_NO_DATA) {
			printf(STRING("%s %d %s\n"), SqlState, NativeError, Msg);
			i++;
		}
		if (ret == SQL_ERROR)
			throw std::exception();
	}
}

void sql::AllocSqlConn(TCHAR* connStr)
{
	SQLFUNCEXEX(SQLAllocHandle(SQL_HANDLE_ENV, nullptr, &hEnv), SQL_HANDLE_ENV, hEnv);
	SQLSetEnvAttr(hEnv, 200, (SQLPOINTER)3UL, 0);
	SQLFUNCEXEX(SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hdb), SQL_HANDLE_ENV, hEnv);
	TCHAR* newConnStr = nullptr;
	short maxLen = 0;
	if (connStr == nullptr) {
		newConnStr = new TCHAR[2048];
		maxLen = 2048;
		SQLFUNCEXEX(SQLDriverConnect(hdb, GetDesktopWindow(), (SQLTCHAR*)connStr, SQL_NTS, (SQLTCHAR*)newConnStr, maxLen, &maxLen, SQL_DRIVER_COMPLETE_REQUIRED), SQL_HANDLE_DBC, hdb);
		tofstream myfile;
		myfile.open("config.cfg", std::ios::trunc);
		myfile << STRING("connectionString = \"");
		myfile.write(newConnStr, maxLen);
		myfile << STRING("\"\nupdateRate = \"5000\"");
		myfile.close();
		delete[] newConnStr;
	}
	else
		SQLFUNCEXEX(SQLDriverConnect(hdb, GetDesktopWindow(), (SQLTCHAR*)connStr, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE_REQUIRED), SQL_HANDLE_DBC, hdb);
	SQLFUNCEXEX(SQLSetConnectAttr(hdb, SQL_ACCESS_MODE, (SQLPOINTER)SQL_MODE_READ_ONLY, 0), SQL_HANDLE_DBC, hdb);
	SQLFUNCEXEX(SQLAllocHandle(SQL_HANDLE_STMT, hdb, &hStmt), SQL_HANDLE_DBC, hdb);
}

void sql::closeSqlConn()
{
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	SQLDisconnect(hdb);
	SQLFreeHandle(SQL_HANDLE_DBC, hdb);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void sql::bindCols(Connected::dataRow* row)
{
	SQLFUNCEXEX(SQLBindCol(hStmt, 1, SQL_C_SHORT, &row->id, 0, nullptr), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 2, SQL_C_TCHAR, row->status, 60, nullptr), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 3, SQL_C_TCHAR, row->host_name, row->host_nameSize, &row->host_nameSize), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 4, SQL_C_TCHAR, row->program_name, row->program_nameSize, &row->program_nameSize), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 5, SQL_C_USHORT, &row->cpu_time, 0, nullptr), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 6, SQL_C_USHORT, &row->memory_usage, 0, nullptr), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 7, SQL_C_USHORT, &row->open_transaction_count, 0, nullptr), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 8, SQL_C_ULONG, &row->logical_reads, 0, nullptr), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 9, SQL_C_ULONG, &row->reads, 0, nullptr), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 10, SQL_C_ULONG, &row->row_count, 0, nullptr), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 11, SQL_C_ULONG, &row->writes, 0, nullptr), SQL_HANDLE_STMT, hStmt);
	SQLFUNCEXEX(SQLBindCol(hStmt, 12, SQL_C_TIME, &row->login_time, 0, nullptr), SQL_HANDLE_STMT, hStmt);
}