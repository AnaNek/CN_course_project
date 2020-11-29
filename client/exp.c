#include <stdlib.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
    printf("%d\n", atoi(" 405 6 some text\r\n"));
    return 0;
}
