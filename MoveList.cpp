#include "MoveList.h"
#include "move_encoding.h"

void MoveList::appendMove(int startSquareIndex, int targetSquareIndex, int piece, int promotedPiece, bool fCapture, bool fDoublePawnPush, bool fEnPassant, bool fCastling)
{
    moves[count] = createMove(startSquareIndex, targetSquareIndex, piece, promotedPiece, fCapture, fDoublePawnPush, fEnPassant, fCastling);
    count++;
}

int* MoveList::getMoves()
{
    return moves;
}

const int MoveList::getCount()
{
    return count;
}        
