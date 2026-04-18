//
// Created by Natalie Wagner on 4/17/26.
//
#include <filesystem>
#include <fstream>
#include <iostream>
#include "execute.hpp"

namespace fs = std::filesystem;

struct TestOptions {
    std::string inputDir;
    std::string outputDir;
    std::string compilerPath;
};


int parse_argument(const std::string &argument, const int argIndex, const int argc,
                   char **argv, TestOptions &options) {
    if (argument == "-o") {
        if (argIndex + 1 >= argc) {
            throw std::invalid_argument("Missing output directory");
        }
        options.outputDir = argv[argIndex + 1];
        return 2;
    }

    if (argument == "-i") {
        if (argIndex + 1 >= argc) {
            throw std::invalid_argument("Missing input directory");
        }
        options.inputDir = argv[argIndex + 1];
        return 2;
    }

    if (argument == "-c") {
        if (argIndex + 1 >= argc) {
            throw std::invalid_argument("Missing compiler path");
        }
        options.compilerPath = argv[argIndex + 1];
        return 2;
    }
    throw std::invalid_argument("Unknown option");
}

void compile_file(const fs::path &file_path, const TestOptions &options) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file " + file_path.string());
    }
    file.close();
    fs::path outputFile = fs::canonical(options.outputDir);
    outputFile.append(file_path.stem().string());

    std::vector<std::string> args = {
        fs::canonical(file_path).string(),
        "-o",
        outputFile.string(),
    };

    fs::path compiler = fs::canonical(options.compilerPath);
    std::cout << compiler;
    for (const auto & arg: args) {
        std::cout << " " << arg;
    }
    std::cout << std::endl;

    fork_exec(compiler.string(), args);
}

int main(int argc, char *argv[]) {
    TestOptions options;
    options.inputDir = "./test/test-code";
    options.outputDir = "./test/test-out";
    options.compilerPath = "./cmake-build-debug/c_compiler";
    try {
        int i = 1;
        while (i < argc) {
            i += parse_argument(argv[i], i, argc, argv, options);
        }
    } catch (std::invalid_argument &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    if (options.inputDir.empty() || options.outputDir.empty() || options.compilerPath.empty()) {
        throw std::invalid_argument("Missing required option");
    }

    fs::path inputDir = options.inputDir;

    std::vector<fs::path> testFiles;

    for (const auto &entry: fs::directory_iterator(inputDir)) {
        if (auto &path = entry.path(); path.extension() == ".c") {
            testFiles.push_back(path);
        }
    }

    for (const auto &entry: testFiles) {
        compile_file(entry, options);
    }
}
