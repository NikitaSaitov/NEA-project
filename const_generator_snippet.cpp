/*
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
*/
