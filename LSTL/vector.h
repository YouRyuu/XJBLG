#ifndef VECTOR_H
#define VECTOR_H

#include "alloc.h"
#include "algorithm.h"
#include "construct.h"
#include "uninitalized.h"



namespace lstl
{

    template <class T, class Alloc = allocator<T> >
    class vector
    {
    public:
        typedef T value_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef value_type *iterator;
        typedef const value_type *const_iterator;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef Alloc dataAllocator;

    private:
        T *start;        //目前使用空间开始
        T *finish;       //目前使用空间的尾部
        T *endOfStorage; //目前可用空间尾部

    public:
        iterator begin()
        {
            return start;
        }
        const_iterator begin() const
        {
            return start;
        }
        iterator end()
        {
            return finish;
        }
        const_iterator end() const
        {
            return finish;
        }
        size_type size() const
        {
            return size_type(end() - begin());
        }
        size_type capacity() const
        {
            return size_type(endOfStorage - begin());
        }
        bool empty() const
        {
            return begin() == end();
        }
        reference operator[](size_type n)
        {
            return *(begin() + n);
        }
        const_reference operator[](size_type n) const
        {
            return *(begin() + n);
        }
        reference front()
        {
            return *begin();
        }
        const_reference front() const
        {
            return *begin();
        }
        reference back()
        {
            return *(end() - 1);
        }
        const_reference back() const
        {
            return *(end() - 1);
        }

        iterator allocateAndFill(size_type n, const T &value)
        {
            iterator ret = dataAllocator::allocate(n);
            uninitialized_fill_n(ret, n, value);
            return ret;
        }

        void fillInitialize(size_type n, const T &value)
        {
            start = allocateAndFill(n, value);
            finish = start + n;
            endOfStorage = finish;
        }

        pointer allocate(size_type n)
        {
            return dataAllocator::allocate(n);
        }

        void deallocate()
        {
            if (start)
            {
                dataAllocator::deallocate(start, endOfStorage - start);
            }
        }

    public: //构造析构函数
        vector() : start(0), finish(0), endOfStorage(0) {}
        vector(size_type n) : start(0), finish(0), endOfStorage(0)
        {
            start = dataAllocator::allocate(n);
            finish = uninitialized_fill_n(start, n, T());
            endOfStorage = start + n;
        }
        vector(size_type n, const T &value) : start(0), finish(0), endOfStorage(0)
        {
            start = dataAllocator::allocate(n);
            finish = uninitialized_fill_n(start, n, value);
            endOfStorage = start + n;
        }
        vector(const vector<T, Alloc> &otr) : start(0), finish(0), endOfStorage(0)
        {
            start = dataAllocator::allocate(otr.size());
            finish = uninitialized_copy(otr.begin(), otr.end(), start);
            endOfStorage = finish;
        }
        vector(const T *first, const T *last)
        {
            start = dataAllocator::allocate(last - first);
            finish = uninitialized_copy(first, last, start);
            endOfStorage = start + last - first;
        }
        ~vector()
        {
            destroy(start, finish);
            dataAllocator::deallocate(start, capacity());
        }

    public: //容器操作函数
        void insert_aux(iterator position);

        void insert_aux(iterator position, const T &value);

        //从position开始，插入n个元素
        void insert(iterator position, size_type n, const T &value);

        void push_back(const T &value);

        void pop_back();

        iterator erase(iterator position);

        iterator erase(iterator first, iterator last);

        void clear();

        void resize(size_type newSize, const T &value);

        void resize(size_type newSize);
    };

    template <class T, class Alloc>
    void vector<T, Alloc>::insert_aux(iterator position)
    {
        if (finish != endOfStorage) //还有备用空间
        {
            construct(finish, *(finish - 1));
            ++finish;
            copy_backward(position, finish - 2, finish - 1); //依次往后移动1个元素
            *position = T();
        }
        else //需要扩容
        {
            const size_type oldSize = size();
            const size_type newSize = oldSize == 0 ? 1 : 2 * oldSize; //按照原空间的2倍扩容
            iterator newStart = dataAllocator::allocate(newSize);
            iterator newFinish = newStart;
            try
            {
                newFinish = uninitialized_copy(start, position, newStart);
                construct(newFinish);
                ++newFinish;
                newFinish = uninitialized_copy(position, finish, newFinish);
            }
            catch (std::exception e)
            {
                destroy(newStart, newFinish);
                dataAllocator::deallocate(newStart, newSize);
                throw;
            }
            destroy(begin(), end());
            deallocate();
            start = newStart;
            finish = newFinish;
            endOfStorage = start + newSize;
        }
    }

    template <class T, class Alloc>
    void vector<T, Alloc>::insert_aux(iterator position, const T &value)
    {
        if (finish != endOfStorage) //还有备用空间
        {
            construct(finish, *(finish - 1));
            ++finish;
            T v = value;
            std::copy_backward(position, finish - 2, finish - 1); //依次往后移动1个元素
            *position = v;
        }
        else //需要扩容
        {
            const size_type oldSize = size();
            const size_type newSize = oldSize == 0 ? 1 : 2 * oldSize; //按照原空间的2倍扩容
            iterator newStart = dataAllocator::allocate(newSize);
            iterator newFinish = newStart;
            try
            {
                newFinish = uninitialized_copy(start, position, newStart);
                construct(newFinish, value);
                ++newFinish;
                newFinish = uninitialized_copy(position, finish, newFinish);
            }
            catch (std::exception e)
            {
                destroy(newStart, newFinish);
                dataAllocator::deallocate(newStart, newSize);
                throw;
            }
            destroy(begin(), end());
            deallocate();
            start = newStart;
            finish = newFinish;
            endOfStorage = start + newSize;
        }
    }

    //从position开始，插入n个元素
    template <class T, class Alloc>
    void vector<T, Alloc>::insert(iterator position, size_type n, const T &value)
    {
        if (n != 0)
        {
            if (size_type(endOfStorage - finish) >= n) //空闲空间够插入n个元素
            {
                T v = value;
                size_type elems = finish - position; //插入点后的元素数量
                iterator oldFinish = finish;
                //这里为什么需要分这两种情况呢，因为这插入点后的元素肯定是要移动的，而后面的空间可能是没有初始化的
                if (elems > n) //如果要插入的数量小于插入点后的元素数量，分两次移动position到finish区间的值
                {
                    uninitialized_copy(finish - n, finish, finish);
                    finish += n;
                    copy_backward(position, oldFinish - n, oldFinish);
                    fill(position, position + n, v);
                }
                else
                {
                    uninitialized_fill_n(finish, n - elems, v);
                    finish += n - elems;
                    uninitialized_copy(position, oldFinish, finish);
                    finish += elems;
                    fill(position, oldFinish, v);
                }
            }
            else
            {
                const size_type oldSize = size();
                const size_type newSize = oldSize + (oldSize > n ? oldSize : n);
                iterator newStart = dataAllocator::allocate(newSize);
                iterator newFinish = newStart;
                try
                {
                    newFinish = uninitialized_copy(start, position, newStart);
                    newFinish = uninitialized_fill_n(newFinish, n, value);
                    newFinish = uninitialized_copy(position, finish, newFinish);
                }
                catch (std::exception e)
                {
                    destroy(newStart, newFinish);
                    dataAllocator::deallocate(newStart, newSize);
                    throw;
                }
                destroy(start, finish);
                deallocate();
                start = newStart;
                finish = newFinish;
                endOfStorage = start + newSize;
            }
        }
    }

    template <class T, class Alloc>
    void vector<T, Alloc>::push_back(const T &value)
    {
        if (finish != endOfStorage) //容器还没有满
        {
            construct(finish, value); //在finish处构造新增的值
            ++finish;
        }
        else //容器满了，需要扩容
        {
            insert_aux(end(), value);
        }
    }

    template <class T, class Alloc>
    void vector<T, Alloc>::pop_back() //取出尾端元素
    {
        --finish;
        destroy(finish);
    }

    template <class T, class Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(iterator position) //删除position上的元素
    {
        if (position + 1 != end())
        {
            copy(position + 1, finish, position);
        }
        --finish;
        destroy(finish);
        return position;
    }

    template <class T, class Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(iterator first, iterator last) //删除first到last上的元素
    {
        iterator position = copy(last, finish, first);
        destroy(position, finish);
        finish = finish - (last - first);
        return first;
    }

    template <class T, class Alloc>
    void vector<T, Alloc>::clear()
    {
        erase(begin(), end());
    }

    template <class T, class Alloc>
    void vector<T, Alloc>::resize(size_type newSize, const T &value)
    {
        if (newSize < size())
        {
            erase(begin() + newSize, end());
        }
        else //扩容
        {
            insert(end(), newSize - size(), value);
        }
    }

    template <class T, class Alloc>
    void vector<T, Alloc>::resize(size_type newSize)
    {
        resize(newSize, T());
    }
} // end of namespace
#endif