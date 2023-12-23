#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <variant>

#include "Interpreter.h"
#include "../tools/colors.h"

using namespace lox;
using namespace Types;

extern bool hadRuntimeError;

static void runtimeError(RuntimeError &error) {
    std::cerr << "\e[31m";
    std::cerr << "\n[line " << error.token.line() << "] " << "Interprete Error: "
              << error.what() << std::endl;
    hadRuntimeError = true;
}


// Service function
auto Interpreter::convert_to_one_type(std::vector<Types::Number*> nums)
    -> void {

    if (nums.size() == 0) return;
    auto max_index = nums[0]->index();
    for (auto num : nums)
        if (max_index < num->index())
            max_index = num->index();

    for (auto& num : nums)
        switch (max_index){
            case 1:
                std::visit([&num](auto value) { *num = (int)value; }, *num);
                break;
            case 2:
                std::visit([&num](auto value) { *num = (double)value; }, *num);
                break;
            default: break;
        }
}
auto Interpreter::evaluate(Expr *expr) -> Lit { return expr->accept(this); }
auto Interpreter::executeFuncBlock(Env env, std::vector<Stmt *> &statements)
    -> void {
    auto saved_env = environment;
    auto back = [&saved_env](Env *env) { std::swap(*env, saved_env); };
    std::unique_ptr<Env, decltype(back)> backup(&environment, back);

    environment = env;
    environment->enclosing = globals.get();
    for (auto stmt : statements)
        execute(stmt);
}
auto Interpreter::executeBlock(Env env, std::vector<Stmt *> &statements)
    -> void {
    auto saved_env = environment;
    auto back = [&saved_env](Env *env) { std::swap(*env, saved_env); };
    std::unique_ptr<Env, decltype(back)> backup(&environment, back);

    environment = env;
    environment->enclosing = saved_env.get();
    for (auto stmt : statements)
        execute(stmt);
}
auto Interpreter::execute(Stmt *stmt) -> void { stmt->accept(this); }
auto Interpreter::isTruthy(Lit obj) -> bool {
    if (Types::isNil(obj))
        return false;
    if (std::holds_alternative<bool>(obj))
        return std::get<bool>(obj);
    return true;
}
auto Interpreter::isEqual(Lit lhs, Lit rhs) -> bool {
    // if (Types::isNil(lhs) and Types::isNil(rhs))
    //     return true;
    // if (!Types::isNil(lhs))
    //     return false;

    return lhs == rhs;
}
auto Interpreter::stringify(const Lit &value) -> std::string {
    return std::visit(Types::Stringify(), value);
}
auto Interpreter::operation(Types::Token op, double lhs, double rhs) -> Lit {
    using namespace Types;
    switch (op.type()) {
    case MINUS:
        return lhs - rhs;
    case SLASH:
        return lhs / rhs;
    case STAR:
        return lhs * rhs;
    case PLUS:
        return lhs + rhs;
    case GREATER:
        return lhs > rhs;
    case LESS:
        return lhs < rhs;
    case GREATER_EQUAL:
        return lhs >= rhs;
    case LESS_EQUAL:
        return lhs <= rhs;
    default:
        throw RuntimeError(op, "there is no operation '" + op.lexeme() +
                                   "' for doubles");
    }
}
auto Interpreter::operation(Types::Token op, int lhs, int rhs) -> Lit {
    using namespace Types;
    switch (op.type()) {
    case MINUS:
        return lhs - rhs;
    case SLASH:
        return lhs / rhs;
    case STAR:
        return lhs * rhs;
    case PLUS:
        return lhs + rhs;
    case GREATER:
        return lhs > rhs;
    case LESS:
        return lhs < rhs;
    case GREATER_EQUAL:
        return lhs >= rhs;
    case LESS_EQUAL:
        return lhs <= rhs;
    case SHIFT_LEFT:
        return lhs << rhs;
    case SHIFT_RIGHT:
        return lhs >> rhs;
    default:
        throw RuntimeError(op, "there is no operation '" + op.lexeme() +
                                   "' for integers");
    }
}
auto Interpreter::interprete(std::vector<Stmt *> statements) -> void {
    try {

        for (auto &statement : statements)
            execute(statement);

    } catch (RuntimeError &err) {
        runtimeError(err);
    }
}


// Statements
auto Interpreter::visit(ExpressionStmt *stmt) -> void { evaluate(stmt->expr); }
auto Interpreter::visit(PrintStmt *stmt) -> void {
    auto value = evaluate(stmt->expr);
    std::cout << WHITE << stringify(value) << std::endl;
}
auto Interpreter::visit(VarStmt *stmt) -> void {
    Lit value = nullptr;
    if (stmt->init != nullptr)
        value = evaluate(stmt->init);

    environment->define(stmt->name.lexeme(), value);
}
auto Interpreter::visit(BlockStmt *stmt) -> void {
    executeBlock(Env(new Environment()), stmt->statements);
}
auto Interpreter::visit(WhileStmt *stmt) -> void {
    while (isTruthy(evaluate(stmt->condition)))
        execute(stmt->body);
}
auto Interpreter::visit(IfStmt *stmt) -> void {
    if (isTruthy(evaluate(stmt->condition)))
        execute(stmt->thenBranch);
    else if (stmt->elseBranch)
        execute(stmt->elseBranch);
}
auto Interpreter::visit(FunctionStmt *stmt) -> void {
    // Function* function = new Function(stmt, new Environment(*environment));
    Function *function = new Function(stmt);
    environment->define(stmt->name.lexeme(), function);
}
auto Interpreter::visit(ReturnStmt *stmt) -> void {
    Lit value = nullptr;
    if (stmt->expr)
        value = evaluate(stmt->expr);

    throw Return(value);
}


// Expressions
auto Interpreter::visit(LiteralExpr *expr) -> Lit { return expr->value; }
auto Interpreter::visit(BinaryExpr *expr) -> Lit {
    using namespace Types;
    Lit left = evaluate(expr->left);
    Lit right = evaluate(expr->right);

    if (expr->op.type() == BANG_EQUAL)
        return !isEqual(left, right);
    if (expr->op.type() == EQUAL_EQUAL)
        return isEqual(left, right);

    Number *lhsNum = std::get_if<Number>(&left);
    Number *rhsNum = std::get_if<Number>(&right);

    // convert all to double if there're different
    if (lhsNum and rhsNum) {
        convert_to_one_type({lhsNum, rhsNum});
        {
            auto lhs = std::get_if<uint8_t>(lhsNum);
            auto rhs = std::get_if<uint8_t>(rhsNum);
            if (lhs and rhs)
                return operation(expr->op, *lhs, *rhs);
        }
        {
            auto lhs = std::get_if<int>(lhsNum);
            auto rhs = std::get_if<int>(rhsNum);
            if (lhs and rhs)
                return operation(expr->op, *lhs, *rhs);
        }
        {
            auto lhs = std::get_if<double>(lhsNum);
            auto rhs = std::get_if<double>(rhsNum);
            if (lhs and rhs)
                return operation(expr->op, *lhs, *rhs);
        }
    }

    if (lhsNum and not rhsNum)
        throw RuntimeError(expr->op, "Second operand should be number.");

    // operations with strings
    auto lhs = std::get_if<std::string>(&left);

    if (lhs == nullptr)
        throw RuntimeError(expr->op, "First operand should be number or string.");

    return *lhs + stringify(right);
}
auto Interpreter::visit(LogicalExpr *expr) -> Lit {
    Lit left = evaluate(expr->left);

    if (expr->op.type() == Types::OR) {
        if (isTruthy(left))
            return left;
    } else {
        if (!isTruthy(left))
            return left;
    }

    return evaluate(expr->right);
}
auto Interpreter::visit(GroupingExpr *expr) -> Lit {
    return evaluate(expr->expr);
}
auto Interpreter::visit(UnaryExpr *expr) -> Lit {
    using namespace Types;
    Lit right = evaluate(expr->right);

    Number* num;
    switch (expr->op.type()) {
    case BANG:
        return !isTruthy(right);
    case MINUS:
        num = std::get_if<Number>(&right);
        if (!num) throw RuntimeError(expr->op, "Unary operand for '-' should be number");
        std::visit([] (auto& value) { value = -value; }, *num);
        return *num;
    default:
        return nullptr;
    }
}
auto Interpreter::visit(VariableExpr *expr) -> Lit {
    return environment->get(expr->name);
}
auto Interpreter::visit(AssignExpr *expr) -> Lit {
    auto value = evaluate(expr->value);
    environment->assign(expr->name, value);
    return value;
}
auto Interpreter::visit(CallExpr *expr) -> Lit {
    auto callee = evaluate(expr->callee);

    std::vector<Lit> arguments;

    for (auto &argument : expr->arguments)
        arguments.push_back(evaluate(argument));

    if (!std::holds_alternative<Callable *>(callee))
        throw RuntimeError(expr->paren, "Can only call functions.");

    auto function = std::get<Callable *>(callee);

    if ((int)arguments.size() != function->arity())
        throw RuntimeError(expr->paren,
                           "Expect " + std::to_string(function->arity()) +
                               " arguments but got " +
                               std::to_string(arguments.size()) + ".");

    return function->call(this, expr->paren, arguments);
}
