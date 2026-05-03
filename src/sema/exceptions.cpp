//
// Created by Natalie Wagner on 5/3/26.
//

#include "exceptions.hpp"

#include <iostream>
#include <sstream>

SemaAnalysis::ExprException::ExprException(const AST::Expression *expression, const std::string &error_message): runtime_error(error_message) {
    std::stringstream s;
    s << "Exception parsing expression of type " << expression->get_name() << std::endl;
    s << runtime_error::what();
    exceptionMessage = s.str();
}

const char *SemaAnalysis::ExprException::what() const noexcept {
    return exceptionMessage.c_str();
}

void SemaAnalysis::print_exception(const std::exception &e, int level) {
    std::cerr << std::string(level,  ' ') << "sema: " << e.what() << std::endl;
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception &nestedException) {
        print_exception(nestedException, level + 1);
    } catch (...) {}
}