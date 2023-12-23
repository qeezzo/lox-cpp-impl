#include <cmath>
#include <iostream>

#include "Checker.h"
#include "../Error/Error.h"

extern bool hadError;
extern bool hadRuntimeError;

auto Checker::check(std::vector<Stmt*> stmts) -> void
{
    for (auto& statement : stmts)
        consider(statement);
}

auto Checker::consider(Stmt* statement) -> void
{
    statement->accept(this);
}

auto Checker::consider(Expr* expr) -> void
{
    expr->accept(this);
}

auto Checker::check_declaration(Types::Token name) -> void
{
    if (!environment->check(name))
        error(name.line(), "'" + name.lexeme() + "' wasn't declared.");
}

auto Checker::check_duplication(Types::Token name) -> void
{
    if (environment->check_local(name))
        error(name.line(), "Duplication of '" + name.lexeme() + "'.");
}

auto Checker::error(int line, std::string msg) -> void
{
    report(line, "Checker", msg);
}

// Statements
auto Checker::visit(ExpressionStmt* stmt) -> void
{
    consider(stmt->expr);
}

auto Checker::visit(PrintStmt* stmt) -> void
{
    consider(stmt->expr);
}

auto Checker::visit(VarStmt* stmt) -> void
{
    if (stmt->init)
        consider(stmt->init);
    check_duplication(stmt->name);

    environment->define(stmt->name.lexeme(), nullptr);
}

auto Checker::visit(BlockStmt* stmt) -> void
{
    LocalEnvironment local(this);
    for (auto statement : stmt->statements)
        consider(statement);
}

auto Checker::visit(WhileStmt* stmt) -> void
{
    consider(stmt->condition);
    consider(stmt->body);
}

auto Checker::visit(IfStmt* stmt) -> void
{
    consider(stmt->condition);
    consider(stmt->thenBranch);
    if (stmt->elseBranch)
        consider(stmt->elseBranch);
}

auto Checker::visit(FunctionStmt* stmt) -> void
{
    check_duplication(stmt->name);

    //Function* function = new Function(stmt, new Environment(*environment));
    Function* function = new Function(stmt);
    environment->define(stmt->name.lexeme(), function);

    // Function scope
    auto saved_env = environment;
    auto back = [&saved_env](Env *env) { std::swap(*env, saved_env); };
    std::unique_ptr<Env, decltype(back)> backup(&environment, back);

    Env env = std::make_shared<Environment>();
    environment = env;
    environment->enclosing = globals.get();


    for (auto decl : stmt->params)
        environment->define(decl.lexeme(), nullptr);

    check(stmt->body);
}

auto Checker::visit(ReturnStmt* stmt) -> void
{
    consider(stmt->expr);
}



// Expressions
auto Checker::visit(BinaryExpr* expr) -> Lit
{
    consider(expr->left);
    consider(expr->right);
    return nullptr;
}

auto Checker::visit(LogicalExpr* expr) -> Lit
{
    consider(expr->left);
    consider(expr->right);
    return nullptr;
}

auto Checker::visit(GroupingExpr* expr) -> Lit
{
    consider(expr->expr);
    return nullptr;
}

auto Checker::visit(LiteralExpr* expr) -> Lit
{
    return nullptr;
}

auto Checker::visit(UnaryExpr* expr) -> Lit
{
    consider(expr->right);
    return nullptr;
}

auto Checker::visit(VariableExpr* expr) -> Lit
{
    check_declaration(expr->name);
    return nullptr;
}

auto Checker::visit(AssignExpr* expr) -> Lit
{
    check_declaration(expr->name);

    consider(expr->value);
    return nullptr;
}

auto Checker::visit(CallExpr* expr) -> Lit
{
    consider(expr->callee);
    for (auto arg : expr->arguments)
        consider(arg);
    return nullptr;
}
