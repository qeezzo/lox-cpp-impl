#pragma once

#include <exception>
#include <iostream>
#include <string>

namespace lox
{
    void report(int line, std::string stage, std::string msg);
    void report(std::ostream& out, int line, std::string stage, std::string msg);
}
