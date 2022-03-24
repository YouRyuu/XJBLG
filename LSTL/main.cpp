#include <iostream>
using namespace std;

#include "construct.h"
#include "iterator.h"
//#include "type_traits.h"

void test(lstl::true_type)
{
    cout<<"ok"<<endl;
}

void test(lstl::false_type)
{
    cout<<"no ok"<<endl;
}

template <class T>
    void func(T *)
    {
        typedef typename lstl::type_traits<T>::is_POD_type isPod;
        test(isPod());
    }



int main()
{
    int a = 10;
    func(&a);

}