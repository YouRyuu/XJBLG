#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <algorithm>
#include <unordered_map>
class JsonItem
{
public:
    bool parseJson(const char *begin, const char *end)
    {
        // json解析器
        bool ret = true;
        const char *start = begin;
        while (start != end && ret)
        {
            if (*start == '{' || *start == ',') //逗号和{后必跟冒号
            {
                if (*(start + 1) != '"')
                    ret = false;
                else
                    ret = true;
                if (ret)
                {
                    start = start + 2;                               //移动到key的第一个字母
                    const char *keyEnd = std::find(start, end, '"'); //找到key的结束位置
                    if (keyEnd != end)                               //取键
                    {
                        std::string key;
                        key.assign(start, keyEnd);
                        start = keyEnd + 1; // start到了冒号这里
                        if (*start != ':')
                            ret = false;
                        if (ret) //取值
                        {
                            start = start + 1;
                            if (*start == '"') //是字符串
                            {
                                start = start + 1; //到value的第一个字母
                                const char *valueEnd = std::find(start, end, '"');
                                if (valueEnd != end)
                                {
                                    std::string value;
                                    value.assign(start, valueEnd);
                                    start = valueEnd + 1; //一个键值对已经取完了
                                    jsonItem[key] = value;
                                    // printf("Key:%s, Value:%s\n", key.c_str(), value.c_str());
                                }
                                else
                                    ret = false;
                            }
                            else if (*start == '{')        //是列表
                            {
            //{"code":"1000","sender":"1234","recver":"1235","time":"2022-02-02 14:05:34","body":{"1236":"username","1237":"nameuser"}}
                                const char *valueEnd = std::find(start, end, '}');
                                if(valueEnd != end)
                                {
                                    std::string value;
                                    value.assign(start, valueEnd+1);
                                    start = valueEnd+1;
                                    jsonItem[key] = value;
                                }
                                else
                                    ret = false;
                            }
                            else //是数字
                                ret = false;
                        }
                    }
                    else
                        ret = false;
                }
            }
            else if (*start == '}')
            {
                ret = true;
                start = start + 1;
            }
            else
                ret = false;
        }
        if (*(start - 1) != '}')
            ret = false;
        if (!ret)
            jsonItem.clear();
        return ret;
    }
    std::unordered_map<std::string, std::string> jsonItem;
};

// std::string getNowTime()
// {
//     time_t now = time(0);
//     tm *ltm = localtime(&now);
//     std::string year = std::to_string(1900 + ltm->tm_year) + "/";
//     std::string month = std::to_string(1 + ltm->tm_mon) + "/";
//     std::string day = std::to_string(ltm->tm_mday) + " ";
//     std::string hour = std::to_string(ltm->tm_hour) + ":";
//     std::string mins = std::to_string(ltm->tm_min) + ":";
//     std::string sec = std::to_string(ltm->tm_sec);
//     return year + month + day + hour + mins + sec;
// }

#endif