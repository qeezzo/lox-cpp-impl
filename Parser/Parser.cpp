#include <cmath>
#include <iostream>

#include "../Error/Error.h"
#include "Expr.h"
#include "Parser.h"
#include "Stmt.h"

using namespace lox;
using namespace Types;

extern bool hadError;

// ======================
// |   Public methods   |
// ======================

auto Parser::parse() -> std::vector<Stmt *> {
    std::vector<Stmt *> statements;

    while (!isAtEnd()) {
        auto decl = declaration();
        if (decl)
            statements.push_back(decl);
    }

    return statements;
}

// ======================
// |   Service methods  |
// ======================

auto Parser::push(Token token) -> void { stack.push(token); }
auto Parser::isAtEnd() -> bool { return peek().type() == LOX_EOF; }
auto Parser::peek() -> Token& { return tokens[current]; }
auto Parser::previous() -> Token& {
    if (current - 1 < 0)
        throw error(peek(), "");
    return tokens[current - 1];
}
auto Parser::top() -> Token& { return stack.top(); }
auto Parser::matchMemory(TokenType memory) -> bool {
    if (stack.empty())
        return false;
    if (!checkMemory(memory))
        return false;
    stack.pop();
    return true;
}
auto Parser::checkMemory(TokenType memory) -> bool {
    if (stack.empty())
        return false;
    return stack.top().type() == memory;
}
auto Parser::match(std::vector<TokenType> &&types) -> bool {
    for (auto type : types)
        if (check(type)) {
            advance();
            return true;
        }
    return false;
}
auto Parser::match(std::vector<TokenType> &&types, TokenType memory) -> bool {
    for (auto type : types)
        if (check(type, memory)) {
            advance();
            stack.pop();
            return true;
        }

    return false;
}
auto Parser::check(TokenType type) -> bool {
    if (isAtEnd())
        return false;
    return peek().type() == type;
}
auto Parser::check(TokenType type, TokenType memory) -> bool {
    if (isAtEnd() or stack.empty())
        return false;
    return peek().type() == type and stack.top().type() == memory;
}
auto Parser::advance() -> Token {
    if (!isAtEnd())
        ++current;
    return previous();
}
auto Parser::consume(TokenType type, std::string &&msg) -> Token {
    if (check(type))
        return advance();
    throw error(peek(), std::move(msg));
}
auto Parser::error(Token token, std::string &&msg) -> ParseError {
    if (token.type() == LOX_EOF)
        report(token.line(), "Parser", "at end. " + msg);
    else
        report(token.line(), "Parser", "at '" + token.lexeme() + "'. " + msg);

    throw ParseError("Parse error occured!");
}
auto Parser::synchronize() -> void {
    advance();

    while (!isAtEnd()) {
        if (previous().type() == SEMICOLON)
            return;

        switch (peek().type()) {
        case FUN:
        case VAR:
        case FOR:
        case IF:
        case WHILE:
        case PRINT:
        case RETURN:
            return;

        default:
            break;
        }

        advance();
    }
}
auto Parser::pop() -> Token {
    auto token = stack.top();
    stack.pop();
    return token;
}

// ======================
// |       RULES        |
// ======================

auto Parser::expression() -> Expr * {
    //LocalPush expr(this, previous());
    return assignment();
}
auto Parser::assignment() -> Expr * {
    auto expr = logic_or();

    if (check(EQUAL) and checkMemory(IDENTIFIER)) {
        advance();
        auto val = pop();
        auto value = assignment();
        return new AssignExpr(val, value);

    } else if (match({EQUAL}))
        error(previous(), "Invalid assignment target.");
    matchMemory(IDENTIFIER);

    return expr;
}
auto Parser::logic_or() -> Expr * {
    auto expr = logic_and();

    while (match({OR})) {
        Token op = previous();
        auto right = logic_and();

        expr = new LogicalExpr(expr, op, right);
    }

    return expr;
}
auto Parser::logic_and() -> Expr * {
    auto expr = equality();

    while (match({AND})) {
        Token op = previous();
        auto right = equality();

        expr = new LogicalExpr(expr, op, right);
    }

    return expr;
}
auto Parser::equality() -> Expr * {
    Expr *expr = comparison();

    while (match({BANG_EQUAL, EQUAL_EQUAL})) {
        Token op = previous();
        Expr *right = comparison();
        expr = new BinaryExpr(expr, op, right);
    }

    return expr;
}
auto Parser::comparison() -> Expr * {
    Expr *expr = shift();

    while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
        Token op = previous();
        Expr *right = term();
        expr = new BinaryExpr(expr, op, right);
    }

    return expr;
}
auto Parser::shift() -> Expr * {
    Expr *expr = term();

    while (match({SHIFT_LEFT, SHIFT_RIGHT})) {
        Token op = previous();
        Expr *right = term();
        expr = new BinaryExpr(expr, op, right);
    }

    return expr;
}
auto Parser::term() -> Expr * {
    Expr *expr = factor();

    while (match({MINUS, PLUS})) {
        Token op = previous();
        Expr *right = factor();
        expr = new BinaryExpr(expr, op, right);
    }

    return expr;
}
auto Parser::factor() -> Expr * {
    Expr *expr = unary();

    while (match({SLASH, STAR})) {
        Token op = previous();
        Expr *right = unary();
        expr = new BinaryExpr(expr, op, right);
    }

    return expr;
}
auto Parser::unary() -> Expr * {
    if (match({BANG, MINUS})) {
        Token op = previous();
        Expr *right = unary();
        return new UnaryExpr(op, right);
    }

    return call();
}
auto Parser::call() -> Expr * {
    auto expr = primary();

    while (true) {
        if (match({LEFT_PAREN})) {
            matchMemory(IDENTIFIER);

            expr = finishCall(expr);

        } else {
            break;
        }
    }

    return expr;
}
auto Parser::finishCall(Expr *callee) -> Expr * {
    std::vector<Expr *> arguments;

    if (!check(RIGHT_PAREN)) {
        do {
            arguments.push_back(expression());
        } while (match({COMMA}));
    }

    auto paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");

    return new CallExpr(callee, paren, std::move(arguments));
}
auto Parser::primary() -> Expr * {
    matchMemory(IDENTIFIER);

    if (match({NIL}))
        return new LiteralExpr(Literal(nullptr));
    if (match({FALSE}))
        return new LiteralExpr(Literal(false));
    if (match({TRUE}))
        return new LiteralExpr(Literal(true));

    if (match({NUMBER, STRING}))
        return new LiteralExpr(previous().literal());
    if (match({IDENTIFIER})) {
        push(previous());
        return new VariableExpr(previous());
    }

    if (match({LEFT_PAREN})) {
        Expr *expr = expression();
        consume(RIGHT_PAREN, "Expect ')' after expression");
        return new GroupingExpr(expr);
    }

    throw error(peek(), "Expect expression.");
}
auto Parser::declaration() -> Stmt * {
    try {

        if (match({VAR}))
            return varDeclStmt();
        if (match({FUN})) {
            LocalPush fun(this, previous());
            return funDeclStmt("function");
        }
        return statement();

    } catch (ParseError err) {
        synchronize();
        return nullptr;
    }
}
auto Parser::statement() -> Stmt * {
    if (match({PRINT}))
        return printStmt();
    if (match({LEFT_BRACE}))
        return blockStmt();
    if (match({IF}))
        return ifStmt();
    if (match({WHILE}))
        return whileStmt();
    if (match({FOR}))
        return forStmt();
    if (check(RETURN) and checkMemory(FUN)) {
        advance();
        return returnStmt();
    } else if (match({RETURN}))
        error(previous(), "Return statement is outside of function scope.");
    return exprStmt();
}
auto Parser::exprStmt() -> Stmt * {
    Expr *expr = expression();
    consume(SEMICOLON, "Expect ';' after expression.");

    return new ExpressionStmt(expr);
}
auto Parser::printStmt() -> Stmt * {
    Expr *expr = expression();
    consume(SEMICOLON, "Expect ';' after value.");

    return new PrintStmt(expr);
}
auto Parser::varDeclStmt() -> Stmt * {
    Token name = consume(IDENTIFIER, "Excpect variable name.");

    Expr *init = nullptr;
    if (match({EQUAL}))
        init = expression();

    consume(SEMICOLON, "Expect ';' after variable declaration.");
    return new VarStmt(name, init);
}
auto Parser::block() -> std::vector<Stmt *> {
    std::vector<Stmt *> statements;

    while (!check(RIGHT_BRACE) and !isAtEnd()) {
        auto decl = declaration();
        if (decl) statements.push_back(decl);
    }

    consume(RIGHT_BRACE, "Expect '}' after block.");
    return statements;
}
auto Parser::blockStmt() -> Stmt * {
    return new BlockStmt(block());
}
auto Parser::whileStmt() -> Stmt * {
    using namespace Types;
    consume(LEFT_PAREN, "Expect '(' after while.");
    Expr *condition = expression();
    consume(RIGHT_PAREN, "Expect ')' after while condition.");

    Stmt *body = statement();

    return new WhileStmt(condition, body);
}
auto Parser::forStmt() -> Stmt * {
    using namespace Types;
    consume(LEFT_PAREN, "Expect '(' after for.");

    Stmt *init;
    if (match({SEMICOLON}))
        init = nullptr;
    else if (match({VAR}))
        init = varDeclStmt();
    else
        init = exprStmt();

    Expr *condition = nullptr;
    if (!check(SEMICOLON))
        condition = expression();

    consume(SEMICOLON, "Expect ';' after loop condition.");

    Expr *increment = nullptr;
    if (!check(RIGHT_PAREN))
        increment = expression();

    consume(RIGHT_PAREN, "Expect ')' after for clauses.");

    Stmt *body = statement();

    if (increment)
        body = new BlockStmt({body, new ExpressionStmt(increment)});

    if (!condition)
        condition = new LiteralExpr(true);
    body = new WhileStmt(condition, body);

    if (init)
        body = new BlockStmt({init, body});

    return body;
}
auto Parser::ifStmt() -> Stmt * {
    using namespace Types;
    consume(LEFT_PAREN, "Expect '(' after if.");
    Expr *condition = expression();
    consume(RIGHT_PAREN, "Expect ')' after if condition.");

    Stmt *thenBranch = statement();

    Stmt *elseBranch = nullptr;
    if (match({ELSE}))
        elseBranch = statement();

    return new IfStmt(condition, thenBranch, elseBranch);
}
auto Parser::funDeclStmt(const std::string &kind) -> Stmt * {
    Token name = consume(IDENTIFIER, "Expect " + kind + " name.");
    consume(LEFT_PAREN, "Expect '(' after " + name.lexeme() + " name.");

    std::vector<Token> params;

    if (!check(RIGHT_PAREN)) {
        do {
            params.push_back(consume(IDENTIFIER, "Expect parameter name."));
        } while (match({COMMA}));
    }

    consume(RIGHT_PAREN, "Expect ')' after parameters.");
    consume(LEFT_BRACE, "Expect '{' before " + name.lexeme() + " body.");

    auto body = block();

    return new FunctionStmt(name, params, body);
}
auto Parser::returnStmt() -> Stmt * {
    Token keyword = previous();

    Expr *value{};
    if (!check(SEMICOLON))
        value = expression();

    consume(SEMICOLON, "Expect ';' after return value.");
    return new ReturnStmt(keyword, value);
}
