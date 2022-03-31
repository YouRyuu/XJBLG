//参考《STL源码剖析》一书
// type_traits
//类型萃取

#ifndef TYPETRAITS_H
#define TYPETRAITS_H

namespace lstl
{

    struct _true_type
    {
    };

    struct _false_type
    {
    };

    template <class T>
    class type_traits
    {
    public:
        typedef _false_type has_trivial_default_constructor; //默认的构造函数
        typedef _false_type has_trivial_copy_constructor;    //默认的拷贝构造函数
        typedef _false_type has_trivial_assignment_operator; //默认的拷贝赋值构造函数
        typedef _false_type has_trivial_destructor;          //默认的析构函数
        typedef _false_type is_POD_type;                     // pod类型
    };

    template <>
    struct type_traits<bool>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };

    template <>
    struct type_traits<char>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };

    template <>
    struct type_traits<signed char>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };

    template <>
    struct type_traits<unsigned char>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };

    template <>
    struct type_traits<short>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };
    template <>
    struct type_traits<unsigned short>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };

    template <>
    struct type_traits<int>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };

    template <>
    struct type_traits<unsigned int>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };

    template <>
    struct type_traits<long>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };

    template <>
    struct type_traits<unsigned long>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };

    template <class T>
    struct type_traits<T *>
    {
        typedef _true_type has_trivial_default_constructor;
        typedef _true_type has_trivial_copy_constructor;
        typedef _true_type has_trivial_assignment_operator;
        typedef _true_type has_trivial_destructor;
        typedef _true_type is_POD_type;
    };
}
#endif