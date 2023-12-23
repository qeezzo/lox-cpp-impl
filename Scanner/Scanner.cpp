#include <cctype>
#include <iostream>
#include <vector>

#include "Scanner.h"

using namespace lox;

std::unordered_map<std::string, Types::TokenType> Scanner::keywords{
    {"and", Types::AND},     /*{"class", Types::CLASS},*/   {"else", Types::ELSE},
    {"false", Types::FALSE}, {"fun", Types::FUN},       {"for", Types::FOR},
    {"if", Types::IF},       {"nil", Types::NIL},       {"or", Types::OR},
    {"print", Types::PRINT}, {"return", Types::RETURN}, /*{"super", Types::SUPER},*/
    /*{"this", Types::THIS},*/   {"true", Types::TRUE},     {"var", Types::VAR},
    {"while", Types::WHILE}};

void Scanner::addToken(Types::TokenType type) {
    std::string lexeme = source.substr(start, current - start);
    tokens.push_back({type, std::move(lexeme), line, start, current - start});
}

void Scanner::addToken(Types::TokenType type, Types::Literal value) {
    std::string lexeme = source.substr(start, current - start);
    tokens.push_back(
        {type, std::move(lexeme), value, line, start, current - start});
}

bool Scanner::match(char expected) {
    if (isAtEnd())
        return false;
    if (source[current] != expected)
        return false;

    current++;
    return true;
}

char Scanner::peekNext() {
    if ((size_t)current + 1 >= source.length())
        return '\0';
    return source[current + 1];
}

void Scanner::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (isAtEnd()) {
        report(line, "Scanner", "Unterminated string.");
        return;
    }

    // The closing "
    advance();

    // Trim the quotes
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(Types::STRING, value);
}

void Scanner::number() {
    while (std::isdigit(peek()))
        advance();

    if (peek() == '.' && std::isdigit(peekNext())) {
        advance();
        while (std::isdigit(peek()))
            advance();

        Types::Number value = std::stod(source.substr(start, current - start));
        addToken(Types::NUMBER, value);
        return;
    }

    Types::Number value = std::stoi(source.substr(start, current - start));
    addToken(Types::NUMBER, value);
}

auto Scanner::integer_format() -> void {
    size_t store{};
    Types::Number value = (int) 0;
    if (peek() == 'x' and
        (std::isdigit(peekNext()) or
         ('a' <= tolower(peekNext()) and tolower(peekNext()) <= 'f'))) {
        advance();
        while (std::isdigit(peek()) or
               ('a' <= tolower(peek()) and tolower(peek()) <= 'f'))
            advance();

        value = std::stoi(source.substr(start, current - start), &store, 16);
    } else if (peek() == 'b' and (peekNext() == '0' or peekNext() == '1')) {
        advance();

        int count = 0;
        while (peek() == '0' or peek() == '1')
            advance(), count++;

        if (count > 8)
            report(line, "Scanner", "Max 8 bits, got " + std::to_string(count));

        value = (uint8_t) std::stoi(source.substr(start, current - start), &store, 2);
    } else if (peek() == '.') {
        number();
        return;
    }

    addToken(Types::NUMBER, value);
}

void Scanner::identifier() {
    while (std::isalnum(peek()) or peek() == '_')
        advance();

    auto type = keywords.find(source.substr(start, current - start));
    if (type != keywords.end()) {
        addToken(type->second);
        return;
    }
    addToken(Types::IDENTIFIER);
}

void Scanner::character() {
    while (peek() != '\'' and not isAtEnd()) {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (isAtEnd()) {
        report(line, "Scanner", "Unterminated single quotes.");
        return;
    }

    // closing '
    advance();
    if (current - start - 2 != 1) {
        report(line, "Scanner", "Wrong size of char.");
        return;
    }

    char value = source.substr(start + 1, current - start - 2)[0];
    addToken(Types::STRING, value);
}

void Scanner::scanToken() {
    using namespace Types;
    char ch = advance();
    switch (ch) {
    // single character tokens
    case '(': addToken(LEFT_PAREN); break;
    case ')': addToken(RIGHT_PAREN); break;
    case '{': addToken(LEFT_BRACE); break;
    case '}': addToken(RIGHT_BRACE); break;
    case ',': addToken(COMMA); break;
    //case '.': addToken(DOT); break;
    case '-': addToken(MINUS); break;
    case '+': addToken(PLUS); break;
    case ';': addToken(SEMICOLON); break;
    case '*': addToken(STAR); break;

    // single or double character tokens
    case '!': addToken(match('=') ? BANG_EQUAL : BANG); break;
    case '=': addToken(match('=') ? EQUAL_EQUAL : EQUAL); break;
    case '<': addToken(match('=') ? LESS_EQUAL : match('<') ? SHIFT_LEFT : LESS); break;
    case '>': addToken(match('=') ? GREATER_EQUAL
                     : match('>') ? SHIFT_RIGHT
                                  : GREATER);
        break;

    // division or comments
    case '/':
        if (match('/'))
            while (peek() != '\n' && !isAtEnd())
                advance();
        else if (match('*'))
            while (peek() != '*' && peekNext() != '/' && !isAtEnd()) {
                if (peek() == '\n') ++line;
                advance();
            }
        else
            addToken(SLASH);
        break;

    // skip white spaces
    case '\n':
        ++line;
    case ' ':
    case '\r':
    case '\t':
        break;

    // string literal
    case '"':
        string();
        break;

    case '\'':
        character();
        break;

    case '0':
        integer_format();
        break;

    default:
        // number literal
        if (std::isdigit(ch))
            number();
        // identifier
        else if (std::isalpha(ch) or ch == '_')
            identifier();
        else
            report(line, "Scanner",
                   "Unexpected character '" + std::string(1, ch) + "'.");
    }
}

std::vector<Types::Token>&& Scanner::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }

    tokens.push_back({Types::LOX_EOF, "", line, start, 0});

    return std::move(tokens);
}
