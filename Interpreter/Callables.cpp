#include "Interpreter.h"
#include <cctype>
#include <cmath>
#include <stack>
#include <unordered_map>
#include <string>
#include <variant>

using namespace lox;
using namespace lox::Types;

auto polish_notation(const std::string& expr) -> std::string;

auto Function::call(Interpreter *interpreter, Token &token,
                    std::vector<Types::Literal> arguments) -> Types::Literal {
    Env env(new Environment());

    for (size_t i{}; i < declaration->params.size(); ++i)
        env->define(declaration->params[i].lexeme(), arguments[i]);

    try {
        interpreter->executeFuncBlock(env, declaration->body);
    } catch (Return &value) {
        return value.value;
    }
    return nullptr;
}

auto PowCallable::call(Interpreter* interpreter, Token& token, std::vector<Types::Literal> arguments) -> Types::Literal
{
    auto num = std::get_if<Types::Number>(&arguments[0]);
    auto power = std::get_if<Types::Number>(&arguments[1]);

    if (!num and !power) throw RuntimeError(token, "args should be numbers");

    return std::visit([] (auto base, auto exp) {
            return std::pow(base, exp);
    }, *num, *power);
}

auto ClockCallable::call(Interpreter* interpreter, Types::Token& token, std::vector<Types::Literal> arguments) -> Types::Literal
{
    return (int) time(NULL);
}

auto Log2Callable::call(Interpreter* interpreter, Types::Token& token, std::vector<Types::Literal> arguments) -> Types::Literal 
{
    auto num = std::get_if<Types::Number>(&arguments[0]);

    if (!num) throw RuntimeError(token, "argument is required to be a number");

    return std::visit([] (auto value) {
            return std::log2(value);
    }, *num);
}

auto PRNCallable::call(Interpreter* interpreter, Types::Token& token, std::vector<Types::Literal> arguments) -> Types::Literal 
{
    auto expr = std::get_if<std::string>(&arguments[0]);

    if (!expr) throw RuntimeError(token, "argument is required to be a string");

    return polish_notation(*expr);
}

auto TypeCallable::call(Interpreter* interpreter, Types::Token& token, std::vector<Types::Literal> arguments) -> Types::Literal 
{
    return std::visit(Types::Typify(), arguments[0]);
}

auto polish_notation(const std::string& expr) -> std::string {
    using namespace std;
    string result;
    stack<char> op_stack;
    std::unordered_map<char, int> priority {
        {'(', 0}, {')', 0}, {',', 1},
        {'[', 1}, {']', 1},
        {'+', 2}, {'-', 2},
        {'*', 3}, {'/', 3},
    };

    int count_arguments = 0;
    bool flag_args = false;
    bool arg = false;
    for (auto ch : expr) {

        if (isalnum(ch)) {
            result.push_back(ch), arg = true;
            continue;
        }
        if (arg) {
            result.push_back(' '), arg = false;
            if (flag_args and count_arguments == 0)
                count_arguments = 1;
        }

        if (ch == ' ') continue;

        if (ch == '(') op_stack.push(ch);
        else if (ch == '[') flag_args = true, op_stack.push(ch);
        else if (ch == ')') {
            while (not op_stack.empty() and op_stack.top() != '(')
                result.push_back(op_stack.top()), op_stack.pop();
            if (not op_stack.empty() and op_stack.top() == '(')
                op_stack.pop();
        }
        else if (ch == ']') {
            while (not op_stack.empty() and op_stack.top() != '[')
                result.push_back(op_stack.top()), op_stack.pop();
            if (not op_stack.empty() and op_stack.top() == '[')
                op_stack.pop();
            result.push_back('@');
            result.append(to_string(count_arguments) + ' ');
            count_arguments = 0;
            flag_args = false;
        }
        else if (ch == ',') {
            count_arguments++;
            while (not op_stack.empty() and op_stack.top() != '[')
                result.push_back(op_stack.top()), op_stack.pop();
        }
        else if (priority.contains(ch)) {
            if (op_stack.empty()) { op_stack.push(ch); continue; }

            while (not op_stack.empty() and priority[ch] <= priority[op_stack.top()])
                result.push_back(op_stack.top()), op_stack.pop();
            op_stack.push(ch);
        }
    }

    while (!op_stack.empty())
        result.push_back(op_stack.top()), op_stack.pop();

    return result;
}

