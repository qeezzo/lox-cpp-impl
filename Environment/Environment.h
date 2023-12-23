#pragma once
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <memory>

#include "../Types/Token.h"

namespace lox
{

class RuntimeError : public std::runtime_error
{
public:
    RuntimeError(Types::Token op, std::string msg):
        std::runtime_error(msg),
        token(op)
    {}
    Types::Token token;
};


class Environment
{
    typedef Types::Literal Lit;
    std::unordered_map<std::string, Lit> values;

public:

    Environment* enclosing = nullptr;

    Environment() = default;
    Environment(Environment* enclosing) : enclosing(enclosing) {}

    auto define(const std::string& name, Lit value) -> void
    {
        values[name] = value;
    }

    auto assign(const Types::Token& name, Lit value) -> void
    {
        if (values.contains(name.lexeme())) {
            values[name.lexeme()] = value;
            return;
        }

        if (enclosing) {
            enclosing->assign(name, value);
            return;
        }

        throw RuntimeError(name, "Undefined variable '" + name.lexeme() + "'.");
    }

    auto get(Types::Token& name) -> Lit
    {
        if (values.contains(name.lexeme()))
            return values[name.lexeme()];

        if (enclosing) return enclosing->get(name);

        throw RuntimeError(name, "Undefined variable '" + name.lexeme() + "'.");
    }

    auto check_local(Types::Token& name) -> bool
    {
        return values.contains(name.lexeme());
    }

    auto check(Types::Token& name) -> bool
    {
        if (values.contains(name.lexeme()))
            return true;
        if (enclosing)
            return enclosing->check(name);
        return false;
    }

}; // class Environment

typedef std::shared_ptr<Environment> Env;

} // namespace lox
