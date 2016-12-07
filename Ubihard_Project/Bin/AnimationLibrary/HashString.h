#pragma once
#include <map>
#include <string>

class HashString
{
private:
	static HashString* singleton;
	std::map<std::string, unsigned int> hashTable;
	unsigned int index = 0;

public:
	//~HashString()
	//{
	//	delete singleton;
	//	singleton = nullptr;
	//}

	static HashString* GetSingleton() { if (!singleton) { singleton = new HashString(); } return singleton; }
	unsigned int GetKey(std::string key);
	void Insert(std::string key) { hashTable[key] = index++; }
	void CleanUp() { delete singleton; singleton = nullptr; }
};