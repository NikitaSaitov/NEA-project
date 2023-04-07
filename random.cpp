#include <iostream>
#include <cstdlib>
#include "random.h"

void seedRandom()
{
    srand(time(NULL));
}

U64 getRandom()
{
    U64 r1, r2, r3, r4;

    r1 = (U64)(rand()) & 0xFFFF;
    r2 = (U64)(rand()) & 0xFFFF;
    r3 = (U64)(rand()) & 0xFFFF;
    r4 = (U64)(rand()) & 0xFFFF;    

    return r1 | (r2 << 16) | (r3 << 32) | (r4 << 48);
}

U64 getRandomFewBits()
{
    return getRandom() & getRandom() & getRandom();
}
