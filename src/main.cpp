#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "lexer.hpp"
#include "parsing/parse.hpp"
#include "sema/sema.hpp"

struct CompilerOptions {
    std::string inFile;
    std::string outFile;
};

int parse_argument(const std::string &argument, const int argIndex, const int argc,
                  char **argv, CompilerOptions &options) {
    if (argument == "-o") {
        if (argIndex + 1 >= argc) {
            throw std::invalid_argument("Missing output file");
        }
        options.outFile = argv[argIndex + 1];
        return 2;
    }

    options.inFile = argument;
    return 1;
}

int main(const int argc, char *argv[]) {
    CompilerOptions options;

    try {
        int i = 1;
        while (i < argc) {
            i += parse_argument(argv[i], i, argc, argv, options);
        }
    } catch (std::invalid_argument &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    if (options.inFile.empty()) {
        std::cout << "Must provide an input file" << std::endl;
        return 1;
    }

    std::ifstream inputFile(options.inFile);

    // Lexer lexer;
    // lexer.tokenize(inputFile);
    //
    // while (lexer.has_token()) {
    //   Token token = lexer.pop();
    //   std::cout << token.type() << " - \"" << token.value() << "\"" << std::endl;
    // }
    //
    // inputFile.seekg(0, std::ios::beg);

    Parser parser;

    const auto ast = parser.parse(inputFile);

    Sema sema;
    sema.accept_ast(ast.get());

    if (options.outFile.empty()) {
        std::cout << ast << std::endl;
    } else {
        std::ofstream outputFile(options.outFile, std::ofstream::out | std::ofstream::trunc);
        outputFile << ast;
        outputFile.close();
    }

    inputFile.close();

    return 0;
}
