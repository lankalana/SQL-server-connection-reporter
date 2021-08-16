#include "console.h"

void console::clear_screen(char fill = ' ') {
	DWORD wrote, cells = s.dwSize.X * s.dwSize.Y;
	FillConsoleOutputCharacter(hOut1, fill, cells, { 0,0 }, &wrote);
	FillConsoleOutputAttribute(hOut1, s.wAttributes, cells, { 0,0 }, &wrote);
	SetConsoleCursorPosition(hOut1, { 0, 0 });
}

void console::addColor(TCHAR* buf, char change)
{
	int len = 0;
	if (change == 0)
		return;
	else if (change > 0)
		memcpy(buf, KRED, 6 * sizeof(TCHAR));
	else
		memcpy(buf, KGRN, 6 * sizeof(TCHAR));
}

void console::print(dbConnections conns, dbConnections prevConns) {
	clear_screen(' ');

	//_CHAR_INFO* consBuf = new _CHAR_INFO[s.dwSize.X * s.dwSize.Y];
	//_SMALL_RECT rect = { 0, 0, s.dwSize.X, s.dwSize.Y };
	//ReadConsoleOutput(hOut1, consBuf, { s.dwSize.X, s.dwSize.Y }, { 0, 0 }, &rect);
	//WriteConsoleOutput(hOut2, consBuf, { s.dwSize.X, s.dwSize.Y }, { 0, 0 }, &rect);
	//delete[] consBuf;
	//SetConsoleActiveScreenBuffer(hOut2);

	int newConnections = 0;
	int oldConnections = 0;

	for (unsigned int h = 0; h < conns.Cnt(); h++) {
		bool newHost = false;
		TCHAR buf[NAME_LEN] = { '\0' };
		TCHAR name[6] = { '\0' };

		if (!(h < prevConns.Cnt())) {
			newHost = true;
			prevConns.AllocNew();
		}

		memcpy(buf, conns.data[h].name, conns.data[h].nameSize);
		addColor(name, newHost);
		printf(STRING("%s%s"), name, buf);

		memset(buf, 0, sizeof(buf));
		memset(name, 0, sizeof(name));

		for (unsigned int p = 0; p < conns.data[h].Cnt(); p++) {
			bool newProg = false, newConns = false;
			char statuses[5] = { 0 };
			char prevStatuses[5] = { 0 };
			dbConn sum, prevSum;

			if (!(p < prevConns.data[h].Cnt())) {
				newProg = true;
				prevConns.data[h].AllocNew();
			}

			for (unsigned int c = 0; c < conns.data[h].data[p].Cnt(); c++) {
				sum += conns.data[h].data[p].data[c];
				statuses[(int)conns.data[h].data[p].data[c].connStatus]++;
				if (c < prevConns.data[h].data[p].Cnt()) {
					prevSum += prevConns.data[h].data[p].data[c];
					prevStatuses[(int)prevConns.data[h].data[p].data[c].connStatus]++;
				}
				else {
					newConnections++;
					newConns = true;
				}
			}
			if (prevConns.data[h].data[p].Cnt() > conns.data[h].data[p].Cnt())
				oldConnections += prevConns.data[h].data[p].Cnt() - conns.data[h].data[p].Cnt();

			memcpy(buf, conns.data[h].data[p].name, conns.data[h].data[p].nameSize);
			addColor(name, newProg);

			TCHAR date[6] = { '\0' };
			addColor(date, getChange(prevSum.login_time, sum.login_time));

			TCHAR cnts[5][6] = { {'\0'} };
			addColor(cnts[0], newConns);
			addColor(cnts[1], getChange(prevStatuses[1], statuses[1]));
			addColor(cnts[2], getChange(prevStatuses[2], statuses[2]));
			addColor(cnts[3], getChange(prevStatuses[3], statuses[3]));
			addColor(cnts[4], getChange(prevStatuses[4], statuses[4]));

			TCHAR cpuMem[2][6] = { {'\0'} };
			addColor(cnts[0], getChange(prevSum.cpu_time, sum.cpu_time));
			addColor(cnts[1], getChange(prevSum.memory_usage, sum.memory_usage));

			TCHAR io[3][6] = { {'\0'} };
			addColor(io[0], getChange(prevSum.reads, sum.reads));
			addColor(io[1], getChange(prevSum.writes, sum.writes));
			addColor(io[2], getChange(prevSum.logical_reads, sum.logical_reads));

			printf(STRING("\n\t%s%s - %s%02d:%02d.%02d\x1B[0m"), name, buf, date, sum.login_time.hour, sum.login_time.minute, sum.login_time.second);
			printf(STRING("\n\t - Connections: %s%d\x1B[0m (Running: %s%d\x1B[0m Sleeping: %s%d\x1B[0m Dormant: %s%d\x1B[0m Preconnect: %s%d\x1B[0m)\n\
\t - CPU time: %s%d\x1B[0m Memory: %s%d\x1B[0m\n\
\t - Reads: %s%d\x1B[0m Writes: %s%d\x1B[0m Memory reads: %s%d\x1B[0m\n"),
cnts[0], conns.data[h].data[p].Cnt(), cnts[1], statuses[1], cnts[2], statuses[2], cnts[3], statuses[3], cnts[4], statuses[4],
cpuMem[0], sum.cpu_time, cpuMem[1], sum.memory_usage,
io[0], (int)sum.reads, io[1], (int)sum.writes, io[2], (int)sum.logical_reads
);

			memset(buf, 0, sizeof(buf));
			memset(name, 0, sizeof(name));
		}
		printf(STRING("\n"));
	}
	printf(STRING("New connections %d, disconnections: %d"), newConnections, oldConnections);
	SetConsoleActiveScreenBuffer(hOut1);
}

void console::InitConsoleOutput(PHANDLER_ROUTINE callBack) {
	if (!SetConsoleCtrlHandler(callBack, TRUE))
		throw 0;
	hOut1 = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut1 == INVALID_HANDLE_VALUE)
		throw 0;
	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut1, &dwMode))
		throw 0;
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut1, dwMode))
		throw 0;
	GetConsoleScreenBufferInfo(hOut1, &s);
	hOut2 = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, 0, CONSOLE_TEXTMODE_BUFFER, 0);
	if (hOut2 == INVALID_HANDLE_VALUE)
		throw 0;
}

template <class T>
char console::getChange(T oldVal, T newVal) {
	if (oldVal == newVal)
		return 0;
	else if (oldVal < newVal)
		return 1;
	else
		return -1;
}