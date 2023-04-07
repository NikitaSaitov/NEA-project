#include "engine_exceptions.h"

const char* LSBOfEmptyBitboardException::what()
{
    return "Cannot return the LSB of an empty bitboard";
}

const char* HashKeysNotInitializedException::what()
{
    return "Hash keys not initialized";
}

const char* CannotFindMagicNumberException::what()
{
    return "Could not find a magic number";
}

const char* MagicNumberNotInitializedException::what()
{
    return "Magic numbers not initialized";
}
