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

void console::print(Connected::DB conns, Connected::DB prevConns) {
	clear_screen(' ');
	for (unsigned int h = 0; h < conns.hosts.size(); h++) {
		bool newHost = false;
		TCHAR buf[NAME_LEN] = { '\0' };
		TCHAR name[6] = { '\0' };

		if (!(h < prevConns.hosts.size())) {
			newHost = true;
			prevConns.hosts.push_back(Connected::Host());
		}

		memcpy(buf, conns.hosts[h].name, conns.hosts[h].nameSize);
		addColor(name, newHost);
		printf(STRING("%s%s"), name, buf);

		memset(buf, 0, sizeof(buf));
		memset(name, 0, sizeof(name));

		for (unsigned int p = 0; p < conns.hosts[h].programs.size(); p++) {
			bool newProg = false;

			if (!(p < prevConns.hosts[h].programs.size())) {
				newProg = true;
				prevConns.hosts[h].programs.push_back(Connected::Program());
			}

			prevConns.hosts[h].programs[p].connections[0] = prevConns.hosts[h].programs[p].Cnt() - conns.hosts[h].programs[p].Cnt();

			memcpy(buf, conns.hosts[h].programs[p].name, conns.hosts[h].programs[p].nameSize);
			addColor(name, newProg);

			TCHAR date[6] = { '\0' };
			addColor(date, getChange(prevConns.hosts[h].programs[p].connectedAt, conns.hosts[h].programs[p].connectedAt));

			TCHAR cnts[6][6] = { {'\0'} };
			addColor(cnts[0], getChange(prevConns.hosts[h].programs[p].Cnt(), conns.hosts[h].programs[p].Cnt()));
			addColor(cnts[1], getChange(prevConns.hosts[h].programs[p].connections[1], conns.hosts[h].programs[p].connections[1]));
			addColor(cnts[2], getChange(prevConns.hosts[h].programs[p].connections[2], conns.hosts[h].programs[p].connections[2]));
			addColor(cnts[3], getChange(prevConns.hosts[h].programs[p].connections[3], conns.hosts[h].programs[p].connections[3]));
			addColor(cnts[4], getChange(prevConns.hosts[h].programs[p].connections[4], conns.hosts[h].programs[p].connections[4]));
			addColor(cnts[5], getChange(prevConns.hosts[h].programs[p].connections[0], conns.hosts[h].programs[p].connections[0]));

			TCHAR cpuMem[2][6] = { {'\0'} };
			addColor(cnts[0], getChange(prevConns.hosts[h].programs[p].cpuUsage, conns.hosts[h].programs[p].cpuUsage));
			addColor(cnts[1], getChange(prevConns.hosts[h].programs[p].memoryUsage, conns.hosts[h].programs[p].memoryUsage));

			TCHAR io[4][6] = { {'\0'} };
			addColor(io[0], getChange(prevConns.hosts[h].programs[p].reads, conns.hosts[h].programs[p].reads));
			addColor(io[1], getChange(prevConns.hosts[h].programs[p].writes, conns.hosts[h].programs[p].writes));
			addColor(io[2], getChange(prevConns.hosts[h].programs[p].logicalReads, conns.hosts[h].programs[p].logicalReads));
			addColor(io[3], getChange(prevConns.hosts[h].programs[p].rowCount, conns.hosts[h].programs[p].rowCount));

			printf(STRING("\n\t%s%s - %s%02d:%02d.%02d\x1B[0m"), name, buf, date, conns.hosts[h].programs[p].connectedAt.hour, conns.hosts[h].programs[p].connectedAt.minute, conns.hosts[h].programs[p].connectedAt.second);
			printf(STRING("\n\t - Connections: %s%d\x1B[0m (Running: %s%d\x1B[0m Sleeping: %s%d\x1B[0m Dormant: %s%d\x1B[0m Preconnect: %s%d\x1B[0m, Disconnected: %s%d)\n\
\t - CPU time: %s%d\x1B[0m Memory: %s%d\x1B[0m\n\
\t - Reads: %s%d\x1B[0m Writes: %s%d\x1B[0m Memory reads: %s%d\x1B[0m\n"),
cnts[0], conns.hosts[h].programs[p].Cnt(),
cnts[1], conns.hosts[h].programs[p].connections[2],
cnts[2], conns.hosts[h].programs[p].connections[1],
cnts[3], conns.hosts[h].programs[p].connections[3],
cnts[4], conns.hosts[h].programs[p].connections[4],
cnts[5], conns.hosts[h].programs[p].connections[0],
cpuMem[0], conns.hosts[h].programs[p].cpuUsage,
cpuMem[1], conns.hosts[h].programs[p].memoryUsage,
io[0], (int)conns.hosts[h].programs[p].reads,
io[1], (int)conns.hosts[h].programs[p].writes,
io[2], (int)conns.hosts[h].programs[p].logicalReads,
io[3], (int)conns.hosts[h].programs[p].rowCount
);

			memset(buf, 0, sizeof(buf));
			memset(name, 0, sizeof(name));
		}
		printf(STRING("\n"));
	}
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