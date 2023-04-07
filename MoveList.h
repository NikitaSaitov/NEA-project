class MoveList
{
    private:

       int moves[256];
       int count = 0; 

    public:

        MoveList(){};

        void appendMove(int startSquareIndex, int targetSquareIndex, int piece, int promotedPiece, bool fCapture, bool fDoublePawnPush, bool fEnPassant, bool fCastling);
        
        int* getMoves();
        const int getCount();
};

