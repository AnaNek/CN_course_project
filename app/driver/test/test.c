#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int value = 0;
    int i = 0;
    
    for (i = 0; i < 100; i++)
    {
        value++;
        sleep(1);
        printf("value = %d\n", value);
    }
    return 0;
}

