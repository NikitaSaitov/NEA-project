#ifndef RANDOM_H
#define RANDOM_H

using U64 = unsigned long long;

void seedRandom();

U64 getRandom();
U64 getRandomFewBits();

#endif