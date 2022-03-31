#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "iterator.h"
#include <string.h>

namespace lstl
{


//fill类函数
//填充[first, end)为value
template <class ForwardIter, class T>
void fill(ForwardIter first, ForwardIter end, const T &value)
{
    for(; first!=end; ++first)
    {
        *first = value;
    }
}

//从first起的size个元素填充为value
template <class OutputIter, class Size, class T>
OutputIter fill_n(OutputIter first, Size size, const T &value)
{
    for(; size>0; ++first, --size)
    {
        *first = value;
    }
    return first;
}


//特例化版本
inline void fill(unsigned char *first, unsigned char *end, const unsigned char &c)
{
    unsigned char v = c;
    memset(first, v, end-first);
}

inline void fill(signed char* first, signed char* end,
                 const signed char &c) {
  signed char v = c;
  memset(first, static_cast<unsigned char>(v), end - first);
}

inline void fill(char* first, char* end, const char &c) {
  char v = c;
  memset(first, static_cast<unsigned char>(v), end - first);
}

template <class Size>
inline unsigned char *fill_n(unsigned char *first, Size size, const unsigned char &c)
{
    fill(first, first+size, c);
    return first + size;
}

template <class Size>
inline signed char *fill_n(signed char *first, Size size, const signed char &c)
{
    fill(first, first+size, c);
    return first + size;
}

template <class Size>
inline char *fill_n(char *first, Size size, const char &c)
{
    fill(first, first+size, c);
    return first + size;
}

//copy类函数
template <class InputIter, class OutputIter, class Distance>
inline OutputIter copy_aux(InputIter first, InputIter end, OutputIter ret, input_iterator_tag, Distance *)
{
    for(; first!=end; ++first, ++ret)
    {
        *ret = *first;
    }
    return ret;
}

template <class InputIter, class OutputIter, class Distance>
inline OutputIter copy_aux(InputIter first, InputIter end, OutputIter ret, random_iterator_tag, Distance *)
{
    for(Distance n = end - first; n>0; --n)
    {
        *ret = *first;
        ++first;
        ++ret;
    }
    return ret;
}

template <class InputIter, class OutputIter>
inline OutputIter copy(InputIter first, InputIter end, OutputIter ret)
{
    return copy_aux(first, end, ret, iterator_category(first), distance_type(first));
}

//内置内型
#define DECLARE_COPY_TRIVIAL(T)     \
inline T* copy(const T *first, const T *end, T *ret){   \
    memmove(ret, first, sizeof(T)*(end-first));         \
    return ret+(end-first);                             \
    }

DECLARE_COPY_TRIVIAL(char)
DECLARE_COPY_TRIVIAL(signed char)
DECLARE_COPY_TRIVIAL(unsigned char)
DECLARE_COPY_TRIVIAL(short)
DECLARE_COPY_TRIVIAL(unsigned short)
DECLARE_COPY_TRIVIAL(int)
DECLARE_COPY_TRIVIAL(unsigned int)
DECLARE_COPY_TRIVIAL(long)
DECLARE_COPY_TRIVIAL(unsigned long)
DECLARE_COPY_TRIVIAL(float)
DECLARE_COPY_TRIVIAL(double)
DECLARE_COPY_TRIVIAL(long double)

//copy_backward类函数
template <class BidirectionalIter1, class BidirectionalIter2, class Distance>
inline BidirectionalIter2 copy_backward_aux(BidirectionalIter1 first, BidirectionalIter1 end, BidirectionalIter2 ret, bidirectional_iterator_tag, Distance*)
{
    while (first!=end)
    {
        *--ret = *--end;
    }
    return ret;
}

template <class RandomIter, class BidirectionalIter, class Distance>
inline BidirectionalIter copy_backward_aux(RandomIter first, RandomIter end, BidirectionalIter ret, random_iterator_tag, Distance*)
{
    for(Distance n = end - first; n>0; --n)
    {
        *--ret = *--end;
    }
    return ret;
}

template <class BI1, class BI2>
inline BI2 copy_backward(BI1 first, BI1 end, BI2 ret)
{
    return copy_backward_aux(first, end, ret, iterator_category(first), distance_type(first));
}

}       //end of namespace
#endif