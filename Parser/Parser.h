#pragma once
#include <cassert>
#include <stack>
#include <stdexcept>
#include <vector>

#include "../Types/Token.h"
#include "Expr.h"
#include "Stmt.h"

namespace lox {
// ======================
// |       RULES        |
// ======================
//
// expression     → assignment ;
// assignment     → IDENTIFIER "=" assignment | logic_or ;
// logic_or       → logic_and ("or" logic_and)* ;
// logic_and      → equality ("and" equality)* ;
// equality       → comparison ( ( "!=" | "==" ) comparison )* ;
// comparison     → shift ( ( ">" | ">=" | "<" | "<=" ) shift )* ;
// shift          → term ( ( ">>" | "<<" ) term )* ;
// term           → factor ( ( "-" | "+" ) factor )* ;
// factor         → unary ( ( "/" | "*" ) unary )* ;
// unary          → ( "!" | "-" ) unary | call ;
// call           → primary ( "(" arguments? ")" )* ;
// primary        → NUMBER | STRING | "true" | "false" | "nil"
//                         | IDENTIFIER | "(" expression ")" ;
//
// binaryExpr     → 
//
// arguments      → expression ( "," expression )* ;
//
// program        → declaration* EOF ;
// declaration    → funDecl | varDecl | statement ;
//
//      funDecl        → "fun" function ;
//      function       → IDENTIFIER "(" parameters? ")" block ;
//      parameters     → IDENTIFIER ( "," IDENTIFIER )* ;
//
// varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;
//
// statement      → exprStmt | printStmt | blockStmt |
//                  ifStmt | whileStmt | forStmt | returnStmt ;
// ifStmt         → "if" "(" expression ")" statement ("else" statemnt)? ;
// whileStmt      → "while" "(" expression ")" statement ;
// forStmt        → "for" "(" (varDecl | expression ";" | ";") expression? ";"
//                          expression? ")" statement ;
// blcokStmt      → '{' declaration '}'
// exprStmt       → expression ";" ;
// printStmt      → "print" expression ";" ;

class Parser {
  private:
    class ParseError : public std::runtime_error {
      public:
        explicit ParseError(const std::string &msg) : std::runtime_error(msg) {}
    };

    class LocalPush {
        const Types::Token token;
        Parser *parser;

      public:
        LocalPush(Parser *parser, const Types::Token token)
            : token(token), parser(parser) {
            parser->push(token);
        }

        ~LocalPush() { assert(parser->pop() == token); }
    };

    std::vector<Types::Token> &tokens;
    std::stack<Types::Token> stack;

    int current = 0;

  public:
    Parser(std::vector<Types::Token>& tokens) : tokens(tokens) { }

    auto parse() -> std::vector<Stmt *>;

  private:
    // Service methods
    auto match(std::vector<Types::TokenType> &&types) -> bool;
    auto match(std::vector<Types::TokenType> &&types, Types::TokenType memory)
        -> bool;
    auto matchMemory(Types::TokenType memory) -> bool;
    auto check(Types::TokenType type) -> bool;
    auto check(Types::TokenType type, Types::TokenType memory) -> bool;
    auto checkMemory(Types::TokenType memory) -> bool;
    auto advance() -> Types::Token;
    auto isAtEnd() -> bool;
    auto peek() -> Types::Token&;
    auto previous() -> Types::Token&;
    auto push(Types::Token token) -> void;
    auto pop() -> Types::Token;
    auto top() -> Types::Token&;

    // error handling
    auto consume(Types::TokenType type, std::string &&msg) -> Types::Token;
    auto error(Types::Token token, std::string &&msg) -> ParseError;
    auto synchronize() -> void;

    // Rules
    auto expression() -> Expr *;
    auto assignment() -> Expr *;
    auto logic_or() -> Expr *;
    auto logic_and() -> Expr *;
    auto equality() -> Expr *;
    auto comparison() -> Expr *;
    auto shift() -> Expr *;
    auto term() -> Expr *;
    auto factor() -> Expr *;
    auto unary() -> Expr *;
    auto call() -> Expr *;
    auto primary() -> Expr *;

    auto finishCall(Expr *callee) -> Expr *;

    auto declaration() -> Stmt *;
    auto statement() -> Stmt *;
    auto varDeclStmt() -> Stmt *;
    auto exprStmt() -> Stmt *;
    auto printStmt() -> Stmt *;
    auto blockStmt() -> Stmt *;
    auto whileStmt() -> Stmt *;
    auto forStmt() -> Stmt *;
    auto ifStmt() -> Stmt *;
    auto funDeclStmt(const std::string& kind) -> Stmt *;
    auto returnStmt() -> Stmt *;

    auto block() -> std::vector<Stmt *>;
};

} // namespace lox
