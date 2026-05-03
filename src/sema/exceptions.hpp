//
// Created by Natalie Wagner on 5/3/26.
//

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP
#include <stdexcept>

#include "../parsing/ast.hpp"
namespace SemaAnalysis {
    class ExprException : public std::runtime_error {
        std::string exceptionMessage;
    public:
        explicit ExprException(const AST::Expression *expression, const std::string &error_message);
        [[nodiscard]] const char * what() const noexcept override;
    };

    void print_exception(const std::exception &e, int level = 0);
}

#endif //EXCEPTIONS_HPP
