//参考《STL源码剖析》一书
// iterator
//迭代器的定义

#ifndef ITERATOR_H
#define ITERATOR_H

#include <new>

namespace lstl
{

    struct input_iterator_tag
    {
    };
    struct output_iterator_tag
    {
    };
    struct forward_iterator_tag : public input_iterator_tag
    {
    };
    struct bidirectional_iterator_tag : public forward_iterator_tag
    {
    };
    struct random_iterator_tag : public bidirectional_iterator_tag
    {
    };

    template <class T, class Distance>
    struct input_iterator
    {
        typedef input_iterator_tag iterator_catagory;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;
        typedef Distance difference_type;
    };

    struct output_iterator
    {
        typedef output_iterator_tag iterator_category;
        typedef void value_type;
        typedef void difference_type;
        typedef void pointer;
        typedef void reference;
    };

    template <class T, class Distance>
    struct forward_iterator
    {
        typedef forward_iterator_tag iterator_category;
        typedef T value_type;
        typedef Distance difference_type;
        typedef T *pointer;
        typedef T &reference;
    };

    template <class T, class Distance>
    struct bidirectional_iterator
    {
        typedef bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef Distance difference_type;
        typedef T *pointer;
        typedef T &reference;
    };

    template <class T, class Distance>
    struct random_iterator
    {
        typedef random_iterator_tag iterator_category;
        typedef T value_type;
        typedef Distance difference_type;
        typedef T *pointer;
        typedef T &reference;
    };

    template <class Category, class T, class Distance = ptrdiff_t, class Pointer = T *, class Reference = T &>
    class iterator
    {
    public:
        typedef Category iterator_category;
        typedef T value_type;
        typedef Distance difference_type;
        typedef Pointer pointer;
        typedef Reference reference;
    };

    template <class Iterator>
    class iterator_traits
    {
    public:
        typedef typename Iterator::iterator_category iterator_category;
        typedef typename Iterator::value_type value_type;
        typedef typename Iterator::difference_type difference_type;
        typedef typename Iterator::pointer pointer;
        typedef typename Iterator::reference reference;
    };

    template <class T>
    class iterator_traits<T *>
    {
    public:
        typedef random_iterator_tag iterator_category;
        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef T *pointer;
        typedef T &reference;
    };

    template <class T>
    class iterator_traits<const T *>
    {
    public:
        typedef random_iterator_tag iterator_category;
        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef const T *pointer;
        typedef const T &reference;
    };

    template <class Iterator>
    typename iterator_traits<Iterator>::iterator_category
    iterator_category(const Iterator &)
    {
        typedef typename iterator_traits<Iterator>::iterator_category category;
        return category();
    }

    template <class Iterator>
    typename iterator_traits<Iterator>::value_type *
    value_type(const Iterator &)
    {
        return static_cast<typename iterator_traits<Iterator>::value_type *>(0);
    }

    template <class Iterator>
    typename iterator_traits<Iterator>::difference_type *
    distance_type(const Iterator &)
    {
        return static_cast<typename iterator_traits<Iterator>::difference_type *>(0);
    }

    template <class InputIterator>
    typename iterator_traits<InputIterator>::difference_type
    distance_patch(InputIterator first, InputIterator end, input_iterator_tag)
    {
        typename iterator_traits<InputIterator>::difference_type n = 0;
        while (first != end)
        {
            ++n;
            ++first;
        }
        return n;
    }

    template <class RandomIterator>
    typename iterator_traits<RandomIterator>::difference_type
    distance_patch(RandomIterator first, RandomIterator end, random_iterator_tag)
    {
        return end - first;
    }

    template <class InputIterator>
    typename iterator_traits<InputIterator>::difference_type
    distance(InputIterator first, InputIterator end)
    {
        typedef typename iterator_traits<InputIterator>::iterator_category category;
        return distance_patch(first, end, category());
    }

    template<class InputIterator, class Distance>
    void advance_patch(InputIterator &i, Distance n ,input_iterator_tag)
    {
        while(n--)
        {
            ++i;
        }
    }

    template<class BidirectionalIterator, class Distance>
    void advance_patch(BidirectionalIterator &i, Distance n, bidirectional_iterator_tag)
    {
        if(n>=0)
        {
            while(n--)
                ++i;
        }
        else
        {
            while(n++)
                --i;
        }
    }

    template<class RandomIterator, class Distance>
    void advance_patch(RandomIterator &i, Distance n, random_iterator_tag)
    {
        i+=n;
    }

    template<class InputIterator, class Distance>
    void advance(InputIterator &i, Distance n)
    {
        advance_patch(i, n, iterator_category(i));
    }

}

#endif
