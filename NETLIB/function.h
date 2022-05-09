#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <algorithm>
#include <unordered_map>
class JsonItem
{
public:
    bool parseJson(const char *begin, const char *end);
    std::unordered_map<std::string, std::string> jsonItem;
};

std::string getNowTime();

std::string getSeq();

#endif