//参考《STL源码剖析》一书
// alloc
//负责内存的分配和回收

#include <new>
#include <stdlib.h>
#include <string.h>

namespace lstl
{
    //直接使用malloc和free函数管理内存
    template <int inst>
    class malloc_alloc_template
    {
    private:
        static void *oom_malloc(size_t n); //处理内存不足时候的情况
        static void *oom_realloc(void *, size_t n);
        static void (*malloc_error_handler)(); //内存分配出错的时候的处理函数
    public:
        static void *allocate(size_t n)
        {
            void *ret = malloc(n);
            if (ret == 0)
                ret = oom_malloc(n);
            return ret;
        }

        static void deallocate(void *p, size_t)
        {
            free(p);
        }

        static void *reallocate(void *p, size_t, size_t n)
        {
            void *ret = realloc(p, n);
            if (ret == 0)
                ret = oom_realloc(p, n);
            return ret;
        }
    };

    template <int inst>
    void (*malloc_alloc_template<inst>::malloc_error_handler)() = 0;

    template <int inst>
    void *malloc_alloc_template<inst>::oom_malloc(size_t n)
    {
        void *ret;
        for (;;)
        {
            if (malloc_error_handler == 0)
                throw std::bad_alloc();
            ret = malloc(n);
            if (ret)
                return ret;
        }
    }

    template <int inst>
    void *malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
    {
        void *ret;
        for (;;)
        {
            if (malloc_error_handler == 0)
                throw std::bad_alloc();
            ret = realloc(p, n);
            if (ret)
                return ret;
        }
    }

    typedef malloc_alloc_template<0> malloc_alloc;

    template <class P, class Alloc>
    class simple_alloc
    {
    public:
        static P *allocate(size_t n)
        {
            return 0 == n ? 0 : Alloc::allocate(n * sizeof(P));
        }

        static P *allocate(void)
        {
            return Alloc::allocate(sizeof(P));
        }

        static void deallocate(P *p, size_t n)
        {
            if (n != 0)
            {
                Alloc::deallocate(p, n);
            }
        }

        static void deallocate(P *p)
        {
            Alloc::deallocate(p, sizeof(P));
        }
    };

    enum
    {
        ALIGN = 8
    }; //每一个区块的内存上调大小
    enum
    {
        MAX_BYTE = 128
    }; //所能维护的最大的一个小区块大小
    enum
    {
        NFREELISTS = MAX_BYTE / ALIGN
    }; //链表个数
    //使用内存池实现的内存分配器
    template <int inst>
    class default_alloc_template
    {
    private:
        static size_t round_up(size_t bytes) //将bytes上调到8的倍数
        {
            return (((bytes) + ALIGN - 1) & ~(ALIGN - 1));
        }

        union obj
        {
            union obj *link;
            char client[1];
        };

        static obj *free_list[NFREELISTS]; //类似于散列表实现

        static size_t freelist_index(size_t bytes)
        {
            return (((bytes) + ALIGN - 1) / ALIGN - 1); //根据byte寻找对应的节点
        }

        static void *refill(size_t n);                     //将一个大小为n的区块加入到内存池中去
        static char *chunk_alloc(size_t size, int &nobjs); //申请nobjs个size大小的区块，如果不能满足nobjs个，会减少nobjs

        static char *start_free; //内存池起始位置，由chunk_alloc设置
        static char *end_free;   //内存池结束位置， 由chunk_alloc设置
        static size_t heap_size; //

    public:
        static void *allocate(size_t n)
        {
            void *ret = 0;
            if (n > (size_t)MAX_BYTE) //如果要申请的内存大于次级分配器所支持的最大内存，就使用第一级分配器
                ret = malloc_alloc::allocate(n);
            else
            {
                obj **freeList = free_list + freelist_index(n); //找到容纳n的区块的位置
                obj *_ret = *freeList;
                if (_ret == 0)                 //如果此时没有了剩余空闲内存块
                    ret = refill(round_up(n)); //新开辟
                else
                {
                    *freeList = _ret->link; //取走，然后把下一块内存放在前面
                    ret = _ret;
                }
            }
            return ret;
        }

        static void deallocate(void *p, size_t n)
        {
            if (n > (size_t)(MAX_BYTE))
                malloc_alloc::deallocate(p, n);
            else
            {
                //找到这个要释放的内存块在freelist中的位置然后归还之
                obj **freeList = free_list + freelist_index(n);
                obj *q = (obj *)p;
                q->link = *freeList;
                *freeList = q;
            }
        }

        static void *reallocate(void *p, size_t old_size, size_t new_size);
    };

    template <int inst>
    char *default_alloc_template<inst>::chunk_alloc(size_t size, int &nobjs)
    {
        char *ret;
        size_t total_bytes = size * nobjs;
        size_t left_bytes = end_free - start_free;
        if (left_bytes >= total_bytes)
        {
            ret = start_free;
            start_free += total_bytes;
            return ret;
        }
        else if (left_bytes >= size) //不够所有的nobjs，但是能满足一部分
        {
            nobjs = (int)(left_bytes / size);
            total_bytes = nobjs * size;
            ret = start_free;
            start_free += total_bytes;
            return ret;
        }
        else //一个也满足不了
        {
            size_t bytesToGet = total_bytes * 2 + round_up(heap_size >> 4);
            if (left_bytes > 0) //内存池中还有一些小空间
            {
                obj **freeList = freeList + freelist_index(left_bytes);
                ((obj *)start_free)->link = *freeList;
                *freeList = (obj *)start_free;
            }
            //补充内存池
            start_free = (char *)malloc(bytesToGet);
            if (start_free == 0) // malloc失败
            {
                size_t i;
                obj **freeList, *p;
                //在freelist中寻找有没有合适的空间
                for (i = size; i < (size_t)MAX_BYTE; i += (size_t)ALIGN)
                {
                    freeList = free_list + freelist_index(i);
                    p = *freeList;
                    if (p != 0)
                    {
                        *freeList = p->link;
                        start_free = (char *)p;
                        end_free = start_free + i;
                        return chunk_alloc(size, nobjs);
                    }
                }
                end_free = 0;
                start_free = (char *)malloc_alloc::allocate(bytesToGet);
            }
            heap_size += bytesToGet;
            end_free = start_free + bytesToGet;
            return chunk_alloc(size, nobjs);
        }
    }

    template <int inst>
    void *default_alloc_template<inst>::refill(size_t n)
    {
        int nobjs = 20;
        char *chunk = chunk_alloc(n, nobjs);
        obj **freeList;
        obj *ret, *current, *next;
        int i;
        if (nobjs == 1)
            return chunk;
        freeList = free_list + freelist_index(n);
        ret = (obj *)chunk;
        *freeList = next = (obj *)(chunk + n);
        for (i = 1;; ++i)
        {
            current = next;
            next = (obj *)((char *)next + n);
            if (nobjs - 1 == i)
            {
                current->link = 0;
                break;
            }
            else
            {
                current->link = next;
            }
        }
        return ret;
    }

    template <int inst>
    void *default_alloc_template<inst>::reallocate(void *p, size_t old_size, size_t new_size)
    {
        void *ret;
        size_t copy_size;
        if (old_size > (size_t)MAX_BYTE && new_size > (size_t)MAX_BYTE)
            return (realloc(p, new_size));
        if (round_up(old_size) == round_up(new_size))
            return p;
        ret = allocate(new_size);
        copy_size = new_size > old_size ? old_size : new_size;
        memcpy(ret, p, copy_size);
        deallocate(p, old_size);
        return ret;
    }

    template <int inst>
    char *default_alloc_template<inst>::start_free = 0;
    template <int inst>
    char *default_alloc_template<inst>::end_free = 0;
    template <int inst>
    size_t default_alloc_template<inst>::heap_size = 0;
    template <int inst>
    typename default_alloc_template<inst>::obj *default_alloc_template<inst>::free_list[NFREELISTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    typedef default_alloc_template<0> alloc;

    template <class T>
    class alloctor
    {
        typedef alloc Alloc;

    public:
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef T *pointer;
        typedef const T *const_pointer;
        typedef T &reference;
        typedef const T &const_reference;
        typedef T value_type;

        template <class T2>
        struct rebind
        {
            typedef alloctor<T2> other;
        };

        alloctor() {}
        pointer address(reference x)
        {
            return &x;
        }

        const_pointer const_address(const_reference x)
        {
            return &x;
        }

        T *allocate(size_type n, const void *hint = 0)
        {
            return n != 0 ? static_cast<T *>(Alloc::allocate(n * sizeof(T))) : 0;
        }

        void deallocate(pointer p, size_type n)
        {
        Alloc:
            deallocate(p, n * sizeof(T));
        }

        size_type maxSize() const
        {
            //返回可配置T的最大容量
            // size_t是无符号整数，所以size_t(-1)返回UINT_MAX
            return size_t(-1) / sizeof(T);
        }

        void construct(pointer p, const T &value)
        {
            new (p) T(value);
        }

        void destroy(pointer p)
        {
            p->~T();
        }
    };

} // namespace 
