#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "Checker/Checker.h"
#include "Interpreter/Interpreter.h"
#include "Parser/Parser.h"
#include "Scanner/Scanner.h"
#include "Types/Token.h"
#include "tools/printer_ast.h"
#include "tools/printer_identifiers.h"
#include "tools/colors.h"

using namespace lox;

bool hadRuntimeError = false;
bool hadError = false;


class Config {

    void ast() { print_ast = true; }
    void help() { print_help = true; }
    void id_table() { print_id_table = true; }
    void lex_table() { print_lex_table = true; }

    std::unordered_map<std::string, void (Config::*)()> keys {
        {"--ast", &Config::ast},
        {"-a", &Config::ast},
        {"--help", &Config::help},
        {"-h", &Config::help},
        {"--id-table", &Config::id_table},
        {"-i", &Config::id_table},
        {"--lex-table", &Config::lex_table},
        {"-l", &Config::lex_table},
    };

  public:

    std::string file;

    bool print_ast = false;
    bool print_help = false;
    bool print_id_table = false;
    bool print_lex_table = false;

    bool prompt = true;
    bool interprete = true;

    Config(int argc, char* argv[]) {
        if (argc == 1) return;

        for (int i = 1; i < argc; ++i) {
            char* arg = argv[i];

            if (arg[0] != '-') {
                if (not file.empty()) {
                    std::cerr << RED "encounter several src files; should be only one" << std::endl;
                    std::exit(64);
                }
                file = arg;
                prompt = false;
                continue;
            }

            if (not keys.contains(arg)) {
                std::cerr << RED "unknown argument: " << arg << std::endl;
                std::exit(64);
            }
            (this->*keys[arg])();
            interprete = false;
        }
    }

};

std::once_flag flag_id_print_natives;
void run(std::string &&input, Config& config) {
    Scanner scanner(std::move(input));
    std::vector<Types::Token> tokens = scanner.scanTokens();

    if (config.print_lex_table) {
        int line = 0;
        for (auto& token : tokens) {
            while (line != token.line()) {
                line++;
                std::cout << "\n[" << line << "]\t";
            }
            std::cout << Types::TokenTypeString(token.type()) << " ";
        }
        std::cout << std::endl;
    }

    Parser parser(tokens);
    auto stmts = parser.parse();

    static Checker checker;
    checker.check(stmts);

    if (hadError)
        return;

    if (config.print_ast) {
        tools::AstPrinter printer(std::cout);
        printer.print(stmts);
    }

    if (config.print_id_table) {
        static tools::IdPrinter printer(std::cout);
        std::call_once(flag_id_print_natives,
                &tools::IdPrinter::print_natives, &printer);
        printer.print(stmts);
    }

    if (config.interprete) {
        static Interpreter interpreter;
        interpreter.interprete(stmts);
    }
}

void runFile(Config& config) {
    std::ifstream file(config.file);

    std::stringstream buf;
    buf << file.rdbuf();

    std::string str = buf.str();
    // delete trailing newline
    str.pop_back();
    run(std::move(str), config);

    if (hadRuntimeError)
        std::exit(70);
    if (hadError)
        std::exit(64);
}

void runPrompt(Config& config) {
    std::cout << "\e[33m>> " GREEN;
    for (;;) {
        std::string input;
        std::string str;

        int brace_count = 0;
        do {
            std::getline(std::cin, input);
            for (auto ch : input) {
                if (ch == '{') brace_count++;
                if (ch == '}') brace_count--;
            }
            str += input;
            if (brace_count > 0) 
                std::cout << "\e[33m... " GREEN;
        } while (brace_count > 0);

        std::cout << "\e[0m";
        if (str.empty())
            break;

        run(std::move(str), config);
        hadRuntimeError = false;
        hadError = false;
        std::cout << "\n\e[33m>> " GREEN;
    }
}

void print_help();
int main(int argc, char *argv[]) {
    Config config(argc, argv);

    if (config.print_help)
        print_help();
    else if (config.file.empty())
        runPrompt(config);
    else
        runFile(config);
}

void print_help() {
    std::cout << "Usage: zrv [keys] [script]\n";
    std::cout << "Any flag will turn interpretation off. Can be combine together.\n";
    std::cout << "Avaible keys:\n";
    std::cout << "\t-h\t--help\t\tprints this message\n";
    std::cout << "\t-a\t--ast\t\tprints abstract syntax tree\n";
    std::cout << "\t-i\t--id-table\tprints table of identifiers\n";
    std::cout << "\t-l\t--lex-table\tprints table of lexemes types\n";
}
