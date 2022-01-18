#include <iostream>
#include "Eventloop.h"
#include <unistd.h>

EventLoop* gloop;

void print()
{
    std::cout<<"print func"<<std::endl;
}

int main()
{
    EventLoop loop;
    gloop = &loop;
    
    return 0;
}