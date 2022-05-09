//#include "mysqlDB.h"
#include "../function.h"
#include <unistd.h>
#include <iostream>
#include <vector>
int main()
{
    //srand((unsigned int)time(NULL));
    for(int i=0; i<10; ++i)
    {
        //std::cout<<time(NULL)<<std::endl;
    std::cout<<getSeq()<<std::endl;
    //sleep(1);
    }

    return 0;
}