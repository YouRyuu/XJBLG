#include <exception>
#include <string.h>
#include <algorithm>
#include "type_traits.h"
#include "iterator.h"
#ifndef UNINITALIZED_H
#define UNINITALIZED_H

namespace lstl
{

    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_copy_aux(InputIter first, InputIter end, ForwardIter ret, _true_type)
    {
        return copy(first, end, ret);
    }

    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_copy_aux(InputIter first, InputIter end, ForwardIter ret, _false_type)
    {
        ForwardIter curr = ret;
        try
        {
            for (; first != end; ++first, ++curr)
            {
                construct(&*curr, *first);
            }
            return curr;
        }
        catch (std::exception e)
        {
            destroy(ret, curr);
            throw;
        }
    }

    template <class InputIter, class ForwardIter, class T>
    ForwardIter uninitialized_copy(InputIter first, InputIter end, ForwardIter ret, T *)
    {
        typedef typename type_traits<T>::is_POD_type isPod;
        return uninitialized_copy_aux(first, end, ret, isPod());
    }

    //把从[first, end)的值复制到ret开始的区域内
    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_copy(InputIter first, InputIter end, ForwardIter ret)
    {
        return uninitialized_copy(first, end, ret, value_type(ret));
    }

    char *uninitialized_copy(const char *first, const char *end,
                             char *ret)
    {
        memmove(ret, first, end - first);
        return ret + (end - first);
    }

    template <class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_copy_n_aux(InputIter first, Size size, ForwardIter ret, input_iterator_tag)
    {
        ForwardIter cur = ret;
        try
        {
            for (; size > 0; --size, ++first, ++cur)
            {
                construct(&*cur, *first);
            }
            return cur;
        }
        catch (std::exception e)
        {
            destroy(ret, cur);
            throw;
        }
    }

    template <class RandomIter, class Size, class ForwardIter>
    ForwardIter uninitialized_copy_n_aux(RandomIter first, Size size, ForwardIter ret, random_iterator_tag)
    {
        RandomIter end = first + size;
        return uninitialized_copy(first, end, ret);
    }

    template <class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_copy_n(InputIter first, Size size, ForwardIter ret)
    {
        return uninitialized_copy_n_aux(first, size, ret, iterator_category(first));
    }

    template <class ForwardIter, class T>
    void uninitialized_fill_aux(ForwardIter first, ForwardIter end, const T &x, _true_type)
    {
        fill(first, end, x);
    }

    template <class ForwardIter, class T>
    void uninitialized_fill_aux(ForwardIter first, ForwardIter end, const T &x, _false_type)
    {
        ForwardIter cur = first;
        try
        {
            for (; cur != end; ++cur)
                construct(&*cur, x);
        }
        catch (std::exception e)
        {
            destory(first, cur);
            throw;
        }
    }

    template <class ForwardIter, class T, class T1>
    void _uninitialized_fill(ForwardIter first, ForwardIter end, const T &x, T1 *)
    {
        typedef typename type_traits<T1>::is_POD_type is_POD;
        uninitialized_fill_aux(first, end, x, is_POD());
    }

    //把从[first, end)范围内的空间填充数据x
    template <class ForwardIter, class T>
    void uninitialized_fill(ForwardIter first, ForwardIter end, const T &x)
    {
        _uninitialized_fill(first, end, x, value_type(first));
    }

    template <class ForwardIter, class Size, class T>
    ForwardIter uninitialized_fill_n_aux(ForwardIter first, Size size, const T &x, _true_type)
    {
        return fill_n(first, size, x);
    }

    template <class ForwardIter, class Size, class T>
    ForwardIter uninitialized_fill_n_aux(ForwardIter first, Size size, const T &x, _false_type)
    {
        ForwardIter cur = first;
        try
        {
            for (; size > 0; --size, ++cur)
            {
                construct(&*cur, x);
            }
            return cur;
        }
        catch (std::exception e)
        {
            destroy(first, cur);
            throw;
        }
    }

    template <class ForwardIter, class Size, class T, class T1>
    ForwardIter _uninitialized_fill_n(ForwardIter first, Size size, const T &x, T1 *)
    {
        typedef typename type_traits<T1>::is_POD_type is_POD;
        return uninitialized_fill_n_aux(first, size, x, is_POD());
    }

    template <class ForwardIter, class Size, class T>
    ForwardIter uninitialized_fill_n(ForwardIter first, Size size, const T &x)
    {
        return _uninitialized_fill_n(first, size, x, value_type(first));
    }

} // end of namespace

#endif