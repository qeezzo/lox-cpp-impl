#include "printer_ast.h"
#include "colors.h"
#include <memory>
using namespace lox::tools;


auto AstPrinter::print(Expr *expr) -> void {
    // out << std::string(nest_level, '\t');
    out << COLOR_EXPR;
    expr->accept(this);
    out << "\n" COLOR_STMT;
}

auto AstPrinter::print(Stmt *stmt) -> void {
    stmt->accept(this);
    out << COLOR_STMT;
}

auto AstPrinter::print(std::vector<Stmt *> statements) -> void {
    out << COLOR_STMT;
    for (auto statement : statements)
        print(statement);
    out << WHITE;
}

auto AstPrinter::print(const std::string &str) -> void {
    out << std::string(nest_level, '\t') << str;
}

auto AstPrinter::println(const std::string &str) -> void {
    out << std::string(nest_level, '\t') << str << std::endl;
}

void AstPrinter::parenthesize(const std::string &name,
                              const std::vector<Expr *> exprs) {
    out << "(" COLOR_OP << name;
    for (auto &expr : exprs) {
        out << " " COLOR_ARG;
        expr->accept(this);
        out << COLOR_EXPR;
    }
    out << ")";
}

auto AstPrinter::stringify(Lit &value) -> std::string {
    return std::visit(Types::Stringify(), value);
}

auto AstPrinter::visit(BinaryExpr *expr) -> Types::Literal {
    parenthesize(expr->op.lexeme(), {expr->left, expr->right});
    return nullptr;
}

auto AstPrinter::visit(GroupingExpr *expr) -> Types::Literal {
    parenthesize("group", {expr->expr});
    return nullptr;
}

auto AstPrinter::visit(LiteralExpr *expr) -> Types::Literal {
    out << COLOR_LITERAL "<" << stringify(expr->value) << ">" COLOR_EXPR;
    return nullptr;
}

auto AstPrinter::visit(UnaryExpr *expr) -> Types::Literal {
    parenthesize(expr->op.lexeme(), {expr->right});
    return nullptr;
}

auto AstPrinter::visit(LogicalExpr *expr) -> Lit {
    parenthesize(expr->op.lexeme(), {expr->left, expr->right});
    return nullptr;
}

auto AstPrinter::visit(VariableExpr *expr) -> Lit {
    out << expr->name.lexeme();
    return nullptr;
}

auto AstPrinter::visit(AssignExpr *expr) -> Lit {
    auto drop = std::make_unique<VariableExpr>(expr->name);
    parenthesize("=", {drop.get(), expr->value});
    return nullptr;
}

auto AstPrinter::visit(CallExpr *expr) -> Lit {
    std::vector<Expr *> arguments{expr->callee};
    arguments.insert(arguments.end(), expr->arguments.begin(),
                     expr->arguments.end());

    parenthesize("()", arguments);
    return nullptr;
}

// statements
auto AstPrinter::visit(ExpressionStmt *stmt) -> void {
    println("ExpressionStmt:");
    LocalNestLevel local_nest(nest_level);
    print("Expr: ");
    print(stmt->expr);
}

auto AstPrinter::visit(PrintStmt *stmt) -> void {
    println("PrintStmt:");
    LocalNestLevel local_nest(nest_level);
    print("Expr: ");
    print(stmt->expr);
}

auto AstPrinter::visit(VarStmt *stmt) -> void {
    println("VarStmt: ");
    LocalNestLevel local_nest(nest_level);
    println("VarName: " COLOR_INER + stmt->name.lexeme());
    if (stmt->init) {
        print(COLOR_STMT "InitExpr: ");
        print(stmt->init);
    }
}

auto AstPrinter::visit(BlockStmt *stmt) -> void {
    println("BlockStmt:");
    LocalNestLevel local_nest(nest_level);
    print(stmt->statements);
}

auto AstPrinter::visit(IfStmt *stmt) -> void {
    println("IfStmt:");
    LocalNestLevel local_nest(nest_level);
    print("Condition: ");
    print(stmt->condition);

    println("ThenBranch: ");
    {
        LocalNestLevel branch_nest(nest_level);
        print(stmt->thenBranch);
    }

    if (stmt->elseBranch) {
        println("ElseBranch: ");
        LocalNestLevel branch_nest(nest_level);
        print(stmt->elseBranch);
    }
}

auto AstPrinter::visit(WhileStmt *stmt) -> void {
    println("WhileStmt:");
    LocalNestLevel local_nest(nest_level);
    print("Condition: ");
    print(stmt->condition);
    println("Body: ");
    LocalNestLevel body_nest(nest_level);
    print(stmt->body);
}

auto AstPrinter::visit(FunctionStmt *stmt) -> void {
    println("FunctionStmt: " COLOR_INER + stmt->name.lexeme());
    LocalNestLevel local_nest(nest_level);

    std::string params{};
    for (auto &token : stmt->params)
        params += token.lexeme() + " ";
    println(COLOR_STMT "Parameters: " COLOR_INER + params);

    println(COLOR_STMT "Body: ");
    LocalNestLevel body_nest(nest_level);
    print(stmt->body);
}

auto AstPrinter::visit(ReturnStmt *stmt) -> void {
    println("ReturnStmt:");
    LocalNestLevel local_nest(nest_level);
    print("Expr: ");
    print(stmt->expr);
}
