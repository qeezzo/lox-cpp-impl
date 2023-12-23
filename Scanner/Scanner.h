#pragma once
#include <istream>
#include <unordered_map>
#include <vector>

#include "../Types/Token.h"
#include "../Error/Error.h"

extern bool hadError;

namespace lox
{

class Scanner
{
private:

    std::string source;
    std::vector<Types::Token> tokens;

    static std::unordered_map<std::string, Types::TokenType> keywords;

    int start = 0;
    int current = 0;
    int line = 1;

    auto addToken(const Types::TokenType type) -> void;
    auto addToken(const Types::TokenType type, Types::Literal value) -> void;

    auto advance() -> char { return source[current++]; }
    auto isAtEnd() -> char { return (size_t) current >= source.length(); }
    auto match(char expected) -> bool;
    auto peek() -> char { if (isAtEnd()) return '\0'; return source[current]; }
    auto peekNext() -> char;

    auto string() -> void;
    auto character() -> void;
    auto number() -> void;
    auto integer_format() -> void;
    auto identifier() -> void;

    auto scanToken() -> void;

public:

    Scanner (std::string&& input):
        source(std::move(input))
    { }

    std::vector<Types::Token>&& scanTokens();

};

}
