#include <cstring>
#include "magic_bitboards.h"
#include "bitboard_operations.h"
#include "engine_exceptions.h"
#include "random.h"
#include "const.h"

enum {white, black, both};

U64 maskPawnAttacks(int squareIndex, int sideToMove)
{
    U64 pawnsBitboard = 0ULL, attacksBitboard = 0ULL;
    setBit(pawnsBitboard, squareIndex);

    switch (sideToMove)
    {
    case white:

        if(pawnsBitboard & NOT_H_FILE){
            attacksBitboard |= (pawnsBitboard >> 7);
        }
        if(pawnsBitboard & NOT_A_FILE){
            attacksBitboard |= (pawnsBitboard >> 9);
        }
        break;

    case black:

        if(pawnsBitboard & NOT_H_FILE){
            attacksBitboard |= (pawnsBitboard << 9);
        }
        if(pawnsBitboard & NOT_A_FILE){
            attacksBitboard |= (pawnsBitboard << 7);
        }
        break;
    }
    return attacksBitboard;
}

U64 maskKnightAttacks(int squareIndex)
{
    U64 knightsBitboard = 0ULL, attacksBitboard = 0ULL;
    setBit(knightsBitboard, squareIndex);

    if(knightsBitboard & NOT_H_FILE){
            attacksBitboard |= (knightsBitboard >> 15);
    }
    if(knightsBitboard & NOT_A_FILE){
            attacksBitboard |= (knightsBitboard >> 17);
    }
    if(knightsBitboard & NOT_HG_FILE){
            attacksBitboard |= (knightsBitboard >> 6);
    }
    if(knightsBitboard & NOT_AB_FILE){
            attacksBitboard |= (knightsBitboard >> 10);
    }
    if(knightsBitboard & NOT_A_FILE){
            attacksBitboard |= (knightsBitboard << 15);
    }
    if(knightsBitboard & NOT_H_FILE){
            attacksBitboard |= (knightsBitboard << 17);
    }
    if(knightsBitboard & NOT_AB_FILE){
            attacksBitboard |= (knightsBitboard << 6);
    }
    if(knightsBitboard & NOT_HG_FILE){
            attacksBitboard |= (knightsBitboard << 10);
    }
    return attacksBitboard;
}

U64 maskBishopAttacks(int squareIndex)
{
    U64 attacksBitboard = 0ULL;

    int rank, file;

    int targetRank = squareIndex / 8;
    int targetFile = squareIndex % 8;

    for(rank = targetRank + 1, file  = targetFile + 1; rank < 7 && file < 7; rank++, file++){
        setBit(attacksBitboard, rank * 8 + file);
    }
    for(rank = targetRank - 1, file = targetFile + 1; rank > 0 && file < 7; rank--, file++){
        setBit(attacksBitboard, rank * 8 + file);
    }
    for(rank = targetRank + 1, file = targetFile - 1; rank < 7 && file > 0; rank++, file--){
        setBit(attacksBitboard, rank * 8 + file);
    }
    for(rank = targetRank - 1, file = targetFile - 1; rank > 0 && file > 0; rank--, file--){
        setBit(attacksBitboard, rank * 8 + file);
    }
    return attacksBitboard;
}

U64 maskRookAttacks(int squareIndex)
{
    U64 attacksBitboard = 0ULL;

    int rank, file;

    int targetRank = squareIndex / 8;
    int targetFile = squareIndex % 8;

    for(rank = targetRank + 1; rank < 7; rank++){
        setBit(attacksBitboard, rank * 8 + targetFile);
    }
    for(rank = targetRank - 1; rank > 0; rank--){
        setBit(attacksBitboard, rank * 8 + targetFile);
    }
    for(file = targetFile + 1; file < 7; file++){
        setBit(attacksBitboard, targetRank * 8 + file);
    }
    for(file = targetFile - 1; file > 0; file--){
        setBit(attacksBitboard, targetRank * 8 + file);
    }
    return attacksBitboard;
}

U64 maskKingAttacks(int squareIndex)
{
    U64 kingBitboard = 0ULL, attacksBitboard = 0ULL;
    setBit(kingBitboard, squareIndex);

    if(kingBitboard >> 8){
        attacksBitboard |= kingBitboard >> 8;
    }
    if(kingBitboard & NOT_H_FILE){
        attacksBitboard |= kingBitboard >> 7;
    }
    if(kingBitboard & NOT_A_FILE){
        attacksBitboard |= kingBitboard >> 9;
        attacksBitboard |= kingBitboard >> 1;
    }
    if(kingBitboard << 8){
        attacksBitboard |= kingBitboard << 8;
    }
    if(kingBitboard & NOT_A_FILE){
        attacksBitboard |= kingBitboard << 7;
    }
    if(kingBitboard & NOT_H_FILE){
        attacksBitboard |= kingBitboard << 9;
        attacksBitboard |= kingBitboard << 1;
    }
    return attacksBitboard;
}

U64 generateBishopAttacks(int squareIndex, const U64& occupancy)
{
    U64 attacksBitboard = 0ULL;
    
    int rank, file;

    int targetRank = squareIndex / 8;
    int targetFile = squareIndex % 8;

    for(rank = targetRank + 1, file  = targetFile + 1; rank < 8 && file < 8; rank++, file++){
        setBit(attacksBitboard, rank * 8 + file);
        if(getBit(occupancy, rank * 8 + file)){
            break;
        }
    }
    for(rank = targetRank - 1, file = targetFile + 1; rank >= 0 && file < 8; rank--, file++){
        setBit(attacksBitboard, rank * 8 + file);
        if(getBit(occupancy, rank * 8 + file)){
            break;
        }
    }
    for(rank = targetRank + 1, file = targetFile - 1; rank < 8 && file >= 0; rank++, file--){
        setBit(attacksBitboard, rank * 8 + file);
        if(getBit(occupancy, rank * 8 + file)){
            break;
        }
    }
    for(rank = targetRank - 1, file = targetFile - 1; rank >= 0 && file >= 0; rank--, file--){
        setBit(attacksBitboard, rank * 8 + file);
        if(getBit(occupancy, rank * 8 + file)){
            break;
        }
    }
    return attacksBitboard;
}

U64 generateRookAttacks(int squareIndex, const U64& occupancy)
{
    U64 attacksBitboard = 0ULL;

    int rank, file;

    int targetRank = squareIndex / 8;
    int targetFile = squareIndex % 8;

    for(rank = targetRank + 1; rank < 8; rank++){
        setBit(attacksBitboard, rank * 8 + targetFile);
        if(getBit(occupancy, rank * 8 + targetFile)){
            break;
        }
    }
    for(rank = targetRank - 1; rank >= 0; rank--){
        setBit(attacksBitboard, rank * 8 + targetFile);
        if(getBit(occupancy, rank * 8 + targetFile)){
            break;
        }
    }
    for(file = targetFile + 1; file < 8; file++){
        setBit(attacksBitboard, targetRank * 8 + file);
        if(getBit(occupancy, targetRank * 8 + file)){
            break;
        }
    }
    for(file = targetFile - 1; file >= 0; file--){
        setBit(attacksBitboard, targetRank * 8 + file);
        if(getBit(occupancy, targetRank * 8 + file)){
            break;
        }
    }
    return attacksBitboard;
}

U64 generateOccupancy(int occupancyIndex, int relevantBits, U64 attackMask)
{
    U64 bitboard = 0ULL;

    for (int currentBit = 0; currentBit < relevantBits; currentBit++){
        
        int squareIndex = getLS1BIndex(attackMask);
        popBit(attackMask, squareIndex);

        if (occupancyIndex & (1 << currentBit)){
            setBit(bitboard, squareIndex);
        }
    }
    return bitboard;
}

U64 findMagicNumber(int squareIndex, int relevantBits, bool fBishop)
{
    U64 occupancies[4096], attacks[4096], usedAttacks[4096];
    //Assign the attack mask
    U64 attackMask = fBishop ? maskBishopAttacks(squareIndex) : maskRookAttacks(squareIndex);
    //The position of the piece impacts the amount of squares it controls, which influences the number of possible occupancies
    int maxOccupancyIndex = 1 << relevantBits;

    for(int index = 0; index < maxOccupancyIndex; index++){
        occupancies[index] = generateOccupancy(index, relevantBits, attackMask);
        attacks[index] = fBishop ? generateBishopAttacks(squareIndex, occupancies[index]) : generateRookAttacks(squareIndex, occupancies[index]);
    }

    seedRandom();

    //Trial and error method needs a lot of repetitions, values from 10^7 to 10^9 should be used
    for(int i = 0; i < 100000000; i++){

        U64 magicNumber  = getRandomFewBits();
        //Skip the magic if the magic index is too large
        if(getPopulationCount((attackMask * magicNumber) & 0xFF00000000000000) < 6){
            continue;
        }

        memset(usedAttacks, 0, sizeof(usedAttacks));

        int index;
        bool fFail;

        for(index = 0, fFail = false; !fFail && index < maxOccupancyIndex; index++){

            //Generate the magic number and destroy the garbage bits
            int magicIndex = (int)((occupancies[index] * magicNumber) >> (64 - relevantBits));

            //Test the magic number
            if(usedAttacks[magicIndex] == 0ULL){
                usedAttacks[magicIndex] = attacks[index];
            }else if(usedAttacks[magicIndex] != attacks[index]){
                fFail = true;
            }
        }

        if(!fFail){
            return magicNumber;
        }
    }

    throw CannotFindMagicNumberException();
    return 0ULL;
}