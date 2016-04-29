#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"


bool add(int a, int b)
{
    if (a < b)
        return false;
    else
        return true;
}

void CHECK_STAT(bool in)
{
    printf("FN Call: %d\n", in);
}

int tmain()
{
#define CHECK_STAT(x) if (!(x)) return false
    printf("\nTMain\n");

    CHECK_STAT(add(5,3));
    printf("Success\n");

    CHECK_STAT(add(5,
                3));
    printf("Success\n");

    CHECK_STAT(add(1,3));
    printf("Fail\n");
#undef CHECK_STAT
}

int emain()
{
    printf("\nEMain\n");

    CHECK_STAT(add(5,3));
    printf("Success\n");
    CHECK_STAT(add(1,3));
    printf("Fail\n");
}

int rmain()
{
#define CHECK_STAT(x) if (!(x)) return false
    printf("\nRMain\n");

    CHECK_STAT(add(5,3));
    printf("Success\n");
    CHECK_STAT(add(1,3));
    printf("Fail\n");
}

int wmain()
{
    printf("\nWMain\n");

    CHECK_STAT(add(5,3));
    printf("Success\n");
    CHECK_STAT(add(1,3));
    printf("Fail\n");
}

#undef CHECK_STAT

int main()
{
    printf("Tmain returns %d\n", tmain());
    printf("Emain returns %d\n", emain());
    printf("Rmain returns %d\n", rmain());
    printf("Wmain returns %d\n", wmain());

    printf("\nMain\n");
    CHECK_STAT(add(5,3));
}
