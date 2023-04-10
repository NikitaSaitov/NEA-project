#ifndef MAGICS_H
#define MAGICS_H

using U64 = unsigned long long;

U64 maskPawnAttacks(int squareIndex, int sideToMove);
U64 maskKnightAttacks(int squareIndex);
U64 maskBishopAttacks(int squareIndex);
U64 maskRookAttacks(int squareIndex);
U64 maskKingAttacks(int squareIndex);

U64 generateBishopAttacks(int squareIndex, const U64& occupancy);
U64 generateRookAttacks(int squareIndex, const U64& occupancy);
U64 generateOccupancy(int occupancyIndex, int relevantBits, U64 attackMask);

U64 findMagicNumber(int squareIndex, int relevantBits, bool fBishop);

#endif

