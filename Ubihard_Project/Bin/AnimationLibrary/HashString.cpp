#include "HashString.h"

HashString* HashString::singleton = 0;

unsigned int HashString::GetKey(std::string key)
{ 
	unsigned int result = -1;

	key += '\0';

	auto stuff = hashTable.find(key);

	if (stuff != hashTable.end())
	{ 
		result =  hashTable[key];
	}

	return result;
}