//参考《STL源码剖析》一书
// construct
//负责对象的构造和析构

#include <new>
#include "type_traits.h"

namespace lstl
{

    template <class T1, class T2>
    inline void construct(T1 *p, const T2 &value)
    {
        new ((void *)p) T1(value); // placement new
    }

    template <class T>
    inline void construct(T *p)
    {
        new ((void *)p) T();
    }

    template <class T>
    inline void destroy(T *p)
    {
        p->~T();
    }

    template <class Iterator>
    inline void destroy_aux(Iterator first, Iterator end, false_type)
    {
        for (; first != end; ++first)
        {
            destroy(&*first);
        }
    }

    template <class Iterator>
    inline void destroy_aux(Iterator first, Iterator end, true_type)
    {
    }

    template <class Iterator, class T>
    inline void destroy(Iterator first, Iterator end, T *)
    {
        typedef typename type_traits<T>::has_trivial_destructor trivial_destructor;
        destroy_aux(first, end, trivial_destructor());
    }

    template <class Iterator>
    inline void destroy(Iterator first, Iterator end)
    {
        destroy(first, end, value_type(first));
    }

    inline void destroy(char *, char *) {}
    inline void destroy(int *, int *) {}
    inline void destroy(long *, long *) {}
    inline void destroy(float *, float *) {}
    inline void destroy(double *, double *) {}

}