#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
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

bool isSame(const TCHAR* a, const TCHAR* b);

struct dbConn
{
	__int16 id = 0;
	unsigned short cpu_time = 0;
	unsigned short memory_usage = 0;
	unsigned short open_transaction_count = 0;
	unsigned int logical_reads = 0;
	unsigned int reads = 0;
	unsigned int row_count = 0;
	unsigned int writes = 0;
	TIME_STRUCT login_time = TIME_STRUCT();
	TIME_STRUCT last_request_start_time = TIME_STRUCT();
	char connStatus = 0; //1 = 'R', 'S', 'D', 'P'
	dbConn& operator+=(const dbConn& a);
	static char getStatus(const TCHAR* str);
};
bool operator!= (const dbConn& a, const dbConn& b);
bool operator== (const TIME_STRUCT& t1, const TIME_STRUCT& t2);
bool operator< (const TIME_STRUCT& t1, const TIME_STRUCT& t2);

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
	dynamicDbTree<T>() {
		data = std::vector<T>();
		data.reserve(allocLen);
	}
	~dynamicDbTree<T>() {
	}
	T* AllocNew() {
		if (allocLen <= cnt || cnt < allocLen - 6) {
			allocLen = cnt + 5;
			data.reserve(allocLen);
		}
		data.push_back(T());
		return &data[cnt++];
	}

	virtual void Clear() = 0;

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
		allocLen = obj.allocLen;
		data = obj.data;
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
	void Clear() {
		data.clear();
		cnt = 0;
	}
};

class host : public dynamicDbTree<program>
{
public:
	bool Contains(TCHAR* name, unsigned int size) { return Get(name, size) != nullptr; }
	program* Get(TCHAR* name, unsigned int size);
	void Clear() {
		for (program& p : data)
			p.Clear();
		data.clear();
	}
};

class dbConnections : public dynamicDbTree<host>
{
public:
	bool Contains(TCHAR* name, unsigned int size) { return Get(name, size) != nullptr; }
	host* Get(TCHAR* name, unsigned int size);
	void Clear() {
		for (host& h : data)
			h.Clear();
		data.clear();
	}
};