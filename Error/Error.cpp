#include "Error.h"
using namespace lox;

extern bool hadError;
extern bool hadRuntimeError;

void lox::report(int line, std::string stage, std::string msg)
{
    hadError = true;
    std::cerr << "\e[31m[line " << line << "] " + stage + " Error: " << msg << std::endl;
}

void lox::report(std::ostream& out, int line, std::string stage, std::string msg)
{
    hadError = true;
    std::cerr << "[line " << line << "] " + stage + " Error: " << msg << std::endl;
}
