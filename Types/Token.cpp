#include "Token.h"
#include <sstream>
#include <variant>

using namespace lox::Types;

const std::string& lox::Types::TokenTypeString(const TokenType value) {
    static const std::map<TokenType, std::string> lookup{
        {LEFT_PAREN, "LEFT_PAREN"},
        {RIGHT_PAREN, "RIGHT_PAREN"},
        {LEFT_BRACE, "LEFT_BRACE"},
        {RIGHT_BRACE, "RIGHT_BRACE"},
        {COMMA, "COMMA"},
        {DOT, "DOT"},
        {MINUS, "MINUS"},
        {PLUS, "PLUS"},
        {SEMICOLON, "SEMICOLON"},
        {SLASH, "SLASH"},
        {STAR, "STAR"},
        {BANG, "BANG"},
        {BANG_EQUAL, "BANG_EQUAL"},
        {EQUAL, "EQUAL"},
        {EQUAL_EQUAL, "EQUAL_EQUAL"},
        {GREATER, "GREATER"},
        {GREATER_EQUAL, "GREATER_EQUAL"},
        {LESS, "LESS"},
        {LESS_EQUAL, "LESS_EQUAL"},
        {IDENTIFIER, "IDENTIFIER"},
        {STRING, "STRING"},
        {NUMBER, "NUMBER"},
        {AND, "AND"},
        {CLASS, "CLASS"},
        {ELSE, "ELSE"},
        {FALSE, "FALSE"},
        {FUN, "FUN"},
        {FOR, "FOR"},
        {IF, "IF"},
        {NIL, "NIL"},
        {OR, "OR"},
        {PRINT, "PRINT"},
        {RETURN, "RETURN"},
        {SUPER, "SUPER"},
        {THIS, "THIS"},
        {TRUE, "TRUE"},
        {VAR, "VAR"},
        {WHILE, "WHILE"}};
    return lookup.find(value)->second;
}

auto lox::Types::isNil(Literal &lit) -> bool {
    return std::holds_alternative<std::nullptr_t>(lit);
}

auto Stringify::operator()(std::nullptr_t) -> std::string { return "nil"; }

auto Stringify::operator()(std::string value) -> std::string { return value; }

auto Stringify::operator()(char value) -> std::string {
    return std::string(1, value);
}

auto Stringify::operator()(double value) -> std::string {
    std::ostringstream str;
    str << value;
    return str.str();
}

auto Stringify::operator()(uint8_t value) -> std::string {
    return std::to_string(value);
}

auto Stringify::operator()(int value) -> std::string {
    return std::to_string(value);
}

auto Stringify::operator()(Number value) -> std::string {
    return std::visit(*this, value);
}

auto Stringify::operator()(bool value) -> std::string {
    if (value)
        return "true";
    return "false";
}

auto Stringify::operator()(Callable *value) -> std::string {
    return value->toString();
}



auto Typify::operator()(std::nullptr_t) -> std::string {
    return "nil";
}

auto Typify::operator()(std::string value) -> std::string {
    return "string";
}

auto Typify::operator()(char value) -> std::string {
    return "character";
}

auto Typify::operator()(double value) -> std::string {
    return "double";
}

auto Typify::operator()(uint8_t value) -> std::string {
    return "unsigned byte";
}

auto Typify::operator()(int value) -> std::string {
    return "integer";
}

auto Typify::operator()(Number value) -> std::string {
    return std::visit(*this, value);
}

auto Typify::operator()(bool value) -> std::string {
    return "bool";
}

auto Typify::operator()(Callable *value) -> std::string {
    return "callable";
}
