#include <iostream>

#define NAME_LEN 256
#define SLEEP_STEP 250

#include "sql.h"
#include "dbConn.h"
#include "console.h"

static console cons;
static sql sqlObj;

BOOL WINAPI onConsoleEvent(DWORD CtrlType) {
	switch (CtrlType) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		sqlObj.closeSqlConn();
		break;
	}
	return TRUE;
}

int wmain()
{
	TCHAR* connStr = nullptr;
	int updateRate = 5000;
	try {
		cons.InitConsoleOutput(&onConsoleEvent);
		sqlObj.loadConfig(connStr, &updateRate);
		sqlObj.AllocSqlConn(connStr);
		if (connStr != nullptr)
			delete[] connStr;
	}
	catch (std::exception e) {
		std::cout << e.what();
		std::cin.get();
		goto end;
	}
	catch (DWORD e) {
		std::cout << e;
		std::cin.get();
		goto end;
	}

	try {
		dbConnections prevData;
		dbConn row;
		TCHAR host_name[NAME_LEN] = { 0 };
		TCHAR program_name[NAME_LEN] = { 0 };
		SQLLEN host_nameSize = NAME_LEN;
		SQLLEN program_nameSize = NAME_LEN;
		TCHAR status[60] = { 0 };
		sqlObj.bindCols(&row, host_name, &host_nameSize, program_name, &program_nameSize, status);

		while (true)
		{
			dbConnections data;
			sqlObj.fetchNext();
			SQLRETURN res = sqlObj.Fetch();
			while (res != SQL_NO_DATA)
			{
				row.connStatus = dbConn::getStatus(status);
				host* hst = data.Get(host_name, (uint32_t)host_nameSize);
				if (hst == nullptr)
				{
					hst = data.AllocNew();
					memcpy(hst->name, host_name, (uint32_t)host_nameSize);
					hst->nameSize = (int)host_nameSize;
				}
				program* prog = (*hst).Get(program_name, (uint32_t)program_nameSize);
				if (prog == nullptr)
				{
					prog = hst->AllocNew();
					memcpy(prog->name, program_name, (uint32_t)program_nameSize);
					prog->nameSize = (int)program_nameSize;
				}
				memcpy(prog->AllocNew(), &row, sizeof(dbConn));
				res = sqlObj.Fetch();
			}
			sqlObj.CloseCursor();
			if (data != prevData) {
				cons.print(data, prevData);
				prevData = data;
			}
			for (USHORT i = 0; i < updateRate; i += SLEEP_STEP)
				Sleep(SLEEP_STEP);
		}
	}
	catch (std::exception e) {
		std::cout << e.what();
		std::cin.get();
	}
end:
	sqlObj.closeSqlConn();
	return 0;
}