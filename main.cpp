//********Libraries, files and custom types********
#include <iostream>
#include <cstring>
#include <chrono>
#include <functional>
#include "enum.h"
#include "const.h"    
#include "bitboard_operations.h"
#include "random.h"
#include "magic_bitboards.h"
#include "engine_exceptions.h"
#include "AttackTable.h"
#include "move_encoding.h"
#include "MoveList.h"
#include "TranspositionTable.h"

using U64 = unsigned long long;
using std::string, std::cout, std::chrono::high_resolution_clock;

//********Global Class Instances********\\

AttackTable Attacks;
TranspositionTable Transpositions[MAX_HASH_SIZE];

//********Dynamically generated hash keys********\\

U64 PIECE_KEYS[12][64] = {0};
U64 ENPASSANT_KEYS[64] = {0};
U64 CASTLING_KEYS[16] = {0};
U64 SIDE_KEY = 0; 

void generateKeys(){

    for(int currentPiece = whitePawn; currentPiece <= blackKing; currentPiece++){

        for(int currentSquareIdex = 0; currentSquareIdex < 64; currentSquareIdex++){
            PIECE_KEYS[currentPiece][currentSquareIdex] = getRandom();
        }

    }

    for(int currentSquareIdex = 0; currentSquareIdex < 64; currentSquareIdex++){
        ENPASSANT_KEYS[currentSquareIdex] = getRandom();
    }

    for(int castlingIndex = 0; castlingIndex < 16; castlingIndex++){
        CASTLING_KEYS[castlingIndex] = getRandom();
    }

    SIDE_KEY = getRandom();
}

//********Board********\\

class Board{

    private:

        U64 bitboards[12];
        U64 occupancies[3];

        int sideToMove = NO_SIDE_TO_MOVE;
        int enPassantSquareIndex = NO_SQUARE_INDEX;
        int canCastle = 0;

        U64 hashKey;

        void resetBitboards(){
            memset(bitboards, 0, sizeof(bitboards)); 
        }

        void resetOcuupancies(){
            memset(occupancies, 0, sizeof(occupancies));
        }

        bool isSquareAttacked(int squareIndex, int sideToMove){ 

            if((sideToMove == white) && (Attacks.getPawnAttacks(black, squareIndex) & bitboards[whitePawn])){
                return true;
            }
            if((sideToMove == black) && (Attacks.getPawnAttacks(white, squareIndex) & bitboards[blackPawn])){
                return true;
            }
            if(Attacks.getKnightAttacks(squareIndex) & ((sideToMove == white) ? bitboards[whiteKnight] : bitboards[blackKnight])){
                return true;
            }
            if(Attacks.getBishopAttacks(squareIndex, occupancies[both]) & ((sideToMove == white) ? bitboards[whiteBishop] : bitboards[blackBishop])){
                return true;
            }
            if(Attacks.getRookAttacks(squareIndex, occupancies[both]) & ((sideToMove == white) ? bitboards[whiteRook] : bitboards[blackRook])){
                return true;
            }
            if(Attacks.getQueenAttacks(squareIndex, occupancies[both]) & ((sideToMove == white) ? bitboards[whiteQueen] : bitboards[blackQueen])){
                return true;
            }
            if(Attacks.getKingAttacks(squareIndex) & ((sideToMove == white) ? bitboards[whiteKing] : bitboards[blackKing])){
                return true;
            }   
            return false;
        }

        void generateHash(){

            if(PIECE_KEYS[0][0] == 0 || CASTLING_KEYS[0] == 0 || ENPASSANT_KEYS[0] == 0 || SIDE_KEY == 0){
                throw HashKeysNotInitializedException();
            }

            for(int currentPiece = whitePawn; currentPiece <= blackKing; currentPiece++){

               U64 currentBiboard = bitboards[currentPiece];

               while(currentBiboard){

                int squareIndex = getLS1BIndex(currentBiboard);
                hashKey ^= PIECE_KEYS[currentPiece][squareIndex];
                popBit(currentBiboard, squareIndex);

               }
            }

            if (enPassantSquareIndex != NO_SQUARE_INDEX){
                hashKey ^= ENPASSANT_KEYS[enPassantSquareIndex];
            }

            hashKey ^= CASTLING_KEYS[canCastle];

            if(sideToMove == black){
                hashKey ^= SIDE_KEY;
            }
        }

    public:

        Board(){}

        Board(string fenString){

            loadFenString(fenString);
            generateHash();

        }

        void populateOccupancies(){

            for(int currentPiece = whitePawn; currentPiece <= whiteKing; currentPiece++){
                occupancies[white] |= bitboards[currentPiece];
            }
            for(int currentPiece = blackPawn; currentPiece <= blackKing; currentPiece++){
                occupancies[black] |= bitboards[currentPiece];
            }
            occupancies[both] = occupancies[white] | occupancies[black];
        }

        void printState(){

            cout << '\n';

            for(int rank = 0; rank < 8; rank++){

                for(int file = 0; file < 8; file++){

                    //Least significant file (LSF) mapping
                    int squareIndex = rank * 8 + file;

                    //Print the ranks
                    if(!file){
                        cout << 8 - rank << "  ";
                    }

                    int piece = -1;

                    for(int currentPiece = whitePawn; currentPiece <= blackKing; currentPiece++){

                        if(getBit(bitboards[currentPiece], squareIndex)){
                            piece = currentPiece;
                        }
                    }
                    cout << ((piece == -1) ? '.' : PIECE_INDEX_TO_ASCII[piece]) << ' ';
                }
                cout << '\n';
            }
            //Print the files
            cout << '\n' << "   a b c d e f g h " << "\n\n";

            //Special position details
            cout << "Turn: " << ((sideToMove != NO_SIDE_TO_MOVE) ? ((!sideToMove) ? "white" : "black") : "not specified") << '\n';
            cout << "Can castle: " << ((canCastle & K) ? 'K' : '-') << ((canCastle & Q) ? 'Q' : '-') << ((canCastle & k) ? 'k' : '-') << ((canCastle & q) ? 'q' : '-') << '\n';
            cout << "EnPassant square: " << ((enPassantSquareIndex != NO_SQUARE_INDEX) ? SQUARE_INDEX_TO_COORDINATES[enPassantSquareIndex] : "no square") << '\n';
            cout << "Hash: " << hashKey << "\n\n";
        };

        struct State {
            U64 currentPieceBitboard;
            U64 currentPieceAttacks;
            int startSquareIndex;
            int targetSquareIndex;
            MoveList output;
            int currentPiece;
        };

        void PickNameYourself1(
            State& s,
            int whitePiece,
            int blackPiece,
            std::function<U64(void)> getAttacks) {
            if((sideToMove == white) ? s.currentPiece == whitePiece : s.currentPiece == blackPiece) {
                while(s.currentPieceBitboard) {
                    s.startSquareIndex = getLS1BIndex(s.currentPieceBitboard);
                    s.currentPieceAttacks = getAttacks() & ((sideToMove == white) ? ~occupancies[white] : ~occupancies[black]);

                    while(s.currentPieceAttacks) {
                        s.targetSquareIndex = getLS1BIndex(s.currentPieceAttacks);
                        // Quiet moves
                        if(!getBit(((sideToMove == white) ? occupancies[black] : occupancies[white]), s.targetSquareIndex)) {
                            s.output.appendMove(s.startSquareIndex, s.targetSquareIndex, s.currentPiece, 0, 0, 0, 0, 0);
                        } else {
                            //Captures
                            s.output.appendMove(s.startSquareIndex, s.targetSquareIndex, s.currentPiece, 0, 1, 0, 0, 0);
                        }
                        popBit(s.currentPieceAttacks, s.targetSquareIndex);
                    }
                    popBit(s.currentPieceBitboard, s.startSquareIndex);
                }
            }
        }

        void SmthAboutPawns(State& s, int color, std::function<bool()> quiteTest) {
            int indexShift = color == white ? -8 : +8;
            int otherColor = color == white ? black : white;
            int thisKnight = color == white ? whiteKnight : blackKnight;
            int thisQueen = color == white ? whiteQueen : blackQueen;
            int lBound = color == white ? a7 : a2;
            int rBound = color == white ? h7 : h2;
            int otherLBound = color == white ? a2 : a7;
            int otherRBound = color == white ? h2 : h7;


            while(s.currentPieceBitboard) {
                
                s.startSquareIndex = getLS1BIndex(s.currentPieceBitboard);
                s.targetSquareIndex = s.startSquareIndex + indexShift;

                //Quiet moves
                if(!quiteTest() && !getBit(occupancies[both], s.targetSquareIndex)){
                    
                    if(s.startSquareIndex >= lBound && s.startSquareIndex <= rBound){

                        for(int promotedPiece = thisKnight; promotedPiece <= thisQueen; promotedPiece++){
                            s.output.appendMove(s.startSquareIndex, s.targetSquareIndex, s.currentPiece, promotedPiece, 0, 0, 0, 0);
                        }
                        
                    }else{
                        s.output.appendMove(s.startSquareIndex, s.targetSquareIndex, s.currentPiece, 0, 0, 0, 0, 0);
                        if((s.startSquareIndex >= otherLBound && s.startSquareIndex <= otherRBound) && !getBit(occupancies[both], s.targetSquareIndex + indexShift)){
                            s.output.appendMove(s.startSquareIndex, s.targetSquareIndex + indexShift, s.currentPiece, 0, 0, 1, 0, 0);
                        }
                    }
                }

                s.currentPieceAttacks = Attacks.getPawnAttacks(color, s.startSquareIndex) & occupancies[otherColor];

                //Captures
                while (s.currentPieceAttacks) {
                    
                    s.targetSquareIndex = getLS1BIndex(s.currentPieceAttacks);

                    if(s.startSquareIndex >= lBound&& s.startSquareIndex <= rBound) {

                        for(int promotedPiece = thisKnight; promotedPiece <= thisQueen; promotedPiece++) {
                            s.output.appendMove(s.startSquareIndex, s.targetSquareIndex, s.currentPiece, promotedPiece, 1, 0, 0, 0);
                        }

                    } else {
                        s.output.appendMove(s.startSquareIndex, s.targetSquareIndex, s.currentPiece, 0, 1, 0, 0, 0);
                    }
                    popBit(s.currentPieceAttacks, s.targetSquareIndex);
                }

                //En passant
                if(enPassantSquareIndex != NO_SQUARE_INDEX){

                    U64 enPassantAttacks = Attacks.getPawnAttacks(color, s.startSquareIndex) & (1ULL << enPassantSquareIndex);
                    if(enPassantAttacks){
                        int enPassantTarget = getLS1BIndex(enPassantAttacks);
                        s.output.appendMove(s.startSquareIndex, enPassantTarget, s.currentPiece, 0, 1, 0, 1, 0);
                    }
                }
                popBit(s.currentPieceBitboard, s.startSquareIndex);
            }
        }

        MoveList generateMoves(){

            State s;

            for(s.currentPiece = whitePawn; s.currentPiece <= blackKing; s.currentPiece++){

                s.currentPieceBitboard = bitboards[s.currentPiece];

                //White pawn moves & castling
                if(sideToMove == white){

                    switch (s.currentPiece)
                    {
                    //Pawns
                    case whitePawn:

                        SmthAboutPawns(s, white, [&]() {
                            return s.targetSquareIndex < a8;
                        });
 
                        break;
                    
                    //Castling
                    case whiteKing:

                        if(canCastle & K){
                            if(!getBit(occupancies[both], f1) && !getBit(occupancies[both], g1) && !isSquareAttacked(e1, black) && !isSquareAttacked(f1, black)){
                                s.output.appendMove(e1, g1, s.currentPiece, 0, 0, 0, 0, 1);
                            }
                        }

                        if(canCastle & Q){
                           if(!getBit(occupancies[both], d1) && !getBit(occupancies[both], c1) && !getBit(occupancies[both], b1) 
                           && !isSquareAttacked(e1, black) && !isSquareAttacked(d1, black)){
                                s.output.appendMove(e1, c1, s.currentPiece, 0, 0, 0, 0, 1);
                            } 
                        }
                        break;
                    }

                //Black pawn moves & castling
                }else if(sideToMove == black){

                    switch (s.currentPiece)
                    {
                    //Pawns
                    case blackPawn:

                        SmthAboutPawns(s, black, [&]() {
                            return s.targetSquareIndex > h1;
                        });

                        break;
                    
                    //Castling
                    case blackKing:

                        if(canCastle & k){
                            if(!getBit(occupancies[both], f8) && !getBit(occupancies[both], g8) && !isSquareAttacked(e8, white) && !isSquareAttacked(f8, white)){
                                s.output.appendMove(e8, g8, s.currentPiece, 0, 0, 0, 0, 1);
                            }
                        }

                        if(canCastle & q){
                           if(!getBit(occupancies[both], d8) && !getBit(occupancies[both], c8) && !getBit(occupancies[both], b8) 
                           && !isSquareAttacked(e8, white) && !isSquareAttacked(d8, white)){
                                s.output.appendMove(e8, c8, s.currentPiece, 0, 0, 0, 0, 1);
                            } 
                        }
                        break;
                    }
                }

                //Knights
                PickNameYourself1(s, whiteKnight, blackKnight, 
                    [&]() {return Attacks.getKnightAttacks(s.startSquareIndex);});
                //Bishops
                PickNameYourself1(s, whiteBishop, blackBishop, 
                    [&]() {return Attacks.getBishopAttacks(s.startSquareIndex, occupancies[both]);});
                //Rooks
                PickNameYourself1(s, whiteRook, blackRook, 
                    [&]() {return Attacks.getRookAttacks(s.startSquareIndex, occupancies[both]);});             
                //Queens
                PickNameYourself1(s, whiteQueen, blackQueen, 
                    [&]() {return Attacks.getQueenAttacks(s.startSquareIndex, occupancies[both]);});           
                //King
                PickNameYourself1(s, whiteKing, blackKing, 
                    [&]() {return Attacks.getKingAttacks(s.startSquareIndex);});
            }
            return s.output;
        }

        bool isKingInCheck(){
            return isSquareAttacked((sideToMove == white) ? getLS1BIndex(bitboards[whiteKing]) : getLS1BIndex(bitboards[blackKing]), sideToMove ^ 1);
        }

        void switchSideToMove(){
            sideToMove ^= 1;
        }

        int makeMove(int move){

            U64 tempBitboards[12], tempOccupancies[3];
            int tempEnPassantSquareIndex = enPassantSquareIndex, tempCanCastle = canCastle;
            memcpy(tempBitboards, bitboards, sizeof(tempBitboards));
            memcpy(tempOccupancies, occupancies, sizeof(tempOccupancies));

            int piece = getPiece(move);
            int startSquareIndex = getStartSquareIndex(move);
            int targetSquareIndex = getTargetSquareIndex(move);
            int promotedPiece = getPromotedPiece(move);

            popBit(bitboards[piece], startSquareIndex);
            setBit(bitboards[piece], targetSquareIndex);

            hashKey ^= PIECE_KEYS[piece][startSquareIndex];
            hashKey ^= PIECE_KEYS[piece][targetSquareIndex];

            if(isCapture(move)){

                int startPiece, endPiece;

                if(sideToMove == white){

                    startPiece = blackPawn;
                    endPiece = blackKing;

                }else if(sideToMove == black){

                    startPiece = whitePawn;
                    endPiece = whiteKing;
                }

                for(int currentPiece = startPiece; currentPiece <= endPiece; currentPiece++){

                    if(getBit(bitboards[currentPiece], targetSquareIndex)){

                        popBit(bitboards[currentPiece], targetSquareIndex);
                        hashKey ^= PIECE_KEYS[currentPiece][targetSquareIndex];
                        break;

                    }
                }
            }

            if(promotedPiece){

                if(sideToMove == white){

                    popBit(bitboards[whitePawn], targetSquareIndex);
                    hashKey ^= PIECE_KEYS[whitePawn][targetSquareIndex];

                }else if(sideToMove == black){

                    popBit(bitboards[blackPawn], targetSquareIndex);
                    hashKey ^= PIECE_KEYS[blackPawn][targetSquareIndex];
                }

                setBit(bitboards[promotedPiece], targetSquareIndex);
                hashKey ^= PIECE_KEYS[promotedPiece][targetSquareIndex];
            }

            if(isEnPassant(move)){

                if(sideToMove == white){

                    popBit(bitboards[blackPawn], targetSquareIndex + 8);
                    hashKey ^= PIECE_KEYS[blackPawn][targetSquareIndex + 8];

                }else if(sideToMove == black){

                    popBit(bitboards[whitePawn], targetSquareIndex - 8);
                    hashKey ^= PIECE_KEYS[whitePawn][targetSquareIndex - 8];
                }
            }

            if(enPassantSquareIndex != NO_SQUARE_INDEX){
                hashKey ^= ENPASSANT_KEYS[enPassantSquareIndex];
            }

            enPassantSquareIndex = NO_SQUARE_INDEX;

            if(isDoublePawnPush(move)){

                if(sideToMove == white){
                    enPassantSquareIndex = targetSquareIndex + 8;
                }else if(sideToMove == black){
                    enPassantSquareIndex = targetSquareIndex - 8;
                }

                hashKey ^= ENPASSANT_KEYS[enPassantSquareIndex];
            }

            if(isCastling(move)){

                switch(targetSquareIndex){

                    case(g1):

                        popBit(bitboards[whiteRook], h1);
                        setBit(bitboards[whiteRook], f1);

                        hashKey ^= PIECE_KEYS[whiteRook][h1];
                        hashKey ^= PIECE_KEYS[whiteRook][f1];

                        break;

                    case(c1):

                        popBit(bitboards[whiteRook], a1);
                        setBit(bitboards[whiteRook], d1);

                        hashKey ^= PIECE_KEYS[whiteRook][a1];
                        hashKey ^= PIECE_KEYS[whiteRook][d1];

                        break;

                    case(g8):

                        popBit(bitboards[blackRook], h8);
                        setBit(bitboards[blackRook], f8);

                        hashKey ^= PIECE_KEYS[blackRook][h8];
                        hashKey ^= PIECE_KEYS[blackRook][f8];

                        break;

                    case(c8):

                        popBit(bitboards[blackRook], a8);
                        setBit(bitboards[blackRook], d8);

                        hashKey ^= PIECE_KEYS[blackRook][a8];
                        hashKey ^= PIECE_KEYS[blackRook][d8];

                        break;
                }
            }

            hashKey ^= CASTLING_KEYS[canCastle];

            canCastle &= CASTLE_STATE[startSquareIndex];
            canCastle &= CASTLE_STATE[targetSquareIndex];

            hashKey ^= CASTLING_KEYS[canCastle];

            resetOcuupancies();
            populateOccupancies();

            if(isKingInCheck()){

                memcpy(bitboards, tempBitboards, sizeof(tempBitboards));
                memcpy(occupancies, tempOccupancies, sizeof(tempOccupancies));
                enPassantSquareIndex = tempEnPassantSquareIndex;
                canCastle = tempCanCastle; 

                return 0;
            }

            switchSideToMove();
            hashKey ^= SIDE_KEY;

            return 1;
        }

        void loadFenString(const string& fenString){

            resetBitboards();
            resetOcuupancies();

            int squareIndex = 0;
            enPassantSquareIndex = NO_SQUARE_INDEX;           

            for(int index = 0; index < fenString.length(); index++){

                char symbol = fenString[index];

                if(isalpha(symbol)){   
                    
                    //Parsing the board state
                    if(squareIndex < 64){

                        setBit(bitboards[PIECE_INDEX_TO_ASCII.find(symbol)], squareIndex);
                        squareIndex++;

                    //Parsing other data (castling, en passant etc.) 
                    }else{

                        switch (symbol)
                        {
                        case 'w': sideToMove = white; break;
                        case 'b': sideToMove = black; break;
                        case 'K': canCastle |= K; break;
                        case 'Q': canCastle |= Q; break;
                        case 'k': canCastle |= k; break;
                        case 'q': canCastle |= q; break;
                        }

                    }

                    //En passant square
                    if((fenString[index - 1] == ' ') && (isdigit(fenString[index + 1]))){

                        int file = symbol - 'a';
                        int rank = 8 - (fenString[index + 1] - '0');
                        enPassantSquareIndex = rank * 8 + file;

                    }

                }else if(isdigit(symbol)){
                    squareIndex += (symbol - '0');
                }
            }
            populateOccupancies();
        }

        void loadMoveString(const string& moveString){

            int startSquareIndex = (moveString[0] - 'a') + (8 - (moveString[1] - '0')) * 8;
            int targetSquareIndex = (moveString[2] - 'a') + (8 - (moveString[3] - '0')) * 8;

            MoveList moves = generateMoves();

            for(int moveIndex = 0; moveIndex < moves.getCount(); moveIndex++){

                int move = moves.getMoves()[moveIndex];

                if(startSquareIndex == getStartSquareIndex(move) && targetSquareIndex == getTargetSquareIndex(move)){

                    if((moveString[3] == '8' && moveString[4] == 'P') || (moveString[3] == '1' && moveString[4] == 'p')){
                        return;
                    }

                    if(moveString[4]){
                        if((PIECE_INDEX_TO_ASCII[getPromotedPiece(move)] != moveString[4]) || (PIECE_INDEX_TO_ASCII[getPromotedPiece(move) - 6] != moveString[4])){
                            return;
                        }
                    }
                    
                    makeMove(move);
                }
            }
        }

        int staticEvaluate(){

            int score = 0;
            int squareIndex;

            for(int currentPiece = whitePawn; currentPiece <= blackKing; currentPiece++){

                U64 currentPieceBitboard = bitboards[currentPiece];

                while(currentPieceBitboard){

                    score += MATERIAL_SCORE[currentPiece];
                    squareIndex = getLS1BIndex(currentPieceBitboard);
                    popBit(currentPieceBitboard, squareIndex);
                
                    switch(currentPiece){

                    case(whitePawn):
                        score += PAWN_SCORE[squareIndex];
                        break;
                    case(whiteKnight):
                        score += KNIGHT_SCORE[squareIndex];
                        break;
                    case(whiteBishop):
                        score += BISHOP_SCORE[squareIndex];
                        break;
                    case(whiteRook):
                        score += ROOK_SCORE[squareIndex];
                        break;
                    case(whiteKing):
                        score += PAWN_SCORE[squareIndex];
                        break;
                    case(blackPawn):
                        score -= PAWN_SCORE[OPPOSITE_SIDE[squareIndex]];
                        break;
                    case(blackKnight):
                        score -= KNIGHT_SCORE[OPPOSITE_SIDE[squareIndex]];
                        break;
                    case(blackBishop):
                        score -= BISHOP_SCORE[OPPOSITE_SIDE[squareIndex]];
                        break;
                    case(blackRook):
                        score -= ROOK_SCORE[OPPOSITE_SIDE[squareIndex]];
                        break; 
                    case(blackKing):
                        score -= KING_SCORE[OPPOSITE_SIDE[squareIndex]];
                        break;
                    }

                }
            }
            //In negamax the score is evaluated relative to the side
            return (sideToMove == white) ? score : -score;
        }

        int getSideToMove(){
            return sideToMove;
        }

        void resetEnPassantSquareIndex(){
            enPassantSquareIndex = NO_SQUARE_INDEX;
        }

        U64* getBitboards(){
            return bitboards;
        }

};


//********Position********\\

class Position{

    private:
         
        Board currentBoard;
        int killerMoves[2][MAX_SEARCH_DEPTH];
        int historyMoves[12][64];

        int pvTable[MAX_SEARCH_DEPTH][MAX_SEARCH_DEPTH];
        int pvLength[MAX_SEARCH_DEPTH];

        bool fPVScore = false;
        bool fPVFollow = true;

        int bestMove;
        int ply, searchNodes;

    public:

        Position(string fenString){
            currentBoard = Board(fenString);
            ply = 0, searchNodes = 0;
        }

        U64 perft(int depth){

            U64 nodes = 0ULL;

            if(depth == 0){
                return 1ULL;
            }

            MoveList moves = currentBoard.generateMoves();

            for(int moveIndex = 0; moveIndex < moves.getCount(); moveIndex++){
                
                Board temporaryBoard = currentBoard;

                if(!currentBoard.makeMove(moves.getMoves()[moveIndex])){
                    continue;
                }

                nodes += perft(depth - 1);
                currentBoard = temporaryBoard;
            }
            return nodes;
        }

        void perftDebugInfo(int depth){

            cout << "\n    Performance test\n\n";

            U64 nodes = 0ULL;
            MoveList moves = currentBoard.generateMoves();

            for(int moveIndex = 0; moveIndex < moves.getCount(); moveIndex++){

                int currentMove = moves.getMoves()[moveIndex];

                Board temporaryBoard = currentBoard;

                if(!currentBoard.makeMove(currentMove)){
                    continue;
                }

                U64 currentNodes = perft(depth - 1);
                nodes += currentNodes;

                currentBoard = temporaryBoard;

                cout << "Move: "<< SQUARE_INDEX_TO_COORDINATES[getStartSquareIndex(currentMove)] << SQUARE_INDEX_TO_COORDINATES[getTargetSquareIndex(currentMove)]; 
                cout << ((getPromotedPiece(currentMove) != 0) ? PIECE_INDEX_TO_ASCII[getPromotedPiece(currentMove)] : ' ');
                cout << "\tnodes: " << currentNodes << '\n';

            }   

            cout << "\nDepth: " << depth;
            cout << "\nTotal number of nodes: " << nodes;
        }

        void setPVScore(MoveList& moveList){

            fPVFollow = false;

            for(int moveIndex = 0; moveIndex < moveList.getCount(); moveIndex++){
                if(moveList.getMoves()[moveIndex] == pvTable[0][ply]){

                    fPVScore = true;
                    fPVFollow = true;

                }
            }  
        }

        int scoreMove(int move){

            if(fPVScore && pvTable[0][ply] == move){
                
                fPVScore = false;
                return 20000;
                
            }

            if(isCapture(move)){

                int targetPiece = 0, startPiece, endPiece;

                if(currentBoard.getSideToMove() == white){

                    startPiece = blackPawn;
                    endPiece = blackKing;

                }else if(currentBoard.getSideToMove() == black){

                    startPiece = whitePawn;
                    endPiece = whiteKing;
                }

                for(int currentPiece = startPiece; currentPiece <= endPiece; currentPiece++){

                    if(getBit(currentBoard.getBitboards()[currentPiece], getTargetSquareIndex(move))){
                        targetPiece = currentPiece;
                        break;
                    }
                }

                return MVV_LVA[getPiece(move)][targetPiece] + 10000;

            }else if(killerMoves[0][ply] == move){

                return 9000;

            }else if(killerMoves[1][ply] == move){

                return 8000;

            }else{

                return historyMoves[getPiece(move)][getTargetSquareIndex(move)];
            }
        }

        void merge(int* moveArray, int leftIndex, int middleIndex, int rightIndex){

            int leftArraySize = middleIndex - leftIndex + 1;
            int rightArraySize = rightIndex - middleIndex;

            int leftArray[leftArraySize], rightArray[rightArraySize];

            for(int i = 0; i < leftArraySize; i++){
                leftArray[i] = moveArray[leftIndex + i];
            }

            for(int j = 0; j < rightArraySize; j++){
                rightArray[j] = moveArray[middleIndex + j + 1];
            }

            int i = 0, j = 0, k = leftIndex;

            while (i < leftArraySize && j < rightArraySize){

                if(scoreMove(leftArray[i]) > scoreMove(rightArray[j])){
                    moveArray[k] = leftArray[i];
                    i++;
                }else{
                    moveArray[k] = rightArray[j];
                    j++;
                }

                k++;
            }

            while(i < leftArraySize){
                moveArray[k] = leftArray[i];
                i++; k++;
            }

            while(j < rightArraySize){
                moveArray[k] = rightArray[j];
                j++; k++;
            }
        }

        void mergeSort(int* moveArray, int leftIndex, int rightIndex){

            if(leftIndex < rightIndex){

                int middleIndex = leftIndex + (rightIndex - leftIndex) / 2;
                mergeSort(moveArray, leftIndex, middleIndex);
                mergeSort(moveArray, middleIndex + 1, rightIndex);
                merge(moveArray, leftIndex, middleIndex, rightIndex);
            }
        }

        void sortMoves(MoveList& moveList){
            mergeSort(moveList.getMoves(), 0, moveList.getCount() - 1);
        }

        int quiescence(int alpha, int beta){

            searchNodes++;
            int evaluation = currentBoard.staticEvaluate();

            if(evaluation >= beta){
                return beta;
            }

            if(evaluation > alpha){
                alpha = evaluation;
            }

            MoveList moves = currentBoard.generateMoves();

            sortMoves(moves);

            for(int moveIndex = 0; moveIndex < moves.getCount(); moveIndex++){

                int currentMove = moves.getMoves()[moveIndex];
                
                if(isCapture(currentMove)){

                    Board temporaryBoard = currentBoard;
                    ply++;

                    if(!currentBoard.makeMove(moves.getMoves()[moveIndex])){
                        ply--;
                        continue;
                    }

                    int score = -quiescence(-beta, -alpha);
                    ply--;
                    currentBoard = temporaryBoard;

                    if(score >= beta){
                        return beta;
                    }

                    if(score > alpha){
                        alpha = score;
                    }
                }
   
            }
            return alpha;
        }

        int negamax(int alpha, int beta, int depth){

            pvLength[ply] = ply;

            if(depth == 0){
                return quiescence(alpha, beta);
            }

            if(ply > MAX_SEARCH_DEPTH - 1){
                return currentBoard.staticEvaluate();
            }

            searchNodes++;
            bool inCheck = currentBoard.isKingInCheck();

            if(inCheck){
                depth++;
            }

            int legalMoves = 0;
            int currentBestMove;

            if(depth >= REDUCTION_LIMIT && !inCheck && ply){

                Board nullMoveTemporaryBoard = currentBoard;
                currentBoard.switchSideToMove();
                currentBoard.resetEnPassantSquareIndex();

                int nullMoveScore = -negamax(-beta, -beta + 1, depth - REDUCTION_LIMIT);

                currentBoard = nullMoveTemporaryBoard;

                if(nullMoveScore >= beta){
                    return beta;
                }
            }
            
            MoveList moves = currentBoard.generateMoves();

            if(fPVFollow){
                setPVScore(moves);
            }

            sortMoves(moves);

            int movesSearched = 0;

            for(int moveIndex = 0; moveIndex < moves.getCount(); moveIndex++){

                Board temporaryBoard = currentBoard;
                int currentMove = moves.getMoves()[moveIndex];
                ply++;

                if(!currentBoard.makeMove(currentMove)){
                    ply--;
                    continue;
                }

                legalMoves++;

                int score;

                if(movesSearched == 0){
                    score = -negamax(-beta, -alpha, depth - 1);
                }else{

                    if(
                        movesSearched >= FULL_DEPTH_MOVES &&
                        depth >= REDUCTION_LIMIT &&
                        inCheck == false &&
                        !isCapture(currentMove) &&
                        !getPromotedPiece(currentMove)
                    ){
                        score = -negamax(-alpha - 1, -alpha, depth - 2);
                    }else{
                        score = alpha + 1;
                    }

                    if(score > alpha){

                        score = -negamax(-alpha - 1, -alpha, depth - 1);

                        if((score > alpha) && (score < beta)){
                            score = -negamax(-beta, -alpha, depth - 1);
                        }
                    }
                }

                ply--;
                movesSearched++;
                currentBoard = temporaryBoard;

                if(score >= beta){
                    
                    if(!isCapture(currentMove)){
                        killerMoves[1][ply] = killerMoves[0][ply];
                        killerMoves[0][ply] = currentMove;
                    }

                    return beta;
                }

                if(score > alpha){

                    if(!isCapture(currentMove)){
                        historyMoves[getPiece(currentMove)][getTargetSquareIndex(currentMove)];
                    }

                    alpha = score;

                    pvTable[ply][ply] = currentMove;

                    for(int nextPly = ply + 1; nextPly < pvLength[ply + 1]; nextPly++){
                        pvTable[ply][nextPly] = pvTable[ply + 1][nextPly]; 
                    }

                    pvLength[ply] = pvLength[ply + 1];

                    if(!ply){
                        bestMove = currentMove;
                    }
                }
            }

            if(!legalMoves){

                if(inCheck){
                    return CHECKMATE_SCORE + ply;
                }else{
                    return STALEMATE_SCORE;
                }
            }

            return alpha;
        }

        int getSearchNodes(){
            return searchNodes;
        }

        void printPV(){
            for(int i = 0; i < pvLength[0]; i++){
                printMove(pvTable[0][i]);
                cout << ' ';
            }
        }

        void resetSearchVariables(){

            bestMove = 0, searchNodes = 0, ply = 0;
            memset(killerMoves, 0, sizeof(killerMoves));
            memset(historyMoves, 0, sizeof(historyMoves));
            memset(pvTable, 0, sizeof(pvTable));
            memset(pvLength, 0, sizeof(pvLength));

        }

        int getBestMove(){
            return bestMove;
        }

        Board getBoard(){
            return currentBoard;
        }
};

void search(string fenString, int depth){

    generateKeys();

    Position position(fenString);
    position.getBoard().printState();
    position.resetSearchVariables();

    int alpha = -50000, beta = 50000;

    auto start = high_resolution_clock::now();

    for(int currentDepth = 1; currentDepth <= depth; currentDepth++){

        int score = position.negamax(alpha, beta, currentDepth);

        if((score <= alpha) || (score >= beta)){
            alpha = -50000;
            beta = 50000;
            continue;
        }

        alpha = score - ASPIRATION_WINDOW;
        beta  = score + ASPIRATION_WINDOW;

        cout << "\nEvaluation: " << score;
        cout << "\nSearch Nodes: " << position.getSearchNodes();
        cout << "\nPrincipled variation: ";
        position.printPV();
        cout << '\n';

    }

    cout << "\nBest Move: ";
    printMove(position.getBestMove());

    cout << "\n\nTime: " << std::chrono::duration_cast<std::chrono::microseconds>(high_resolution_clock::now() - start).count() << " mircoseconds\n\n";
    
}

int main(int argc, char* args[]){

    search(START_POSITION_FEN, 9);
    return 0;
}

