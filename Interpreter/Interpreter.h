#pragma once
#include <ctime>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <variant>
#include <vector>

#include "../Environment/Environment.h"
#include "../Parser/Expr.h"
#include "../Parser/Stmt.h"
#include "../Types/Token.h"

namespace lox {

class Return : public std::runtime_error {
  public:
    Types::Literal value;
    Return(Types::Literal value) : std::runtime_error(""), value(value) {}
};

class ClockCallable : public Types::Callable {
  public:
    auto arity() -> int override { return 0; }
    auto call(Interpreter *interpreter, Types::Token &token,
              std::vector<Types::Literal> arguments) -> Types::Literal override;
    auto toString() -> std::string override { return "<fun native>"; }
};

class PowCallable : public Types::Callable {
  public:
    auto arity() -> int override { return 2; }
    auto call(Interpreter *interpreter, Types::Token &token,
              std::vector<Types::Literal> arguments) -> Types::Literal override;
    auto toString() -> std::string override { return "<fun native>"; }
};

class Log2Callable : public Types::Callable {
  public:
    auto arity() -> int override { return 1; }
    auto call(Interpreter *interpreter, Types::Token &token,
              std::vector<Types::Literal> arguments) -> Types::Literal override;
    auto toString() -> std::string override { return "<fun native>"; }
};

class PRNCallable : public Types::Callable {
  public:
    auto arity() -> int override { return 1; }
    auto call(Interpreter *interpreter, Types::Token &token,
              std::vector<Types::Literal> arguments) -> Types::Literal override;
    auto toString() -> std::string override { return "<fun native>"; }
};

class TypeCallable : public Types::Callable {
  public:
    auto arity() -> int override { return 1; }
    auto call(Interpreter *interpreter, Types::Token &token,
              std::vector<Types::Literal> arguments) -> Types::Literal override;
    auto toString() -> std::string override { return "<fun native>"; }
};

class Function : public Types::Callable {
  private:
    FunctionStmt *declaration;
    // Env closure;
  public:
    // Function(FunctionStmt* declaration, Environment* closure):
    //     declaration(declaration), closure(closure)
    //{}

    Function(FunctionStmt *declaration) : declaration(declaration) {}

    auto arity() -> int override { return declaration->params.size(); }
    auto toString() -> std::string override {
        return "<fun " + declaration->name.lexeme() + ">";
    }

    auto call(Interpreter *interpreter, Types::Token &token,
              std::vector<Types::Literal> arguments) -> Types::Literal override;
};

class RuntimeError;

class Interpreter : public ExprVisitor, public StmtVisitor {
  public:
    typedef Types::Literal Lit;
    Env globals = std::make_shared<Environment>();

  private:
    Env environment = globals;

    auto evaluate(Expr *expr) -> Lit;
    auto isTruthy(Lit obj) -> bool;
    auto isEqual(Lit lhs, Lit rhs) -> bool;
    auto execute(Stmt *stmt) -> void;
    auto stringify(const Lit &lit) -> std::string;
    auto convert_to_one_type(std::vector<Types::Number*> nums) -> void;

    auto operation(Types::Token op, int lhs, int rhs) -> Lit;
    auto operation(Types::Token op, double lhs, double rhs) -> Lit;

  public:
    Interpreter() {
        globals->define("clock", new ClockCallable());
        globals->define("pow", new PowCallable());
        globals->define("log2", new Log2Callable());
        globals->define("prn", new PRNCallable());
        globals->define("type", new TypeCallable());
    }

    auto interprete(std::vector<Stmt *> stmts) -> void;
    auto executeBlock(Env env, std::vector<Stmt *> &statements) -> void;
    auto executeFuncBlock(Env env, std::vector<Stmt *> &statements) -> void;

    // Expressions
    auto visit(BinaryExpr *expr) -> Lit override;
    auto visit(LogicalExpr *expr) -> Lit override;
    auto visit(GroupingExpr *expr) -> Lit override;
    auto visit(LiteralExpr *expr) -> Lit override;
    auto visit(UnaryExpr *expr) -> Lit override;
    auto visit(VariableExpr *expr) -> Lit override;
    auto visit(AssignExpr *expr) -> Lit override;
    auto visit(CallExpr *expr) -> Lit override;

    // Statements
    auto visit(ExpressionStmt *stmt) -> void override;
    auto visit(PrintStmt *stmt) -> void override;
    auto visit(VarStmt *stmt) -> void override;
    auto visit(BlockStmt *stmt) -> void override;
    auto visit(WhileStmt *stmt) -> void override;
    auto visit(IfStmt *stmt) -> void override;
    auto visit(FunctionStmt *stmt) -> void override;
    auto visit(ReturnStmt *stmt) -> void override;

}; // class Interpreter

} // namespace lox
