#include "dbConn.h"

dbConn& dbConn::operator+=(const dbConn& a)
{
	id++;
	if (connStatus == 0 || a.connStatus > connStatus)
		connStatus = a.connStatus;
	cpu_time += a.cpu_time;
	memory_usage += a.memory_usage;
	open_transaction_count += a.open_transaction_count;
	logical_reads += a.logical_reads;
	reads += a.reads;
	row_count += a.row_count;
	writes += a.writes;
	if (login_time == TIME_STRUCT())
		login_time = a.login_time;
	else if (a.login_time < login_time)
		login_time = a.login_time;
	if (last_request_start_time == TIME_STRUCT())
		last_request_start_time = a.last_request_start_time;
	if (a.last_request_start_time < last_request_start_time)
		last_request_start_time = a.last_request_start_time;
	return *this;
}

char dbConn::getStatus(const TCHAR* str) {
	if (isSame(str, STRING("sleeping")))
		return 1;
	else if (isSame(str, STRING("running")))
		return 2;
	else if (isSame(str, STRING("dormant")))
		return 3;
	else if (isSame(str, STRING("preconnect")))
		return 4;
	else
		return 0;
}

bool isSame(const TCHAR* a, const TCHAR* b) {
	unsigned int i = 0;
	do {
		if (a[i] != b[i])
			return false;
	} while (i++ < 0xFFFFFFFF && a[i] != '\0' && b[i] != '\0');
	return true;
}

bool operator< (const TIME_STRUCT& t1, const TIME_STRUCT& t2) {
	return (t1.hour < t2.hour)
		|| (t1.hour == t2.hour && t1.minute < t2.minute)
		|| (t1.hour == t2.hour && t1.minute == t2.minute && t1.second < t2.second)
		|| (t1.hour == t2.hour && t1.minute == t2.minute && t1.second < t2.second);
	//return (t1.year < t2.year)
	//	|| (t1.year == t2.year && t1.month < t2.month)
	//	|| (t1.year == t2.year && t1.month == t2.month && t1.day < t2.day)
	//	|| (t1.year == t2.year && t1.month == t2.month && t1.day == t2.day && t1.hour < t2.hour)
	//	|| (t1.year == t2.year && t1.month == t2.month && t1.day == t2.day && t1.hour == t2.hour && t1.minute < t2.minute)
	//	|| (t1.year == t2.year && t1.month == t2.month && t1.day == t2.day && t1.hour == t2.hour && t1.minute == t2.minute && t1.second < t2.second)
	//	|| (t1.year == t2.year && t1.month == t2.month && t1.day == t2.day && t1.hour == t2.hour && t1.minute == t2.minute && t1.second = t2.second && t1.fraction < t2.fraction);
}

bool operator== (const TIME_STRUCT& t1, const TIME_STRUCT& t2) {
	return t1.second == t2.second
		&& t1.minute == t2.minute
		&& t1.hour == t2.hour;
	//return t1.fraction == t2.fraction
	//	&& t1.second == t2.second
	//	&& t1.minute == t2.minute
	//	&& t1.hour == t2.hour
	//	&& t1.day == t2.day
	//	&& t1.month == t2.month
	//	&& t1.year == t2.year;
}

bool operator!= (const dbConn& a, const dbConn& b) {
	return a.id != b.id
		|| a.connStatus != b.connStatus
		|| a.cpu_time != b.cpu_time
		|| a.memory_usage != b.memory_usage
		|| a.open_transaction_count != b.open_transaction_count
		|| a.logical_reads != b.logical_reads
		|| a.reads != b.reads
		|| a.row_count != b.row_count
		|| a.writes != b.writes
		|| !(a.login_time == b.login_time);
	//|| a.last_request_start_time != b.last_request_start_time;
}

dbConn* program::Get(__int16 id) {
	for (unsigned int i = 0; i < cnt; i++) {
		if (data[i].id == id)
			return &data[i];
	}
	return nullptr;
}

program* host::Get(TCHAR* name, unsigned int size) {
	for (unsigned int i = 0; i < cnt; i++) {
		if (data[i].nameSize != size)
			continue;
		bool isSame = true;
		for (uint16_t j = 0; j < size / sizeof(TCHAR); j++) {
			if (data[i].name[j] != name[j]) {
				isSame = false;
				break;
			}
		}
		if (isSame)
			return &data[i];
	}
	return nullptr;
}

host* dbConnections::Get(TCHAR* name, unsigned int size) {
	for (unsigned int i = 0; i < cnt; i++) {
		if (data[i].nameSize != size)
			continue;
		bool isSame = true;
		for (uint16_t j = 0; j < size / sizeof(TCHAR); j++) {
			if (data[i].name[j] != name[j]) {
				isSame = false;
				break;
			}
		}
		if (isSame)
			return &data[i];
	}
	return nullptr;
}