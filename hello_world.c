#include <stdio.h>

int main()
{
    printf("Xin chao\n");
    int sum = 0;
    for (int i = 1; i <= 10; i++)
    {
        sum += i;
        printf("sum = %d\n", sum);
    }
        
    return 0;
}