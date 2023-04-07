using U64 = unsigned long long;

class AttackTable
{
    private:
    
        U64 pawnAttacks[2][64];
        U64 knightAttacks[64];
        U64 kingAttacks[64];
        U64 bishopAttacks[64][512];
        U64 rookAttacks[64][4096];

        U64 bishopMasks[64];
        U64 rookMasks[64];

        U64 bishopMagics[64];
        U64 rookMagics[64];

        void initializeMagicNumbers();
        void initializeLeapingPieceTables();
        void initializeSlidingPieceTables(bool fBishop);

    public:

        AttackTable(){
            
            initializeMagicNumbers();
            initializeLeapingPieceTables();
            initializeSlidingPieceTables(true);
            initializeSlidingPieceTables(false);
        }

        const U64 getPawnAttacks(int sideToMove, int squareIndex);
        const U64 getKnightAttacks(int squareIndex);
        const U64 getKingAttacks(int squareIndex);
        const U64 getBishopAttacks(int squareIndex, U64 occupancy);
        const U64 getRookAttacks(int squareIndex, U64 occupancy);
        const U64 getQueenAttacks(int squareIndex, U64 occupancy);      
};

