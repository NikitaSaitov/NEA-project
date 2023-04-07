#ifndef MOVE_ENCODING_H
#define MOVE_ENCODING_H

#include <iostream>
#include "move_encoding.h"
#include "const.h"

inline int createMove(int startSquareIndex, int targetSquareIndex, int piece, int promotedPiece, bool fCapture, bool fDoublePawnPush, bool fEnPassant, bool fCastling)
{
    return startSquareIndex | (targetSquareIndex << 6) | (piece << 12) | (promotedPiece << 16) | (fCapture << 20) | (fDoublePawnPush << 21) | (fEnPassant << 22) | (fCastling << 23);
}

inline int getStartSquareIndex(int move)
{
    return move & 0x3f;
}

inline int getTargetSquareIndex(int move)
{
    return (move & 0xfc0) >> 6;
}

inline int getPiece(int move)
{
    return (move & 0xf000) >> 12;
}

inline int getPromotedPiece(int move)
{
    return (move & 0xf0000) >> 16;
}

inline bool isCapture(int move)
{
    return move & 0x100000;
}

inline bool isDoublePawnPush(int move)
{
    return move & 0x200000;
}

inline bool isEnPassant(int move)
{
    return move & 0x400000;
}

inline bool isCastling(int move)
{
    return move & 0x800000;
}

inline void printMove(int move)
{
    if(getPromotedPiece(move)){
       std::cout << SQUARE_INDEX_TO_COORDINATES[getStartSquareIndex(move)] << SQUARE_INDEX_TO_COORDINATES[getTargetSquareIndex(move)] << PIECE_INDEX_TO_ASCII[getPromotedPiece(move)];
    }else{
       std::cout << SQUARE_INDEX_TO_COORDINATES[getStartSquareIndex(move)] << SQUARE_INDEX_TO_COORDINATES[getTargetSquareIndex(move)];
    }
}

#endif

