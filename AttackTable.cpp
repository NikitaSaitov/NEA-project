#include "AttackTable.h"
#include "bitboard_operations.h"
#include "magic_bitboards.h"
#include "engine_exceptions.h"
#include "const.h"

enum {white, black, both};

void AttackTable::initializeMagicNumbers(){

    for(int squareIndex = 0; squareIndex < 64; squareIndex++){
        bishopMagics[squareIndex] = findMagicNumber(squareIndex, BISHOP_RELEVANT_BITS[squareIndex], true);
        rookMagics[squareIndex] = findMagicNumber(squareIndex, ROOK_RELEVANT_BITS[squareIndex], false);
    }     
}

void AttackTable::initializeLeapingPieceTables(){

    if(bishopMagics[0] == 0 || rookMagics[0] == 0){
        throw MagicNumberNotInitializedException();
    }

    for(int squareIndex = 0; squareIndex < 64; squareIndex++){

        pawnAttacks[white][squareIndex] = maskPawnAttacks(squareIndex, white);
        pawnAttacks[black][squareIndex] = maskPawnAttacks(squareIndex, black);
        knightAttacks[squareIndex] = maskKnightAttacks(squareIndex);
        kingAttacks[squareIndex] = maskKingAttacks(squareIndex);

    }
}

void AttackTable::initializeSlidingPieceTables(bool fBishop){

    for(int squareIndex = 0; squareIndex < 64; squareIndex++){

        bishopMasks[squareIndex] = maskBishopAttacks(squareIndex);
        rookMasks[squareIndex] = maskRookAttacks(squareIndex);

        U64 attackMask = fBishop ? bishopMasks[squareIndex] : rookMasks[squareIndex];

        int relevantBits = getPopulationCount(attackMask);
        int maxOccupancyIndex = 1 << relevantBits;

        for(int occupancyIndex = 0; occupancyIndex < maxOccupancyIndex; occupancyIndex++){

            U64 occupancy = generateOccupancy(occupancyIndex, relevantBits, attackMask);
                
            if(fBishop){

                int magicIndex = (occupancy * bishopMagics[squareIndex]) >> (64 - BISHOP_RELEVANT_BITS[squareIndex]);
                bishopAttacks[squareIndex][magicIndex] = generateBishopAttacks(squareIndex, occupancy);

            }
            else{

                int magicIndex = (occupancy * rookMagics[squareIndex]) >> (64 - ROOK_RELEVANT_BITS[squareIndex]);
                rookAttacks[squareIndex][magicIndex] = generateRookAttacks(squareIndex, occupancy);

            }
        }
    }
}

const U64 AttackTable::getPawnAttacks(int sideToMove, int squareIndex){
    return pawnAttacks[sideToMove][squareIndex];
}

const U64 AttackTable::getKnightAttacks(int squareIndex){
    return knightAttacks[squareIndex];
}

const U64 AttackTable::getKingAttacks(int squareIndex){
    return kingAttacks[squareIndex];
}

const U64 AttackTable::getBishopAttacks(int squareIndex, U64 occupancy){

    occupancy &= bishopMasks[squareIndex]; 
    occupancy *= bishopMagics[squareIndex];
    occupancy >>= 64 - BISHOP_RELEVANT_BITS[squareIndex];
    return bishopAttacks[squareIndex][occupancy];
}

const U64 AttackTable::getRookAttacks(int squareIndex, U64 occupancy){
    
    occupancy &= rookMasks[squareIndex]; 
    occupancy *= rookMagics[squareIndex];
    occupancy >>= 64 - ROOK_RELEVANT_BITS[squareIndex];
    return rookAttacks[squareIndex][occupancy];
}

const U64 AttackTable::getQueenAttacks(int squareIndex, U64 occupancy){
    return getBishopAttacks(squareIndex, occupancy) | getRookAttacks(squareIndex, occupancy);
}  