#include <string>
#include <algorithm>
bool parseJson(const char *begin, const char *end)
{
    // json解析器
    /*
     *{后必定接一个"
     *{"code":"1000","sender":"1234","recver":"1235","time":"2022-02-02 14:05:34","body":"hello"}
     *{"code":"1000","sender":"1234","recver":"1235","time":"2022-02-02 14:05:34","body":[{"1236":"username"},{"1237":"nameuser"}]}
     *{"code":"1000","sender":"1234","recver":"1235","time":"2022-02-02 14:05:34","body":["111","222"]}
     */
    bool ret = true;
    const char *start = begin;
    while (start != end && ret)
    {
        if (*start == '{' || *start == ',')
        {
            if (*(start + 1) != '"')
                ret = false;
            else ret = true;
            if (ret)
            {
                start = start + 2;
                const char *keyEnd = std::find(start, end, '"');
                if (keyEnd != end)      //取键
                {
                    std::string key;
                    key.assign(start, keyEnd);
                    start = keyEnd + 1;
                    if (*start != ':')
                        ret = false;
                    if (ret)        //取值  
                    {
                        start = start + 1;
                        if (*start == '"')      //是字符串
                        {
                            start = start + 1;
                            const char *valueEnd = std::find(start, end, '"');
                            if (valueEnd != end)
                            {
                                std::string value;
                                value.assign(start, valueEnd);
                                start = valueEnd + 1;       //一个键值对已经取完了
                                printf("Key:%s, Value:%s\n", key.c_str(), value.c_str());
                            }
                            else
                                ret = false;
                        }
                        else if (*start == '[')
                        {
                        }
                        else
                            ret = false;
                    }
                }
                else
                    ret = false;
            }
        }
        else if(*start == '}')
            {
                ret = true;
                start = start + 1;
            }
        else
            ret = false;
    }
    return ret;
}