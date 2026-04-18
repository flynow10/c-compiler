//
// Created by Natalie Wagner on 4/17/26.
//

#ifndef C_COMPILER_EXECUTE_HPP
#define C_COMPILER_EXECUTE_HPP

#include <string>
#include <vector>
// Launch a new process without terminate this process.
// It combines fork + exec system-calls.
void fork_exec(std::string app, std::vector<std::string> args);

#endif //C_COMPILER_EXECUTE_HPP
