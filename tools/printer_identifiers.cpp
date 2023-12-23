#include "printer_identifiers.h"
#include <format>

using namespace lox::tools;

auto IdPrinter::print(Expr *expr) -> void {
    expr->accept(this);
}

auto IdPrinter::print(Stmt *stmt) -> void {
    stmt->accept(this);
}

auto IdPrinter::print_natives() -> void {
    out << std::format(" /{:^13}/{:^13}/{:^15}/", "Name", "Type", "Value") << "\n";
    for (auto& [name, value] : natives)
        out << std::vformat("|{:^13}|{:^13}|{:^15}|", std::make_format_args(
                name, std::visit(Types::Typify(), value),
                stringify(value))) << "\n";
}

auto IdPrinter::print(std::vector<Stmt *> statements) -> void {
    for (auto statement : statements)
        print(statement);
}

auto IdPrinter::print(std::vector<Expr *> statements) -> void {
    for (auto statement : statements)
        print(statement);
}

auto IdPrinter::print(const std::string &str) -> void {
    out << std::string(nest_level, '\t') << str;
}

auto IdPrinter::println(const std::string& name, Lit value) -> void {
    //out << std::string(nest_level * 4, '-');
    out << std::vformat("|{:^13}|{:^13}|{:^15}|", std::make_format_args(
            name, std::visit(Types::Typify(), value),
            stringify(value))) << "\n";
}

auto IdPrinter::stringify(Lit &value) -> std::string {
    return std::visit(Types::Stringify(), value);
}


// statements
auto IdPrinter::visit(ExpressionStmt *stmt) -> void {
    print(stmt->expr);
}

auto IdPrinter::visit(PrintStmt *stmt) -> void {
    print(stmt->expr);
}

auto IdPrinter::visit(VarStmt *stmt) -> void {
    println(stmt->name.lexeme(), stmt->name.literal());
    if (stmt->init)
        print(stmt->init);
}

auto IdPrinter::visit(BlockStmt *stmt) -> void {
    LocalNestLevel local_nest(nest_level);
    for (auto statement : stmt->statements)
        print(statement);
}

auto IdPrinter::visit(IfStmt *stmt) -> void {
    print(stmt->condition);

    {
        LocalNestLevel branch_nest(nest_level);
        print(stmt->thenBranch);
    }

    if (stmt->elseBranch) {
        LocalNestLevel branch_nest(nest_level);
        print(stmt->elseBranch);
    }
}

auto IdPrinter::visit(WhileStmt *stmt) -> void {
    print(stmt->condition);
    LocalNestLevel body_nest(nest_level);
    print(stmt->body);
}

auto IdPrinter::visit(FunctionStmt *stmt) -> void {
    Function* function = new Function(stmt);
    println(stmt->name.lexeme(), function);

    LocalNestLevel local_nest(nest_level);
    for (auto& param : stmt->params)
        println(param.lexeme(), param.literal());

    for (auto statement : stmt->body)
        print(statement);
}

auto IdPrinter::visit(ReturnStmt *stmt) -> void {
    print(stmt->expr);
}


// expressions
auto IdPrinter::visit(BinaryExpr *expr) -> Types::Literal {
    print(expr->left);
    print(expr->right);
    return nullptr;
}

auto IdPrinter::visit(GroupingExpr *expr) -> Types::Literal {
    print(expr->expr);
    return nullptr;
}

auto IdPrinter::visit(LiteralExpr *expr) -> Types::Literal {
    if (set_of_literals.contains(expr->value))
        return nullptr;
    println("<anonymous>", expr->value);
    set_of_literals.insert(expr->value);
    return nullptr;
}

auto IdPrinter::visit(UnaryExpr *expr) -> Types::Literal {
    print(expr->right);
    return nullptr;
}

auto IdPrinter::visit(LogicalExpr *expr) -> Lit {
    print(expr->left);
    print(expr->right);
    return nullptr;
}

auto IdPrinter::visit(VariableExpr *expr) -> Lit {
    return nullptr;
}

auto IdPrinter::visit(AssignExpr *expr) -> Lit {
    auto drop = std::make_unique<VariableExpr>(expr->name);
    print(expr->value);
    print(drop.get());
    return nullptr;
}

auto IdPrinter::visit(CallExpr *expr) -> Lit {
    std::vector<Expr *> arguments{expr->callee};
    arguments.insert(arguments.end(), expr->arguments.begin(),
                     expr->arguments.end());
    print(arguments);
    return nullptr;
}
