#pragma once
#include <boost/flyweight/flyweight.hpp>
#include <string>
#include <optional>
#include <sys/types.h>
#include <variant>
#include <map>
#include <vector>
#include <cstdint>
#include <boost/flyweight.hpp>

namespace lox
{
class Interpreter;

    namespace Types {

    class Callable;

    enum TokenType
    {
        // Single-character tokens.
        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
        COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

        // One or two character tokens.
        BANG, BANG_EQUAL,
        EQUAL, EQUAL_EQUAL,
        GREATER, GREATER_EQUAL,
        LESS, LESS_EQUAL,
        SHIFT_LEFT, SHIFT_RIGHT,

        // Literals.
        IDENTIFIER, STRING, NUMBER,

        // Keywords.
        AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
        PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

        LOX_EOF
    };

    // priority is matter. double has hieghest priority
    typedef std::variant<uint8_t, int, double> Number;
    typedef std::variant<Callable*, std::string, char, Number, bool, std::nullptr_t> Literal;

    const std::string& TokenTypeString(const TokenType value);
    auto isNil(Literal& lit) -> bool;

    class Token;
    class Callable
    {
    public:
        virtual auto arity() -> int = 0;
        virtual auto call(Interpreter* interpreter, Token& token, std::vector<Literal> arguments) -> Literal = 0;
        virtual auto toString() -> std::string = 0;
    };

    class Stringify
    {
    public:
        auto operator()(std::nullptr_t) -> std::string;
        auto operator()(std::string value) -> std::string;
        auto operator()(char value) -> std::string;
        auto operator()(double value) -> std::string;
        auto operator()(int value) -> std::string;
        auto operator()(uint8_t value) -> std::string;
        auto operator()(Number value) -> std::string;
        auto operator()(bool value) -> std::string;
        auto operator()(Callable* value) -> std::string;
    };

    class Typify
    {
    public:
        auto operator()(std::nullptr_t) -> std::string;
        auto operator()(std::string value) -> std::string;
        auto operator()(char value) -> std::string;
        auto operator()(double value) -> std::string;
        auto operator()(int value) -> std::string;
        auto operator()(uint8_t value) -> std::string;
        auto operator()(Number value) -> std::string;
        auto operator()(bool value) -> std::string;
        auto operator()(Callable* value) -> std::string;
    };

    class Token
    {
    private:
        const TokenType _type;
        const std::string _lexeme;

        //OptionalLiteral _literal = std::nullopt;
        //Literal _literal = nullptr;
        boost::flyweights::flyweight<Literal> _literal;

        const int _line = -1;
        const int _offset = -1;
        const int _length = -1;
    public:

        Token(TokenType type, std::string lexeme, Literal literal,
                int line, int offset, int length) :
            _type(type), _lexeme(lexeme), _literal(literal),
            _line(line), _offset(offset), _length(length)
        { }

        Token(TokenType type, std::string lexeme,
                int line, int offset, int length) :
            _type(type), _lexeme(lexeme), _literal(nullptr),
            _line(line), _offset(offset), _length(length)
        { }

        Token(TokenType type, std::string lexeme)
            : _type(type), _lexeme(lexeme), _literal(nullptr)
        { }

        auto type()      const { return _type; }
        auto lexeme()    const { return _lexeme; }
        auto literal()   const { return _literal; }

        auto line()      const { return _line; }
        auto offset()    const { return _offset; }
        auto length()    const { return _length; }

        std::string toString() const
        {
            return TokenTypeString(_type) + " " + _lexeme + " "
                + std::visit(Stringify(), _literal.get());
                //+ std::visit(Stringify(), _literal);
        }

        friend bool operator == (const Token& lhs, const Token& rhs) = default;

    };  // class Token


    }   // namespace Types


}   // namespace lox
