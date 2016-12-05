#pragma once
#include <map>
#include <string>

class HashString
{
private:
	std::map<std::string, unsigned int> hashTable;
	unsigned int index = 0;

public:
	unsigned int GetKey(std::string key) { return hashTable[key]; }
	void Insert(std::string key) { hashTable[key] = index++; }
};