#pragma once

#include "../Environment/Environment.h"
#include "../Interpreter/Interpreter.h"
#include "../Parser/Expr.h"
#include "../Parser/Stmt.h"
#include "../Types/Token.h"

namespace lox {

class Checker : public ExprVisitor, public StmtVisitor {
    class LocalEnvironment {
        Checker *checker;
        Env saved_env;

      public:
        LocalEnvironment(Checker *checker) : checker(checker) {
            saved_env = checker->environment;
            Env env = std::make_shared<Environment>();

            checker->environment = env;
            checker->environment->enclosing = saved_env.get();
        }

        ~LocalEnvironment() { checker->environment = saved_env; }
    };

  public:
    typedef Types::Literal Lit;
    Env globals = std::make_shared<Environment>();

  private:
    Env environment = globals;

    auto consider(Stmt *statement) -> void;
    auto consider(Expr *expr) -> void;
    auto error(int line, std::string msg) -> void;
    auto check_duplication(Types::Token token) -> void;
    auto check_declaration(Types::Token token) -> void;

  public:
    Checker() {
        globals->define("clock", new ClockCallable());
        globals->define("pow", new PowCallable());
        globals->define("log2", new Log2Callable());
        globals->define("prn", new PRNCallable());
        globals->define("type", new TypeCallable());
    }

    auto check(std::vector<Stmt *> stmts) -> void;

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
};

} // namespace lox
