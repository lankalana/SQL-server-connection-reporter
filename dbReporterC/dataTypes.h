#pragma once

#include <windows.h>
#include <sql.h>

#include <vector>

#ifdef UNICODE
#define STRING(x) L ##x
#define ttoi _wtoi
#else
#define STRING(x) x
#define ttoi atoi
#endif // UNICODE

#ifndef NAME_LEN
#define NAME_LEN 256
#endif // !NAME_LEN

namespace Connected
{
	bool isSame(const TCHAR* a, const TCHAR* b);

	struct dataRow {
		__int16 id = 0;
		TCHAR status[60] = { 0 };
		TCHAR host_name[NAME_LEN] = { 0 };
		TCHAR program_name[NAME_LEN] = { 0 };
		SQLLEN host_nameSize = NAME_LEN;
		SQLLEN program_nameSize = NAME_LEN;
		unsigned short cpu_time = 0;
		unsigned short memory_usage = 0;
		unsigned short open_transaction_count = 0;
		unsigned int logical_reads = 0;
		unsigned int reads = 0;
		unsigned int row_count = 0;
		unsigned int writes = 0;
		TIME_STRUCT login_time = TIME_STRUCT();

		char getStatus();
	};

	struct Program {
		TCHAR name[NAME_LEN];
		unsigned short nameSize;
		TIME_STRUCT connectedAt;
		unsigned short connections[5];
		unsigned short cpuUsage = 0;
		unsigned short memoryUsage = 0;
		unsigned int logicalReads = 0;
		unsigned int reads = 0;
		unsigned int rowCount = 0;
		unsigned int writes = 0;

		void Add(dataRow* row);
		inline unsigned short Cnt() {
			return connections[1] + connections[2] + connections[3] + connections[4];
		}
		friend bool operator!= (const Program& a, const Program& b);
	};

	struct Host {
		TCHAR name[NAME_LEN];
		unsigned short nameSize;
		Program* Get(TCHAR* name, unsigned int size);
		friend bool operator!= (const Host& a, const Host& b);
		std::vector<Program> programs;
	};

	struct DB {
		std::vector<Host> hosts;
		Host* Get(TCHAR* name, unsigned int size);
		friend bool operator!= (const DB& a, const DB& b);
	};
}

bool operator< (const TIME_STRUCT& t1, const TIME_STRUCT& t2);
bool operator!= (const TIME_STRUCT& t1, const TIME_STRUCT& t2);
bool operator== (const TIME_STRUCT& t1, const TIME_STRUCT& t2);