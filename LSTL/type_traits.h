//参考《STL源码剖析》一书
// type_traits
//类型萃取

namespace lstl
{

    struct true_type
    {
    };

    struct false_type
    {
    };

    template <class T>
    class type_traits
    {
    public:
        typedef false_type has_trivial_default_constructor; //默认的构造函数
        typedef false_type has_trivial_copy_constructor;    //默认的拷贝构造函数
        typedef false_type has_trivial_assignment_operator; //默认的拷贝赋值构造函数
        typedef false_type has_trivial_destructor;          //默认的析构函数
        typedef false_type is_POD_type;                     // pod类型
    };

    template <>
    struct type_traits<bool>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };

    template <>
    struct type_traits<char>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };

    template <>
    struct type_traits<signed char>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };

    template <>
    struct type_traits<unsigned char>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };

    template <>
    struct type_traits<short>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };
    template <>
    struct type_traits<unsigned short>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };

    template <>
    struct type_traits<int>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };

    template <>
    struct type_traits<unsigned int>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };

    template <>
    struct type_traits<long>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };

    template <>
    struct type_traits<unsigned long>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };

    template <class T>
    struct type_traits<T *>
    {
        typedef true_type has_trivial_default_constructor;
        typedef true_type has_trivial_copy_constructor;
        typedef true_type has_trivial_assignment_operator;
        typedef true_type has_trivial_destructor;
        typedef true_type is_POD_type;
    };
}