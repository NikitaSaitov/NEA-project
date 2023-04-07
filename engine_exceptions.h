#ifndef ENGINE_EXCEPTIONS_H
#define ENGINE_EXCEPTIONS_H

#include <exception>

class LSBOfEmptyBitboardException : public std::exception
{
public:
    const char* what(); 
};

class HashKeysNotInitializedException : public std::exception
{
public:
    const char* what(); 
};

class CannotFindMagicNumberException : public std::exception
{
public:
    const char* what(); 
};

class MagicNumberNotInitializedException : public std::exception
{
public:
    const char* what(); 
};

#endif