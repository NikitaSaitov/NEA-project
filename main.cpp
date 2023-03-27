//********Libraries, files and custom types********
#include <iostream>
#include <cstring>
#include <chrono>
#include <vector>
#include "const.cpp"

using U64 = unsigned long long;

//********square <-> squareIndex conversion and other useful enumerations********
//Convert a square into the squareIndex
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

/*
Convert a square into the squareIndex from the black piece perspective
Used in the staticEvaluate() function
*/
//!TECHNIQUE-C: Single-dimensional arrays
const int OPPOSITE_SIDE[64] = {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

//Enumerate pieces
enum {
    whitePawn, whiteKnight, whiteBishop, whiteRook, whiteQueen, whiteKing, 
    blackPawn, blackKnight, blackBishop, blackRook, blackQueen, blackKing 
};

//Enumerate colours
enum {white, black, both};

//Enumerate castling rights
enum {K=1, Q=2, k=4, q=8};

//********Bit Manipulation********
//Set a bit to '1' at the given squareIndex on a given bitboard
inline void setBit(U64& bitboard, int squareIndex){
    bitboard |= (1ULL << squareIndex);
}

//Pop a bit at the given squareIndex from a given bitboard 
inline void popBit(U64& bitboard, int squareIndex){
    bitboard & (1ULL << squareIndex) ? bitboard ^= (1ULL << squareIndex) : 0;
}

//Get a bit at the given squareIndex from a given bitboard
//!TECHNIQUE-B: Simple user defined algorithms
inline int getBit(U64 bitboard, int squareIndex){
    return (bitboard & (1ULL << squareIndex))? 1 : 0;
}

//Get the number of bits set to '1' on a given bitboard (cardinality)
inline int getPopulationCount(U64 bitboard){

    //!TECHNIQUE-C: Appropriate choice of simple data types
    int populationCount = 0;
    
    while(bitboard){
        populationCount++;
        bitboard &= bitboard - 1;
    }
    return populationCount;
}

//Get the index of the least significant bit on a given biboard
//!TECHNIQUE-B: Simple user defined algorithms
inline int getLS1BIndex(U64 bitboard){

    if(bitboard){
        //Two's complement trick to isolate the LS1B, -1 to get the index 
        return getPopulationCount((bitboard & -bitboard) - 1);
    //If an empty bitboard is passed
    }else{
        return -1;
    }
}

//********Debug********
//Print the bitboard
void printBitboard(const U64& bitboard){

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

//********Pseudo-random numbers********
//Crete a seed for other random functions
void seedRandom(){
    srand(time(NULL));
}

//Get a random bitboard
//!TECHNIQUE-B: Simple user defined algorithms
U64 getRandom(){

    U64 r1, r2, r3, r4;

    r1 = (U64)(rand()) & 0xFFFF;
    r2 = (U64)(rand()) & 0xFFFF;
    r3 = (U64)(rand()) & 0xFFFF;
    r4 = (U64)(rand()) & 0xFFFF;    

    return r1 | (r2 << 16) | (r3 << 32) | (r4 << 48);
}

//Get a random sparsely populated bitboard
U64 getRandomFewBits(){
    return getRandom() & getRandom() & getRandom();
}

//********Move encoding********

inline int createMove(int startSquareIndex, int targetSquareIndex, int piece, int promotedPiece, bool isCapture, bool isDoublePawnPush, bool isEnPassant, bool isCastling){
    return startSquareIndex | (targetSquareIndex << 6) | (piece << 12) | (promotedPiece << 16) | (isCapture << 20) | (isDoublePawnPush << 21) | (isEnPassant << 22) | (isCastling << 23);
}

inline int getStartSquareIndex(int move){
    return move & 0x3f;
}

inline int getTargetSquareIndex(int move){
    return (move & 0xfc0) >> 6;
}

inline int getPiece(int move){
    return (move & 0xf000) >> 12;
}

inline int getPromotedPiece(int move){
    return (move & 0xf0000) >> 16;
}

inline bool isCapture(int move){
    return move & 0x100000;
}

inline bool isDoublePawnPush(int move){
    return move & 0x200000;
}

inline bool isEnPassant(int move){
    return move & 0x400000;
}

inline bool isCastling(int move){
    return move & 0x800000;
}

inline void printMove(int move){
    if(getPromotedPiece(move)){
       std::cout << pSQUARE_INDEX_TO_COORDINATES[getStartSquareIndex(move)] << pSQUARE_INDEX_TO_COORDINATES[getTargetSquareIndex(move)] << PIECE_INDEX_TO_ASCII[getPromotedPiece(move)];
    }else{
       std::cout << pSQUARE_INDEX_TO_COORDINATES[getStartSquareIndex(move)] << pSQUARE_INDEX_TO_COORDINATES[getTargetSquareIndex(move)];
    }
}

//********Attack Table********
//!TECHNIQUE-A: Complex user-defined use of OOP model (classes)
//!TECHNIQUE-B: Simple OOP model
class AttackTable{

    private:

        //!TECHNIQUE-B: Multidimentional arrays
        U64 pawnAttacks[2][64];
        U64 knightAttacks[64];
        U64 kingAttacks[64];
        U64 bishopMasks[64];
        U64 rookMasks[64];
        U64 bishopAttacks[64][512];
        U64 rookAttacks[64][4096];

        //Return a bitboard of pawn attacks for a given squareIndex and side
        U64 maskPawnAttacks(int squareIndex, int sideToMove){

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

        //Return a bitboard of king attacks for a given squareIndex
        U64 maskKingAttacks(int squareIndex){

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

        //Return a bitboard of knight attacks for a given squareIndex
        U64 maskKnightAttacks(int squareIndex){

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

        //Return a bitboard of bishop attacks for a given squareIndex, the outermost bits are dropped to speed up move generation
        U64 maskBishopAttacks(int squareIndex){

            U64 attacksBitboard = 0ULL;

            //!TECHNIQUE-C: Appropriate choice of simple data types
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

        //Return a bitboard of rook attacks for a given squareIndex, the outermost bits are dropped to speed up move generation
        U64 maskRookAttacks(int squareIndex){

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

        //Return a bitboard of bishop attacks for a given square and occupancy
        U64 generateBishopAttacks(int squareIndex, const U64& occupancy){

            U64 attacksBitboard = 0ULL;
            
            int rank, file;

            //!TECHNIQUE-C: Simple mathematical calculations
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
 
        //Return a bitboard of rook attacks for a given square and occupancy
        U64 generateRookAttacks(int squareIndex, const U64& occupancy){

            U64 attacksBitboard = 0ULL;

            int rank, file;

            int targetRank = squareIndex / 8;
            int targetFile = squareIndex % 8;

            //!TECHNIQUE-C: Linear search
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

        //Return a bitboard of occupanies for a given piece, square and occupancy index
        U64 setOccupancy(int occupancyIndex, int relevantBits, U64 attackMask){

            U64 bitboard = 0ULL;

            //!TECHNIQUE-C: Linear search
            for (int currentBit = 0; currentBit < relevantBits; currentBit++){
                
                int squareIndex = getLS1BIndex(attackMask);
                popBit(attackMask, squareIndex);

                if (occupancyIndex & (1 << currentBit)){
                    setBit(bitboard, squareIndex);
                }
            }
            return bitboard;
        }

        //Find a magic number for a given square and a given sliding piece
        //!TECHNIQUE-A: Complex user-defined algorithms
        U64 findMagicNumber(int squareIndex, int relevantBits, bool bishop){

            U64 occupancies[4096], attacks[4096], usedAttacks[4096];
            //Assign the attack mask
            U64 attackMask = bishop ? maskBishopAttacks(squareIndex) : maskRookAttacks(squareIndex);
            //The position of the piece impacts the amount of squares it controls, which influences the number of possible occupancies
            //!TECHNIQUE-C: Simple mathematical calculations
            int maxOccupancyIndex = 1 << relevantBits;

            for(int index = 0; index < maxOccupancyIndex; index++){
                occupancies[index] = setOccupancy(index, relevantBits, attackMask);
                attacks[index] = bishop ? generateBishopAttacks(squareIndex, occupancies[index]) : generateRookAttacks(squareIndex, occupancies[index]);
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

                //!TECHNIQUE-C: Appropriate choice of simple data types
                int index;
                bool fail;

                //!TECHNIQUE-C: Linear search
                for(index = 0, fail = false; !fail && index < maxOccupancyIndex; index++){

                    //Generate the magic number and destroy the garbage bits
                    int magicIndex = (int)((occupancies[index] * magicNumber) >> (64 - relevantBits));

                    //Test the magic number
                    if(usedAttacks[magicIndex] == 0ULL){
                        usedAttacks[magicIndex] = attacks[index];
                    }else if(usedAttacks[magicIndex] != attacks[index]){
                        fail = true;
                    }
                }

                if(!fail){
                    return magicNumber;
                }
            } 

            //Throw an exception if the magic number wasn't found
            throw "MAGIC NUMBER NOT FOUND!";
            return 0ULL;
        }

        //Initialize attack tables for the leaping pieces
        void initializeLeapingPieceTables(){

            for(int squareIndex = 0; squareIndex < 64; squareIndex++){

                pawnAttacks[white][squareIndex] = maskPawnAttacks(squareIndex, white);
                pawnAttacks[black][squareIndex] = maskPawnAttacks(squareIndex, black);
                knightAttacks[squareIndex] = maskKnightAttacks(squareIndex);
                kingAttacks[squareIndex] = maskKingAttacks(squareIndex);

            }
        }

        //Initialize attack tables for the sliding pieces
        void initializeSlidingPieceTables(bool bishop){

            for(int squareIndex = 0; squareIndex < 64; squareIndex++){

                bishopMasks[squareIndex] = maskBishopAttacks(squareIndex);
                rookMasks[squareIndex] = maskRookAttacks(squareIndex);

                U64 attackMask = bishop ? bishopMasks[squareIndex] : rookMasks[squareIndex];

                int relevantBits = getPopulationCount(attackMask);
                int maxOccupancyIndex = 1 << relevantBits;

                for(int occupancyIndex = 0; occupancyIndex < maxOccupancyIndex; occupancyIndex++){

                    U64 occupancy = setOccupancy(occupancyIndex, relevantBits, attackMask);
                        
                    if(bishop){

                        int magicIndex = (occupancy * BISHOP_MAGIC_NUMBERS[squareIndex]) >> (64 - BISHOP_RELEVANT_BITS[squareIndex]);
                        bishopAttacks[squareIndex][magicIndex] = generateBishopAttacks(squareIndex, occupancy);

                    }
                    else{

                        int magicIndex = (occupancy * ROOK_MAGIC_NUMBERS[squareIndex]) >> (64 - ROOK_RELEVANT_BITS[squareIndex]);
                        rookAttacks[squareIndex][magicIndex] = generateRookAttacks(squareIndex, occupancy);

                    }
                }
            }
        }

    public:

        //Initialize all attack tables
        AttackTable(){

            initializeLeapingPieceTables();
            initializeSlidingPieceTables(true);
            initializeSlidingPieceTables(false);
        
        }

        //Get pawn attacks at the given squareIndex for a given side
        U64 getPawnAttacks(int sideToMove, int squareIndex){
            return pawnAttacks[sideToMove][squareIndex];
        }

        //Get knight attacks at the given squareIndex
        U64 getKnightAttacks(int squareIndex){
            return knightAttacks[squareIndex];
        }

        //Get king attacks at the given squareIndex
        U64 getKingAttacks(int squareIndex){
            return kingAttacks[squareIndex];
        }

        //Get bishop attacks at the given squareIndex for a given occupancy
        U64 getBishopAttacks(int squareIndex, U64 occupancy){

            occupancy &= bishopMasks[squareIndex]; 
            occupancy *= BISHOP_MAGIC_NUMBERS[squareIndex];
            occupancy >>= 64 - BISHOP_RELEVANT_BITS[squareIndex];
            return bishopAttacks[squareIndex][occupancy];

        }

        //Get rook attacks at the given squareIndex for a given occupancy
        U64 getRookAttacks(int squareIndex, U64 occupancy){
            
            occupancy &= rookMasks[squareIndex]; 
            occupancy *= ROOK_MAGIC_NUMBERS[squareIndex];
            occupancy >>= 64 - ROOK_RELEVANT_BITS[squareIndex];
            return rookAttacks[squareIndex][occupancy];

        }

        //Get queen attacks at the given squareIndex for a given occupancy
        U64 getQueenAttacks(int squareIndex, U64 occupancy){
            return getBishopAttacks(squareIndex, occupancy) | getRookAttacks(squareIndex, occupancy);
        }

        //Calculate and print masking constants
        void printMaskingConstants(){

            U64 notABitboard = 0, notABBitboard = 0, notHBitboard = 0, notGHBitboard = 0;

            for(int rank = 0; rank < 8; rank++){
                for(int file = 0; file < 8; file++){
                    int squareIndex = rank * 8 + file;
                    if(file != 0){
                        setBit(notABitboard, squareIndex);
                    }
                }
            }
            printBitboard(notABitboard);

            for(int rank = 0; rank < 8; rank++){
                for(int file = 0; file < 8; file++){
                    int squareIndex = rank * 8 + file;
                    if(file != 0 && file != 1){
                        setBit(notABBitboard, squareIndex);
                    }
                }
            }
            printBitboard(notABBitboard);

            for(int rank = 0; rank < 8; rank++){
                for(int file = 0; file < 8; file++){
                    int squareIndex = rank * 8 + file;
                    if(file != 7){
                        setBit(notHBitboard, squareIndex);
                    }
                }
            }
            printBitboard(notHBitboard);

            for(int rank = 0; rank < 8; rank++){
                for(int file = 0; file < 8; file++){
                    int squareIndex = rank * 8 + file;
                    if(file != 6 && file != 7){
                        setBit(notGHBitboard, squareIndex);
                    }
                }
            }
            printBitboard(notGHBitboard);
        }

        //Calculate and print relevant bit constants
        void printRelevantBitConstants(){

            std::cout << '\n' << "BISHOP RELEVANT BITS: \n\n";
            for(int rank = 0; rank < 8; rank++){
                for(int file = 0; file < 8; file++){
                    int squareIndex = rank * 8 + file;
                    std::cout << getPopulationCount(maskBishopAttacks(squareIndex)) << ", ";
                }
                std::cout << '\n';
            }

            std::cout << '\n' << "ROOK RELEVANT BITS: \n\n";
            for(int rank = 0; rank < 8; rank++){
                for(int file = 0; file < 8; file++){
                    int squareIndex = rank * 8 + file;
                    std::cout << getPopulationCount(maskRookAttacks(squareIndex)) << ", ";
                }
                std::cout << '\n';
            }
        }
        
        //Calculate and print magic number constants
        void printMagicNumberConstants(){

            std::cout << '\n' << "BISHOP MAGICS: \n\n";
            for(int squareIndex = 0; squareIndex < 64; squareIndex++){
                std::cout << findMagicNumber(squareIndex, BISHOP_RELEVANT_BITS[squareIndex], true) << "ULL,\n";
            }

            std::cout << '\n' << "ROOK MAGICS: \n\n";
            for(int squareIndex = 0; squareIndex < 64; squareIndex++){
                std::cout << findMagicNumber(squareIndex, ROOK_RELEVANT_BITS[squareIndex], false) << "ULL,\n";
            }
        }  
};


//********MoveList********
//!TECHNIQUE-A: Complex user-defined use of OOP model (classes)
//!TECHNIQUE-B: Simple OOP model
class MoveList{

    private:

        //!TECHNIQUE-C: Single-dimensional arrays
        int moves[256];
        int count = 0;

    public:   

        void addMove(int startSquareIndex, int targetSquareIndex, int piece, int promotedPiece, bool capture, bool doublePawnPush, bool enPassant, bool castling){
            moves[count] = createMove(startSquareIndex, targetSquareIndex, piece, promotedPiece, capture, doublePawnPush, enPassant, castling);
            count++;
        }

        int getCount(){
            return count;
        }        

        int* getMoves(){
            return moves;
        }
};

//********Board********
//!TECHNIQUE-A: Complex user-defined use of OOP model (classes)
class Board{

    private:
        
        AttackTable* pAttackTable;
        U64 bitboards[12];
        U64 occupancies[3];

        int sideToMove = NO_SIDE_TO_MOVE;
        int enPassantSquareIndex = NO_SQUARE_INDEX;
        int canCastle = 0;

        //!TECHNIQUE-B: Multi-dimentioanl arrays
        U64 pieceKeys[12][64];
        U64 enPassantKeys[64];
        U64 castlingKeys[16];
        U64 sideKey; 
        U64 hashKey;

        void resetBitboards(){
            memset(bitboards, 0, sizeof(bitboards)); 
        }

        void resetOcuupancies(){
            memset(occupancies, 0, sizeof(occupancies));
        }

        bool isSquareAttacked(int squareIndex, int sideToMove){ 

            if((sideToMove == white) && (pAttackTable->getPawnAttacks(black, squareIndex) & bitboards[whitePawn])){
                return true;
            }
            if((sideToMove == black) && (pAttackTable->getPawnAttacks(white, squareIndex) & bitboards[blackPawn])){
                return true;
            }
            if(pAttackTable->getKnightAttacks(squareIndex) & ((sideToMove == white) ? bitboards[whiteKnight] : bitboards[blackKnight])){
                return true;
            }
            if(pAttackTable->getBishopAttacks(squareIndex, occupancies[both]) & ((sideToMove == white) ? bitboards[whiteBishop] : bitboards[blackBishop])){
                return true;
            }
            if(pAttackTable->getRookAttacks(squareIndex, occupancies[both]) & ((sideToMove == white) ? bitboards[whiteRook] : bitboards[blackRook])){
                return true;
            }
            if(pAttackTable->getQueenAttacks(squareIndex, occupancies[both]) & ((sideToMove == white) ? bitboards[whiteQueen] : bitboards[blackQueen])){
                return true;
            }
            if(pAttackTable->getKingAttacks(squareIndex) & ((sideToMove == white) ? bitboards[whiteKing] : bitboards[blackKing])){
                return true;
            }   
            return false;
        }

        //!TECHNIQUE-A: Hashing
        void createKeys(){

            seedRandom();

            for(int currentPiece = whitePawn; currentPiece <= blackKing; currentPiece++){

                for(int currentSquareIdex = 0; currentSquareIdex < 64; currentSquareIdex++){
                    pieceKeys[currentPiece][currentSquareIdex] = getRandom();
                }
            }

            for(int currentSquareIdex = 0; currentSquareIdex < 64; currentSquareIdex++){
                enPassantKeys[currentSquareIdex] = getRandom();
            }

            for(int castlingIndex = 0; castlingIndex < 16; castlingIndex++){
                castlingKeys[castlingIndex] = getRandom();
            }

            sideKey = getRandom();
        }

        //!TECHNIQUE-A: Hashing
        void createHash(){

            for(int currentPiece = whitePawn; currentPiece <= blackKing; currentPiece++){

               U64 currentBiboard = bitboards[currentPiece];

               while(currentBiboard){

                int squareIndex = getLS1BIndex(currentBiboard);
                hashKey ^= pieceKeys[currentPiece][squareIndex];
                popBit(currentBiboard, squareIndex);

               }
            }

            if (enPassantSquareIndex != NO_SQUARE_INDEX){
                hashKey ^= enPassantKeys[enPassantSquareIndex];
            }

            hashKey ^= castlingKeys[canCastle];

            if(sideToMove == black){
                hashKey ^= sideKey;
            }

        }

    public:

        Board(){}

        Board(std::string fenString, AttackTable* pAttackTable){

            this->pAttackTable = pAttackTable;
            loadFenString(fenString);

            createKeys();
            createHash();

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

            std::cout << '\n';

            for(int rank = 0; rank < 8; rank++){

                for(int file = 0; file < 8; file++){

                    //Least significant file (LSF) mapping
                    //!TECHNIQUE-C: Simple mathematical calculations
                    int squareIndex = rank * 8 + file;

                    //Print the ranks
                    if(!file){
                        std::cout << 8 - rank << "  ";
                    }

                    int piece = -1;

                    for(int currentPiece = whitePawn; currentPiece <= blackKing; currentPiece++){

                        if(getBit(bitboards[currentPiece], squareIndex)){
                            piece = currentPiece;
                        }
                    }
                    std::cout << ((piece == -1) ? '.' : PIECE_INDEX_TO_ASCII[piece]) << ' ';
                }
                std::cout << '\n';
            }
            //Print the files
            std::cout << '\n' << "   a b c d e f g h " << "\n\n";

            //Special position details
            std::cout << "Turn: " << ((sideToMove != NO_SIDE_TO_MOVE) ? ((!sideToMove) ? "white" : "black") : "not specified") << '\n';
            std::cout << "Can castle: " << ((canCastle & K) ? 'K' : '-') << ((canCastle & Q) ? 'Q' : '-') << ((canCastle & k) ? 'k' : '-') << ((canCastle & q) ? 'q' : '-') << '\n';
            std::cout << "EnPassant square: " << ((enPassantSquareIndex != NO_SQUARE_INDEX) ? pSQUARE_INDEX_TO_COORDINATES[enPassantSquareIndex] : "no square") << '\n';
            std::cout << "Hash key: " << hashKey << "\n\n";
        };

        void createPseudolegalMoves(MoveList& moveList){

            int startSquareIndex, targetSquareIndex;
            U64 currentPieceBitboard, currentPieceAttacks;

            for(int currentPiece = whitePawn; currentPiece <= blackKing; currentPiece++){

                currentPieceBitboard = bitboards[currentPiece];

                //White pawn moves & castling
                if(sideToMove == white){

                    //Pawns
                    if(currentPiece == whitePawn){

                        while(currentPieceBitboard){
                        
                            startSquareIndex = getLS1BIndex(currentPieceBitboard);
                            targetSquareIndex = startSquareIndex - 8;

                            //Quiet moves
                            if(!(targetSquareIndex < a8) && !getBit(occupancies[both], targetSquareIndex)){
                                
                                if(startSquareIndex >= a7 && startSquareIndex <= h7){

                                    for(int promotedPiece = whiteKnight; promotedPiece <= whiteQueen; promotedPiece++){
                                        moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, promotedPiece, 0, 0, 0, 0);
                                    }

                                }else{

                                    moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 0, 0, 0, 0);
                                    if((startSquareIndex >= a2 && startSquareIndex <= h2) && !getBit(occupancies[both], targetSquareIndex - 8)){
                                        moveList.addMove(startSquareIndex, targetSquareIndex - 8, currentPiece, 0, 0, 1, 0, 0);
                                    }

                                }
                            }

                            //Captures
                            currentPieceAttacks = pAttackTable->getPawnAttacks(sideToMove, startSquareIndex) & occupancies[black];

                            while (currentPieceAttacks){
                                
                                targetSquareIndex = getLS1BIndex(currentPieceAttacks);

                                if(startSquareIndex >= a7 && startSquareIndex <= h7){

                                    for(int promotedPiece = whiteKnight; promotedPiece <= whiteQueen; promotedPiece++){
                                        moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, promotedPiece, 1, 0, 0, 0);
                                    }

                                }else{
                                    moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 1, 0, 0, 0);
                                }
                                popBit(currentPieceAttacks, targetSquareIndex);
                            }

                            //En passant
                            if(enPassantSquareIndex != NO_SQUARE_INDEX){

                                U64 enPassantAttacks = pAttackTable->getPawnAttacks(sideToMove, startSquareIndex) & (1ULL << enPassantSquareIndex);
                                if(enPassantAttacks){
                                    int enPassantTarget = getLS1BIndex(enPassantAttacks);
                                    moveList.addMove(startSquareIndex, enPassantTarget, currentPiece, 0, 1, 0, 1, 0);
                                }
                            }
                            popBit(currentPieceBitboard, startSquareIndex);
                        }                       
                    }

                    //Castling
                    if(currentPiece == whiteKing){

                        if(canCastle & K){
                            if(!getBit(occupancies[both], f1) && !getBit(occupancies[both], g1) && !isSquareAttacked(e1, black) && !isSquareAttacked(f1, black)){
                                moveList.addMove(e1, g1, currentPiece, 0, 0, 0, 0, 1);
                            }
                        }

                        if(canCastle & Q){
                           if(!getBit(occupancies[both], d1) && !getBit(occupancies[both], c1) && !getBit(occupancies[both], b1) 
                           && !isSquareAttacked(e1, black) && !isSquareAttacked(d1, black)){
                                moveList.addMove(e1, c1, currentPiece, 0, 0, 0, 0, 1);
                            } 
                        }
                    }

                //Black pawn moves & castling
                }else if(sideToMove == black){

                    //Pawns
                    if(currentPiece == blackPawn){

                        while(currentPieceBitboard){
                            
                            startSquareIndex = getLS1BIndex(currentPieceBitboard);
                            targetSquareIndex = startSquareIndex + 8;

                            //Quiet moves
                            if(!(targetSquareIndex > h1) && !getBit(occupancies[both], targetSquareIndex)){
                                
                                if(startSquareIndex >= a2 && startSquareIndex <= h2){

                                    for(int promotedPiece = blackKnight; promotedPiece <= blackQueen; promotedPiece++){
                                        moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, promotedPiece, 0, 0, 0, 0);
                                    }
                                    
                                }else{
                                    moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 0, 0, 0, 0);
                                    if((startSquareIndex >= a7 && startSquareIndex <= h7) && !getBit(occupancies[both], targetSquareIndex + 8)){
                                        moveList.addMove(startSquareIndex, targetSquareIndex + 8, currentPiece, 0, 0, 1, 0, 0);
                                    }
                                }
                            }

                            currentPieceAttacks = pAttackTable->getPawnAttacks(sideToMove, startSquareIndex) & occupancies[white];

                            //Captures
                            while (currentPieceAttacks){
                                
                                targetSquareIndex = getLS1BIndex(currentPieceAttacks);

                                if(startSquareIndex >= a2 && startSquareIndex <= h2){

                                    for(int promotedPiece = blackKnight; promotedPiece <= blackQueen; promotedPiece++){
                                        moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, promotedPiece, 1, 0, 0, 0);
                                    }

                                }else{
                                    moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 1, 0, 0, 0);
                                }
                                popBit(currentPieceAttacks, targetSquareIndex);
                            }

                            //En passant
                            if(enPassantSquareIndex != NO_SQUARE_INDEX){

                                U64 enPassantAttacks = pAttackTable->getPawnAttacks(sideToMove, startSquareIndex) & (1ULL << enPassantSquareIndex);
                                if(enPassantAttacks){
                                    int enPassantTarget = getLS1BIndex(enPassantAttacks);
                                    moveList.addMove(startSquareIndex, enPassantTarget, currentPiece, 0, 1, 0, 1, 0);
                                }
                            }
                            popBit(currentPieceBitboard, startSquareIndex);
                        }
                    }

                    //Castling
                    if(currentPiece == blackKing){

                        if(canCastle & k){
                            if(!getBit(occupancies[both], f8) && !getBit(occupancies[both], g8) && !isSquareAttacked(e8, white) && !isSquareAttacked(f8, white)){
                                moveList.addMove(e8, g8, currentPiece, 0, 0, 0, 0, 1);
                            }
                        }

                        if(canCastle & q){
                           if(!getBit(occupancies[both], d8) && !getBit(occupancies[both], c8) && !getBit(occupancies[both], b8) 
                           && !isSquareAttacked(e8, white) && !isSquareAttacked(d8, white)){
                                moveList.addMove(e8, c8, currentPiece, 0, 0, 0, 0, 1);
                            } 
                        }
                    }
                }

                //Knights
                if((sideToMove == white) ? currentPiece == whiteKnight : currentPiece == blackKnight){

                    while(currentPieceBitboard){

                        startSquareIndex = getLS1BIndex(currentPieceBitboard);
                        currentPieceAttacks = pAttackTable->getKnightAttacks(startSquareIndex) & ((sideToMove == white) ? ~occupancies[white] : ~occupancies[black]);

                        while(currentPieceAttacks){

                            targetSquareIndex = getLS1BIndex(currentPieceAttacks);

                            //Quiet moves
                            if(!getBit(((sideToMove == white) ? occupancies[black] : occupancies[white]), targetSquareIndex)){
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 0, 0, 0, 0);
                            }else{
                                //Captures
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 1, 0, 0, 0);
                            }
                            popBit(currentPieceAttacks, targetSquareIndex);
                        }
                        popBit(currentPieceBitboard, startSquareIndex);
                    }
                }

                //Bishops
                if((sideToMove == white) ? currentPiece == whiteBishop : currentPiece == blackBishop){

                    while(currentPieceBitboard){

                        startSquareIndex = getLS1BIndex(currentPieceBitboard);
                        currentPieceAttacks = pAttackTable->getBishopAttacks(startSquareIndex, occupancies[both]) & ((sideToMove == white) ? ~occupancies[white] : ~occupancies[black]);

                        while(currentPieceAttacks){

                            targetSquareIndex = getLS1BIndex(currentPieceAttacks);

                            //Quiet moves
                            if(!getBit(((sideToMove == white) ? occupancies[black] : occupancies[white]), targetSquareIndex)){
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 0, 0, 0, 0);
                            }else{
                                //Captures
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 1, 0, 0, 0);
                            }
                            popBit(currentPieceAttacks, targetSquareIndex);
                        }
                        popBit(currentPieceBitboard, startSquareIndex);
                    }
                }
                
                //Rooks
                if((sideToMove == white) ? currentPiece == whiteRook : currentPiece == blackRook){

                    while(currentPieceBitboard){

                        startSquareIndex = getLS1BIndex(currentPieceBitboard);
                        currentPieceAttacks = pAttackTable->getRookAttacks(startSquareIndex, occupancies[both]) & ((sideToMove == white) ? ~occupancies[white] : ~occupancies[black]);

                        while(currentPieceAttacks){

                            targetSquareIndex = getLS1BIndex(currentPieceAttacks);

                            //Quiet moves
                            if(!getBit(((sideToMove == white) ? occupancies[black] : occupancies[white]), targetSquareIndex)){
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 0, 0, 0, 0);
                            }else{
                                //Captures
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 1, 0, 0, 0);
                            }
                            popBit(currentPieceAttacks, targetSquareIndex);
                        }
                        popBit(currentPieceBitboard, startSquareIndex);
                    }
                }
                
                //Queens
                if((sideToMove == white) ? currentPiece == whiteQueen : currentPiece == blackQueen){

                    while(currentPieceBitboard){

                        startSquareIndex = getLS1BIndex(currentPieceBitboard);
                        currentPieceAttacks = pAttackTable->getQueenAttacks(startSquareIndex, occupancies[both]) & ((sideToMove == white) ? ~occupancies[white] : ~occupancies[black]);

                        while(currentPieceAttacks){

                            targetSquareIndex = getLS1BIndex(currentPieceAttacks);

                            //Quiet
                            if(!getBit(((sideToMove == white) ? occupancies[black] : occupancies[white]), targetSquareIndex)){
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 0, 0, 0, 0);
                            }else{
                                //Captures
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 1, 0, 0, 0);
                            }
                            popBit(currentPieceAttacks, targetSquareIndex);
                        }
                        popBit(currentPieceBitboard, startSquareIndex);
                    }
                }
            
                //King
                if((sideToMove == white) ? currentPiece == whiteKing : currentPiece == blackKing){

                    while(currentPieceBitboard){

                        startSquareIndex = getLS1BIndex(currentPieceBitboard);
                        currentPieceAttacks = pAttackTable->getKingAttacks(startSquareIndex) & ((sideToMove == white) ? ~occupancies[white] : ~occupancies[black]);

                        while(currentPieceAttacks){

                            targetSquareIndex = getLS1BIndex(currentPieceAttacks);

                            //Quiet moves
                            if(!getBit(((sideToMove == white) ? occupancies[black] : occupancies[white]), targetSquareIndex)){
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 0, 0, 0, 0);
                            }else{
                                //Captures
                                moveList.addMove(startSquareIndex, targetSquareIndex, currentPiece, 0, 1, 0, 0, 0);
                            }
                            popBit(currentPieceAttacks, targetSquareIndex);
                        }
                        popBit(currentPieceBitboard, startSquareIndex);
                    }
                }
            }
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
                        break;
                    }
                }
            }

            if(promotedPiece){

                popBit(bitboards[(sideToMove == white) ? whitePawn : blackPawn], targetSquareIndex);
                setBit(bitboards[promotedPiece], targetSquareIndex);

            }

            if(isEnPassant(move)){
                (sideToMove == white) ? popBit(bitboards[blackPawn], targetSquareIndex + 8) : popBit(bitboards[whitePawn], targetSquareIndex - 8);
            }
            enPassantSquareIndex = NO_SQUARE_INDEX;

            if(isDoublePawnPush(move)){
                (sideToMove == white) ? (enPassantSquareIndex = targetSquareIndex + 8) : (enPassantSquareIndex = targetSquareIndex - 8);
            }

            if(isCastling(move)){

                switch(targetSquareIndex){

                    case(g1):

                        popBit(bitboards[whiteRook], h1);
                        setBit(bitboards[whiteRook], f1);
                        break;

                    case(c1):

                        popBit(bitboards[whiteRook], a1);
                        setBit(bitboards[whiteRook], d1);
                        break;

                    case(g8):

                        popBit(bitboards[blackRook], h8);
                        setBit(bitboards[blackRook], f8);
                        break;

                    case(c8):

                        popBit(bitboards[blackRook], a8);
                        setBit(bitboards[blackRook], d8);
                        break;
                }
            }

            canCastle &= CASTLE_STATE[startSquareIndex];
            canCastle &= CASTLE_STATE[targetSquareIndex];

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
            return 1;
        }

        void loadFenString(const std::string& fenString){

            resetBitboards();
            resetOcuupancies();

            int squareIndex = 0;
            enPassantSquareIndex = NO_SQUARE_INDEX;           

            for(int index = 0; index < fenString.length(); index++){

                char symbol = fenString[index];

                if(isalpha(symbol)){   
                    
                    //Parsing the board state
                    if(squareIndex < 64){

                        setBit(bitboards[ASCII_TO_PIECE_INDEX[symbol]], squareIndex);
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

        void loadMoveString(const std::string& moveString){

            int startSquareIndex = (moveString[0] - 'a') + (8 - (moveString[1] - '0')) * 8;
            int targetSquareIndex = (moveString[2] - 'a') + (8 - (moveString[3] - '0')) * 8;

            MoveList possibleMoves;
            createPseudolegalMoves(possibleMoves);

            for(int moveIndex = 0; moveIndex < possibleMoves.getCount(); moveIndex++){

                int move = possibleMoves.getMoves()[moveIndex];

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

//********Position********
//!TECHNIQUE-A: Complex user-defined use of OOP model (classes)
//!TECHNIQUE-B: Simple OOP model
class Position{

    private:
         
        Board currentBoard;
        //!TECHNIQUE-B: Multi-dimensional arrays
        int killerMoves[2][MAX_SEARCH_DEPTH];
        int historyMoves[12][64];

        int pvTable[MAX_SEARCH_DEPTH][MAX_SEARCH_DEPTH];
        //!TECHNIQUE-C: Single-dimensional arrays
        int pvLength[MAX_SEARCH_DEPTH];
        bool fPVScore = false;
        bool fPVFollow = false;

        int bestMove;
        int ply, searchNodes;

    public:

        Position(std::string fenString, AttackTable* pAttackTable){
            currentBoard = Board(fenString, pAttackTable);
            ply = 0, searchNodes = 0;
        }

        //!TECHNIQUE-A: Recursive algorithms
        U64 perft(int depth){

            U64 nodes = 0ULL;

            if(depth == 0){
                return 1ULL;
            }

            //!TECHNIQUE-A: Dynamic generation of objects
            MoveList moveList;
            currentBoard.createPseudolegalMoves(moveList);

            for(int moveIndex = 0; moveIndex < moveList.getCount(); moveIndex++){
                
                //!TECHNIQUE-A: Dynamic generation of objects
                Board temporaryBoard = currentBoard;

                if(!currentBoard.makeMove(moveList.getMoves()[moveIndex])){
                    continue;
                }

                nodes += perft(depth - 1);
                currentBoard = temporaryBoard;
            }
            return nodes;
        }

        void perftDebugInfo(int depth){

            std::cout << "\n    Performance test\n\n";

            U64 nodes = 0ULL;
            MoveList moveList;
            currentBoard.createPseudolegalMoves(moveList);

            auto start = std::chrono::high_resolution_clock::now();

            for(int moveIndex = 0; moveIndex < moveList.getCount(); moveIndex++){

                int currentMove = moveList.getMoves()[moveIndex];

                Board temporaryBoard = currentBoard;

                if(!currentBoard.makeMove(currentMove)){
                    continue;
                }

                U64 currentNodes = perft(depth - 1);
                nodes += currentNodes;

                currentBoard = temporaryBoard;

                std::cout << "Move: "<< pSQUARE_INDEX_TO_COORDINATES[getStartSquareIndex(currentMove)] << pSQUARE_INDEX_TO_COORDINATES[getTargetSquareIndex(currentMove)]; 
                std::cout << ((getPromotedPiece(currentMove) != 0) ? PIECE_INDEX_TO_ASCII[getPromotedPiece(currentMove)] : ' ');
                std::cout << "\tnodes: " << currentNodes << '\n';

            }

            std::cout << "\nDepth: " << depth;
            std::cout << "\nTotal number of nodes: " << nodes;
            std::cout << "\nTest time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() << " mircoseconds\n\n";
        }

        void allowPVScore(MoveList& moveList){

            fPVFollow = false;

            for(int moveIndex = 0; moveIndex < moveList.getCount(); moveIndex++){
                if(moveList.getMoves()[moveIndex] == pvTable[0][ply]){

                    fPVScore = true;
                    fPVFollow = true;

                }
            }  

        }

        int scoreMove(const int& move){

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

        //!TECHNIQUE-A: Mergesort or similarly efficient sort
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

        //!TECHNIQUE-A: Recursive algorithms
        int quiescence(int alpha, int beta){

            searchNodes++;
            int evaluation = currentBoard.staticEvaluate();

            if(evaluation >= beta){
                return beta;
            }

            if(evaluation > alpha){
                alpha = evaluation;
            }

            //!TECHNIQUE-A: Dynamic generation of objects
            MoveList moveList;
            currentBoard.createPseudolegalMoves(moveList);
            sortMoves(moveList);

            for(int moveIndex = 0; moveIndex < moveList.getCount(); moveIndex++){

                int currentMove = moveList.getMoves()[moveIndex];
                
                if(isCapture(currentMove)){

                    //!TECHNIQUE-A: Dynamic generation of objects
                    Board temporaryBoard = currentBoard;
                    ply++;

                    if(!currentBoard.makeMove(moveList.getMoves()[moveIndex])){
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

        //!TECHNIQUE-A: Recursive algorithms
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

            //!TECHNIQUE-A: Complex user-defined algorithms (optimisation)
            if(depth >= REDUCTION_LIMIT && !inCheck && ply){

                //!TECHNIQUE-A: Dynamic generation of objects
                Board nullMoveTemporaryBoard = currentBoard;
                currentBoard.switchSideToMove();
                currentBoard.resetEnPassantSquareIndex();

                int nullMoveScore = -negamax(-beta, -beta + 1, depth - REDUCTION_LIMIT);

                currentBoard = nullMoveTemporaryBoard;

                if(nullMoveScore >= beta){
                    return beta;
                }

            }
            
            //!TECHNIQUE-A: Dynamic generation of objects
            MoveList moveList;
            currentBoard.createPseudolegalMoves(moveList);

            if(fPVFollow){
                allowPVScore(moveList);
            }
            sortMoves(moveList);

            int movesSearched = 0;

            for(int moveIndex = 0; moveIndex < moveList.getCount(); moveIndex++){

                //!TECHNIQUE-A: Dynamic generation of objects
                Board temporaryBoard = currentBoard;
                int currentMove = moveList.getMoves()[moveIndex];
                ply++;

                if(!currentBoard.makeMove(currentMove)){
                    ply--;
                    continue;
                }

                legalMoves++;

                int score;

                //!TECHNIQUE-A: Complex user-defined algorithms (optimisation)
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

                //!TECHNIQUE-A: Complex user-defined algorithms (optimisation)
                if(score >= beta){
                    
                    if(!isCapture(currentMove)){
                        killerMoves[1][ply] = killerMoves[0][ply];
                        killerMoves[0][ply] = currentMove;
                    }

                    return beta;
                }

                //!TECHNIQUE-A: Complex user-defined algorithms (optimisation)
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
                std::cout << ' ';
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

void search(std::string fenString, int depth){

    //Heap memory is nessesary; max static memory size = 1MB, rookAttacks moveArray > 2MB
    //!TECHNIQUE-B: Generation of objects based on simple OOP model
    AttackTable* pAttackTable = new AttackTable(); 

    //!TECHNIQUE-B: Generation of objects based on simple OOP model
    Position position(fenString, pAttackTable);
    position.getBoard().printState();
    position.resetSearchVariables();

    int alpha = -50000, beta = 50000;

    auto start = std::chrono::high_resolution_clock::now();

    for(int currentDepth = 1; currentDepth <= depth; currentDepth++){

        int score = position.negamax(alpha, beta, currentDepth);

        if((score <= alpha) || (score >= beta)){
            alpha = -50000;
            beta = 50000;
            continue;
        }

        alpha = score - ASPIRATION_WINDOW;
        beta  = score + ASPIRATION_WINDOW;

        std::cout << "\nEvaluation: " << score;
        std::cout << "\nSearch Nodes: " << position.getSearchNodes();
        std::cout << "\nPrincipled variation: ";
        position.printPV();
        std::cout << '\n';

    }

    std::cout << "\nBest Move: ";
    printMove(position.getBestMove());

    std::cout << "\n\nTest time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() << " mircoseconds\n\n";

    //Cleanup heap memory
    delete pAttackTable;

}

int main(int argc, char* args[]){

    search(START_POSITION_FEN, 9);

    return 0;

}

