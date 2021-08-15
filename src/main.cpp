#include "sorting.h"
#include "orderfill.h"


int main()
{

    int prices[] = {3, 2, 5, 1};
    int liquidity[] = {100, 400, 500, 200};

    int n = sizeof(prices) / sizeof(prices[0]);

    pairsort(prices, liquidity, n);

     for (int i = 0; i < n; i++) 
        std::cout << prices[i] << " ";
     std::cout << std::endl;   

     for (int i = 0; i < n; i++)
         std::cout << liquidity[i] << " ";
    
    return 0;
}