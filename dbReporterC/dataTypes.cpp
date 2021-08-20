#include "dataTypes.h"

bool Connected::isSame(const TCHAR* a, const TCHAR* b) {
	unsigned int i = 0;
	do {
		if (a[i] != b[i])
			return false;
	} while (i++ < 0xFFFFFFFF && a[i] != '\0' && b[i] != '\0');
	return true;
}

char Connected::dataRow::getStatus() {
	if (isSame(status, STRING("sleeping")))
		return 1;
	else if (isSame(status, STRING("running")))
		return 2;
	else if (isSame(status, STRING("dormant")))
		return 3;
	else if (isSame(status, STRING("preconnect")))
		return 4;
	else
		return 0;
}

void Connected::Program::Add(Connected::dataRow* row) {
	if (row->login_time < connectedAt)
		connectedAt = row->login_time;
	connections[row->getStatus()]++;
	cpuUsage += row->cpu_time;
	memoryUsage += row->memory_usage;
	logicalReads += row->logical_reads;
	reads += row->reads;
	rowCount += row->row_count;
	writes += row->writes;
}

bool Connected::operator!= (const Connected::Program& a, const Connected::Program& b) {
	if (!isSame(a.name, b.name))
		return true;
	return (a.connectedAt != b.connectedAt
		|| a.connections[0] != b.connections[0]
		|| a.connections[1] != b.connections[1]
		|| a.connections[2] != b.connections[2]
		|| a.connections[3] != b.connections[3]
		|| a.connections[4] != b.connections[4]
		|| a.cpuUsage != b.cpuUsage
		|| a.memoryUsage != b.memoryUsage
		|| a.logicalReads != b.logicalReads
		|| a.reads != b.reads
		|| a.rowCount != b.rowCount
		|| a.writes != b.writes);
}

Connected::Program* Connected::Host::Get(TCHAR* name, unsigned int size) {
	for (unsigned int i = 0; i < programs.size(); i++) {
		if (programs[i].nameSize != size)
			continue;
		bool isSame = true;
		for (uint16_t j = 0; j < size / sizeof(TCHAR); j++) {
			if (programs[i].name[j] != name[j]) {
				isSame = false;
				break;
			}
		}
		if (isSame)
			return &programs[i];
	}
	return nullptr;
}

bool Connected::operator!= (const Connected::Host& a, const Connected::Host& b) {
	if (!isSame(a.name, b.name) || a.programs.size() != b.programs.size())
		return true;
	for (USHORT i = 0; i < a.programs.size(); i++)
		if (a.programs[i] != b.programs[i])
			return true;
	return false;
}

Connected::Host* Connected::DB::Get(TCHAR* name, unsigned int size) {
	for (unsigned int i = 0; i < hosts.size(); i++) {
		if (hosts[i].nameSize != size)
			continue;
		bool isSame = true;
		for (uint16_t j = 0; j < size / sizeof(TCHAR); j++) {
			if (hosts[i].name[j] != name[j]) {
				isSame = false;
				break;
			}
		}
		if (isSame)
			return &hosts[i];
	}
	return nullptr;
}

bool Connected::operator!= (const Connected::DB& a, const Connected::DB& b) {
	if (a.hosts.size() != b.hosts.size())
		return true;
	for (USHORT i = 0; i < a.hosts.size(); i++)
		if (a.hosts[i] != b.hosts[i])
			return true;
	return false;
}

bool operator< (const TIME_STRUCT& t1, const TIME_STRUCT& t2) {
	return (t1.hour < t2.hour)
		|| (t1.hour == t2.hour && t1.minute < t2.minute)
		|| (t1.hour == t2.hour && t1.minute == t2.minute && t1.second < t2.second);
}

bool operator!= (const TIME_STRUCT& t1, const TIME_STRUCT& t2) {
	return t1.second != t2.second
		|| t1.minute != t2.minute
		|| t1.hour != t2.hour;
}

bool operator== (const TIME_STRUCT& t1, const TIME_STRUCT& t2) {
	return t1.second == t2.second
		&& t1.minute == t2.minute
		&& t1.hour == t2.hour;
}