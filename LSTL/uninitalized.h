#include <exception>
#include <string.h>
namespace lstl
{
    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_copy_aux(InputIter first, InputIter end, ForwardIter ret, true_type)
    {
        return copy(first, end, ret);
    }

    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_copy_aux(InputIter first, InputIter end, ForwardIter ret, false_type)
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
        }
    }

    template <class InputIter, class ForwardIter, class T>
    ForwardIter uninitialized_copy(InputIter first, InputIter end, ForwardIter ret, T *)
    {
        typedef typename type_traits<T>::is_POD_type isPod;
        return uninitialized_copy_aux(first, end, ret, isPod());
    }

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
}