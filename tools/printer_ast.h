#pragma once
#include <iostream>
#include <vector>

#include "../Parser/Expr.h"
#include "../Parser/Stmt.h"

namespace lox::tools {
class AstPrinter : public ExprVisitor, public StmtVisitor {
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

    auto parenthesize(const std::string &name, const std::vector<Expr *> exprs)
        -> void;

    auto stringify(Lit &lit) -> std::string;
    auto print(const std::string &name) -> void;
    auto println(const std::string &name) -> void;
    auto print(Stmt *stmt) -> void;
    auto print(Expr *expr) -> void;

  public:
    AstPrinter(std::ostream &out = std::clog) : out(out) {}

    auto print(std::vector<Stmt *> statements) -> void;

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
