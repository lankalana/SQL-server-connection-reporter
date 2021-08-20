#include <iostream>

#define NAME_LEN 256
#define SLEEP_STEP 250

#include "sql.h"
#include "console.h"
#include "dataTypes.h"

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
		using namespace Connected;
		DB prevData;
		dataRow row;
		sqlObj.bindCols(&row);
		while (true)
		{
			DB data;
			sqlObj.fetchNext();
			SQLRETURN res = sqlObj.Fetch();
			while (res != SQL_NO_DATA)
			{
				Host* hst = data.Get(row.host_name, (uint32_t)row.host_nameSize);
				if (hst == nullptr)
				{
					data.hosts.push_back(Host());
					hst = &data.hosts.back();
					memcpy(hst->name, row.host_name, (uint32_t)row.host_nameSize);
					hst->nameSize = (int)row.host_nameSize;
				}
				Program* prog = (*hst).Get(row.program_name, (uint32_t)row.program_nameSize);
				if (prog == nullptr)
				{
					hst->programs.push_back(Program());
					prog = &hst->programs.back();
					memcpy(prog->name, row.program_name, (uint32_t)row.program_nameSize);
					prog->nameSize = (int)row.program_nameSize;
				}
				prog->Add(&row);
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