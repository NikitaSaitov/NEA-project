#ifndef BITBOARD_OPERATIONS_H
#define BITBOARD_OPERATIONS_H

#include <iostream>
#include "engine_exceptions.h"

using U64 = unsigned long long;

inline void setBit(U64& bitboard, int squareIndex)
{
    bitboard |= (1ULL << squareIndex);
}

inline void popBit(U64& bitboard, int squareIndex)
{
    bitboard & (1ULL << squareIndex) ? bitboard ^= (1ULL << squareIndex) : 0;
}

inline int getBit(U64 bitboard, int squareIndex)
{
    return (bitboard & (1ULL << squareIndex))? 1 : 0;
}

inline int getPopulationCount(U64 bitboard)
{
    int populationCount = 0;
    
    while(bitboard){
        populationCount++;
        bitboard &= bitboard - 1;
    }
    return populationCount;
}

inline int getLS1BIndex(U64 bitboard)
{
    if(bitboard){
        //Two's complement trick to isolate the LS1B, -1 to get the index 
        return getPopulationCount((bitboard & -bitboard) - 1);
    //If an empty bitboard is passed
    }else{
        throw LSBOfEmptyBitboardException();
    }
}

inline void printBitboard(const U64& bitboard){

    std::cout << '\n' << "Visual representation: " << "\n\n";
    
    for(int rank = 0; rank < 8; rank++){

        for(int file = 0; file < 8; file++){

            //Least significant file (LSF) mapping
            int squareIndex = rank * 8 + file;

            //Print the ranks
            if(!file){
                std::cout << 8 - rank << "  ";
            }
            std::cout << (getBit(bitboard, squareIndex)? 1 : 0) << ' ';
        }
        std::cout << '\n';
    }
    //Print the files
    std::cout << '\n' << "   a b c d e f g h " << '\n';
    //Display the decimal equivalent of a bitboard
    std::cout << '\n' << "Decimal representation: " << bitboard << '\n';
}

#endif