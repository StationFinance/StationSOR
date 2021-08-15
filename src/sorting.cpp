#include "sorting.h"


void pairsort(int price[], int liquidity[], int n)
{
    std::pair<int, int> p_l[n];
  
    // Store the elements of the array in pairs.
    for (int i = 0; i < n; i++)
    {
        p_l[i].first = price[i];
        p_l[i].second = liquidity[i];
    }
  
    // Sort the pair array
    std::sort(p_l, p_l + n);
      
    // Adjust the original arrays
    for (int i = 0; i < n; i++) 
    {
        price[i] = p_l[i].first;
        liquidity[i] = p_l[i].second;
    }
}