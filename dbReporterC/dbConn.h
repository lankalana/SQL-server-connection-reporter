#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <sql.h>

#include <vector>

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

#ifndef NAME_LEN
#define NAME_LEN 512
#endif // !NAME_LEN

bool isSame(const TCHAR* a, const TCHAR* b);

struct dbConn
{
	__int16 id = 0;
	enum class status { NA = 0, Running, Sleeping, Dormant, Preconnect } connStatus = status::NA;
	long cpu_time = 0;
	long memory_usage = 0;
	long open_transaction_count = 0;
	__int64 logical_reads = 0;
	__int64 reads = 0;
	__int64 row_count = 0;
	__int64 writes = 0;
	TIMESTAMP_STRUCT login_time = TIMESTAMP_STRUCT();
	TIMESTAMP_STRUCT last_request_start_time = TIMESTAMP_STRUCT();
	dbConn& operator+=(const dbConn& a);
	static status getStatus(const TCHAR* str);
};
bool operator!= (const dbConn& a, const dbConn& b);
bool operator== (const TIMESTAMP_STRUCT& t1, const TIMESTAMP_STRUCT& t2);
bool operator< (const TIMESTAMP_STRUCT& t1, const TIMESTAMP_STRUCT& t2);

template<class T>
class dynamicDbTree
{
protected:
	unsigned int cnt = 0;
	unsigned int allocLen = 3;
public:
	TCHAR name[NAME_LEN] = { 0 };
	int nameSize = 0;
	std::vector<T> data;
	dynamicDbTree<T>() { data = std::vector<T>(allocLen); }
	~dynamicDbTree<T>() {
		//if (data != nullptr) {
			//delete[] data;
			//data = nullptr;
		//}
	}
	void Add(T item) {
		if (allocLen <= cnt) {
			allocLen += 5;
			std::vector<T> temp = std::vector<T>(allocLen);
			for (unsigned int i = 0; i < cnt; i++)
				temp[i] = data[i];
			//delete[] data;
			data = temp;
		}
		data[cnt++] = item;
	}
	bool Contains(T item) { return Get(item) != nullptr; }
	T* Get(T item) {
		for (unsigned int i = 0; i < cnt; i++) {
			if (data[i] == item)
				return &data[i];
		}
		return nullptr;
	}
	unsigned int Cnt() { return cnt; }

	dynamicDbTree<T>& operator=(const dynamicDbTree<T>& obj) {
		cnt = obj.cnt;
		if (allocLen != obj.allocLen) {
			allocLen = obj.allocLen;
			//delete[] data;
			data.resize(allocLen);// = new T[obj.allocSize];
		}
		for (unsigned int i = 0; i < allocLen; i++)
			data[i] = obj.data[i];
		memcpy(name, obj.name, obj.nameSize);
		nameSize = obj.nameSize;
		return *this;
	}
	friend bool operator!= (const dynamicDbTree<T>& a, const dynamicDbTree<T>& b) {
		if (!isSame(a.name, b.name) || a.cnt != b.cnt)
			return true;
		for (unsigned int i = 0; i < a.cnt; i++) {
			if (a.data[i] != b.data[i])
				return true;
		}
		return false;
	}
};

class program : public dynamicDbTree<dbConn>
{
public:
	bool Contains(__int16 id) { return Get(id) != nullptr; }
	dbConn* Get(__int16 id);
};

class host : public dynamicDbTree<program>
{
public:
	bool Contains(TCHAR* name, unsigned int size) { return Get(name, size) != nullptr; }
	program* Get(TCHAR* name, unsigned int size);
};

class dbConnections : public dynamicDbTree<host>
{
public:
	bool Contains(TCHAR* name, unsigned int size) { return Get(name, size) != nullptr; }
	host* Get(TCHAR* name, unsigned int size);
};