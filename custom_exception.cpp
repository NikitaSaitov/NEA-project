#include <iostream>
#include <exception>

class LSBOfEmptyBitboardException : public std::exception{

    public:
        
        const char* what(){
            return "Cannot return the LSB of an empty bitboard";
        }

};

class HashKeysNotInitializedException : public std::exception{

    public:
    
        const char* what(){
            return "Hash keys not initialized";
        }
};

class CannotFindMagicNumberException : public std::exception{

    public:
    
        const char* what(){
            return "Could not find a magic number";
        }
};

class MagicNumberNotInitializedException : public std::exception{

    public:
    
        const char* what(){
            return "Magic numbers not initialized";
        }
};

