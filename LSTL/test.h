#include <iostream>
template <int T>
class Test
{
    public:
    int dp[2];
    Test()
    {
        dp[0] = 10;
        dp[1] = 20;
    }

    void print()
    {
        for(int i=0; i<2; ++i)
        {
            std::cout<<dp[i]<<std::endl;
        }
    }
};

typedef Test<0> value;