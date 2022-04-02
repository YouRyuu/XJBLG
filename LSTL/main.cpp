#include <iostream>
#include "vector.h"
#include "list.h"
class A
{
    public:
        A(int v):value(v){}
        void print()
        {
            std::cout<<this<<" value:"<<value<<std::endl;
        }

        ~A()
        {
            std::cout<<this<<" is ~"<<std::endl;
        }
    private:
        int value;
};


void testVector()
{
    lstl::vector<A> dp;
    std::cout << dp.size() << std::endl;
    for (int i = 1; i <= 10; ++i)
       dp.push_back(A(i));
    std::cout << dp.capacity() << std::endl;
    for (int i = 0; i < 10; ++i)
    {
       dp[i].print();
    }
    std::cout<<"---------------"<<std::endl;
    for (lstl::vector<A>::iterator it = dp.begin(); it != dp.end(); ++it)
    {
       it->print();
    }
}

void testList()
{
    lstl::list<A> lt;
    std::cout<<lt.size()<<std::endl;
    for(int i=1;i<10;++i)
        lt.push_back(A(i));
    for(int i=11;i<20;++i)
        lt.push_front(A(i));
    for(lstl::list<A>::iterator it = lt.begin(); it!=lt.end(); ++it)
        it->print();
}


int main()
{
    testList();
    return 0;
}