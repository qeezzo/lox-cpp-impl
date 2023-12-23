#pragma once
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../Parser/Expr.h"
#include "../Parser/Stmt.h"
#include "../Interpreter/Interpreter.h"

namespace lox::tools {
class IdPrinter : public ExprVisitor, public StmtVisitor {
  private:
    class LocalNestLevel {
        int &nest;

      public:
        LocalNestLevel(int &nest) : nest(++nest) {}
        ~LocalNestLevel() { --nest; }
    };

    typedef Types::Literal Lit;

    std::ostream &out;
    int nest_level{};

    std::unordered_set<Lit> set_of_literals;
    std::vector<std::pair<std::string, Lit>> natives;

    auto stringify(Lit &lit) -> std::string;
    auto print(const std::string &name) -> void;
    auto println(const std::string& name, Lit value) -> void;
    auto print(Stmt *stmt) -> void;
    auto print(Expr *expr) -> void;


  public:
    IdPrinter(std::ostream &out = std::clog) : out(out) {
        natives.push_back({"clock", new ClockCallable()});
        natives.push_back({"pow", new PowCallable()});
        natives.push_back({"log2", new Log2Callable()});
        natives.push_back({"prn", new PRNCallable()});
        natives.push_back({"type", new TypeCallable()});
    }

    auto print(std::vector<Stmt *> statements) -> void;
    auto print(std::vector<Expr *> statements) -> void;
    auto print_natives() -> void;

    auto visit(BinaryExpr *expr) -> Lit override;
    auto visit(LogicalExpr *expr) -> Lit override;
    auto visit(GroupingExpr *expr) -> Lit override;
    auto visit(LiteralExpr *expr) -> Lit override;
    auto visit(UnaryExpr *expr) -> Lit override;
    auto visit(VariableExpr *expr) -> Lit override;
    auto visit(AssignExpr *expr) -> Lit override;
    auto visit(CallExpr *expr) -> Lit override;

    auto visit(ExpressionStmt *stmt) -> void override;
    auto visit(PrintStmt *stmt) -> void override;
    auto visit(VarStmt *stmt) -> void override;
    auto visit(BlockStmt *stmt) -> void override;
    auto visit(IfStmt *stmt) -> void override;
    auto visit(WhileStmt *stmt) -> void override;
    auto visit(FunctionStmt *stmt) -> void override;
    auto visit(ReturnStmt *stmt) -> void override;
};

} // namespace lox::tools
