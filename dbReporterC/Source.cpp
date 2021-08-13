#include <iostream>

#define NAME_LEN 512
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
		while (true)
		{
			sqlObj.fetchNext();
			dbConnections data;
			dbConn row;
			TCHAR host_name[NAME_LEN] = { 0 };
			SQLLEN host_nameSize = NAME_LEN;
			TCHAR program_name[NAME_LEN] = { 0 };
			SQLLEN program_nameSize = NAME_LEN;
			TCHAR status[60] = { 0 };
			sqlObj.bindCols(&row, host_name, &host_nameSize, program_name, &program_nameSize, status);
			SQLRETURN res = sqlObj.Fetch();
			while (res != SQL_NO_DATA)
			{
				row.connStatus = dbConn::getStatus(status);
				host* hst = data.Get(host_name, (uint32_t)host_nameSize);
				if (hst == nullptr)
				{
					host temp;
					memcpy(temp.name, host_name, (uint32_t)host_nameSize);
					temp.nameSize = (int)host_nameSize;
					data.Add(temp);
					hst = &data.data[data.Cnt() - 1];
				}
				program* prog = (*hst).Get(program_name, (uint32_t)program_nameSize);
				if (prog == nullptr)
				{
					program temp;
					memcpy(temp.name, program_name, (uint32_t)program_nameSize);
					temp.nameSize = (int)program_nameSize;
					(*hst).Add(temp);
					prog = &(*hst).data[(*hst).Cnt() - 1];
				}
				(*prog).Add(row);
				res = sqlObj.Fetch();
			}
			sqlObj.CloseCursor();
			if (data != prevData) {
				cons.print(data, prevData);
				prevData = data;
			}
			for (USHORT i = 0; i < updateRate; i += SLEEP_STEP) {
				//if (GetKeyState(VK_ESCAPE) & 0x8000)
				//	goto end;
				Sleep(SLEEP_STEP);
			}
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