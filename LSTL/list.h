#ifndef LIST_H
#define LIST_H

#include "alloc.h"
#include "algorithm.h"
#include "construct.h"
#include "uninitalized.h"

namespace lstl
{

    // list,双向链表
    template <class T>
    struct listNode
    {
        listNode<T> *next;
        listNode<T> *prev;
        T data;
    };

    //自定义迭代器类型
    template <class T, class Ref, class Ptr>
    struct listIterator
    {
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef Ptr pointer;
        typedef Ref reference;
        typedef listIterator<T, T &, T *> iterator;
        typedef listIterator<T, const T &, const T *> const_iterator;
        typedef listIterator<T, T &, T *> self;
        typedef listNode<T> *link_type;
        link_type node;
        //构造函数
        listIterator(){};
        listIterator(link_type x) : node(x) {}
        listIterator(const iterator x) : node(x.node) {}
        //*操作取的是迭代器里面的值，也就是listNode里的data
        reference operator*() const
        {
            return node->data;
        }
        bool operator==(const self &x)
        {
            return x.node == node;
        }
        bool operator!=(const self &x)
        {
            return x.node != node;
        }

        //迭代器x进行->操作，如：x->p(),实际上进行的是(x.data)->p(),所以先取到data的地址，再进行操作
        pointer operator->() const
        {
            return &(operator*());
        }
        self &operator++()
        {
            node = (link_type)(node->next);
            return *this;
        }
        self &operator++(int)
        {
            self temp = *this;
            ++*this;
            return temp;
        }
        self &operator--()
        {
            node = (link_type)(node->prev);
            return node;
        }
        self operator--(int)
        {
            self temp = *this;
            --*this;
            return temp;
        }
    };

    template <class T, class Ref, class Ptr>
    inline bidirectional_iterator_tag iterator_category(const listIterator<T, Ref, Ptr> &)
    {
        return bidirectional_iterator_tag();
    }

    template <class T, class Ref, class Ptr>
    inline T *value_type(const listIterator<T, Ref, Ptr> &)
    {
        return 0;
    }

    template <class T, class Ref, class Ptr>
    inline ptrdiff_t *distance_type(const listIterator<T, Ref, Ptr> &)
    {
        return 0;
    }

    template <class T, class Alloc = alloc>
    class list
    {
    public:
        typedef listNode<T> list_node;
        typedef list_node *link_type;
        link_type node; // node始终指向这个环形链表的尾节点
        typedef T value_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef listIterator<T, T &, T *> iterator;
        typedef listIterator<T, const T &, const T *> const_iterator;
        typedef simple_alloc<list_node, Alloc> dataAllocator;

    public:
        iterator begin()
        {
            return (link_type)(node->next);
        }
        iterator end()
        {
            return node;
        }
        bool empty() const
        {
            return node->next == node;
        }
        size_type size() 
        {
            return distance(begin(), end());
        }
        reference front()
        {
            return *begin();
        }
        reference back()
        {
            return *(--end());
        }

    public:
        //释放链表上除了node外的所有元素
        void clear();

    protected:
        //配置一个节点
        link_type getNode()
        {
            return dataAllocator::allocate();
        }
        //释放一个节点
        void putNode(link_type n)
        {
            dataAllocator::deallocate(n);
        }
        //配置并构造一个节点
        link_type createNode(const T &v)
        {
            link_type n = getNode();
            try
            {
                construct(&n->data, v);
            }
            catch (std::exception e)
            {
                putNode(n);
                throw;
            }
            return n;
        }
        //释放并析构一个节点
        void destroyNode(link_type n)
        {
            destroy(&n->data);
            putNode(n);
        }

    public:
        list()
        {
            node = getNode();
            node->next = node;
            node->prev = node;
        }
        ~list()
        {
            clear();
            putNode(node);
        }

    public:
        iterator insert(iterator position, const T &v);
        iterator insert(iterator position);
        void insert(iterator positon, size_type n, const T &v);
        void insert(iterator position, const T *first, const T *end);
        void insert(iterator position, const_iterator first, const_iterator);
        void push_front(const T &v);
        void push_back(const T &v);
        iterator erase(iterator position);
        iterator erase(iterator first, iterator end);
        void pop_front();
        void pop_back();
        void remove(const T &v); //将值为v的所有节点都移除
        void unique();           //将连续的数值相同的元素移除直至剩一个
        void resize(size_type newSize, const T &v);
        void transfer(iterator position, iterator first, iterator end);
    };

    template <class T, class Alloc>
    void list<T, Alloc>::clear()
    {
        link_type n = (link_type)node->next;
        while (n != node)
        {
            link_type temp = n;
            n = (link_type)(n->next);
            destroyNode(temp);
        }
        node->next = node;
        node->prev = node;
    }
    //在position之前插入一个新节点
    template <class T, class Alloc>
    typename list<T, Alloc>::iterator list<T, Alloc>::insert(iterator position, const T &v)
    {
        link_type n = createNode(v);
        n->next = position.node;
        n->prev = position.node->prev;
        link_type(position.node->prev)->next = n;
        position.node->prev = n;
        return n;
    }

    template <class T, class Alloc>
    typename list<T, Alloc>::iterator list<T, Alloc>::insert(iterator position)
    {
        return insert(position, T());
    }

    template <class T, class Alloc>
    void list<T, Alloc>::insert(iterator position, size_type n, const T &v)
    {
        for (; n > 0; --n)
        {
            insert(position, v);
        }
    }

    template <class T, class Alloc>
    void list<T, Alloc>::insert(iterator position, const T *first, const T *end)
    {
        for (; first != end; ++first)
        {
            insert(position, *first);
        }
    }

    template <class T, class Alloc>
    void list<T, Alloc>::insert(iterator position, const_iterator first, const_iterator end)
    {
        for (; first != end; ++first)
        {
            insert(position, *first);
        }
    }

    template <class T, class Alloc>
    void list<T, Alloc>::push_front(const T &v)
    {
        insert(begin(), v);
    }

    template <class T, class Alloc>
    void list<T, Alloc>::push_back(const T &v)
    {
        insert(end(), v);
    }

    template <class T, class Alloc>
    typename list<T, Alloc>::iterator list<T, Alloc>::erase(iterator position)
    {
        link_type next = position.node->next;
        link_type prev = position.node->prev;
        prev->next = next;
        next->prev = prev;
        destroyNode(position.node);
        return iterator(next);
    }

    template <class T, class Alloc>
    void list<T, Alloc>::pop_front()
    {
        erase(begin());
    }

    template <class T, class Alloc>
    void list<T, Alloc>::pop_back()
    {
        link_type n = end();
        erase(--n);
    }

    template <class T, class Alloc>
    typename list<T, Alloc>::iterator list<T, Alloc>::erase(iterator first, iterator end)
    {
        while (first != end)
        {
            erase(first++);
        }
        return end;
    }

    template <class T, class Alloc>
    void list<T, Alloc>::remove(const T &v)
    {
        iterator first = begin();
        iterator end = end();
        while (first != end)
        {
            iterator next = first;
            ++next;
            if (*first == v)
                erase(first);
            first = next;
        }
    }
    template <class T, class Alloc>
    void list<T, Alloc>::unique()
    {
        iterator first = begin();
        iterator end = end();
        if (first == end)
            return;
        iterator next = first;
        while (++next != end)
        {
            if (*first == *next)
                erase(next);
            else
                first = next;
            next = first;
        }
    }

    template <class T, class Alloc>
    void list<T, Alloc>::resize(size_type newSize, const T &v)
    {
        iterator i = begin();
        size_type len = 0;
        for(; i!=end() && len<newSize; ++i, ++len);
        if(len==newSize)        //new size较小
        {
            erase(i, end());
        }
        else    //new size较大
        {
            insert(end(), newSize-len, v);
        }
    }
    //将[first, end)中的元素移到position之前
    template <class T, class Alloc>
    void list<T, Alloc>::transfer(iterator position, iterator first, iterator end)
    {
        if(position!=end)
        {
            end.node->prev->next = position.node;
            first.node->prev->next = end.node;
            position.node->prev->next = first.node;
            link_type temp = position.node->prev;
            position.node->prev = end.node->prev;
            end.node->prev = first.node->prev;
            first.node->prev = temp;
        }
    }

}
#endif