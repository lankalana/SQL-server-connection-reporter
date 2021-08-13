#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "dbConn.h"

#ifdef UNICODE
#define STRING(x) L ##x
#define TCOUT std::wcout
#define printf wprintf
#define ttoi _wtoi
#else
#define STRING(x) x
#define TCOUT std::cout
#define printf printf
#define ttoi atoi
#endif // UNICODE

class console {
private:
	const TCHAR* KNRM = STRING("\x1B[0m");
	const TCHAR* KRED = STRING("\x1B[31m");
	const TCHAR* KGRN = STRING("\x1B[32m");
	HANDLE hOut1, hOut2;
	CONSOLE_SCREEN_BUFFER_INFO s;
public:
	void clear_screen(char);
	void addColor(TCHAR* buf, char change);
	void print(dbConnections conns, dbConnections prevConns);
	void InitConsoleOutput(PHANDLER_ROUTINE);

	template <class T>
	char getChange(T oldVal, T newVal);
};
