//
// Created by Natalie Wagner on 2/17/26.
//

#include "ast.hpp"

#include <sstream>
#include <string>
#include <cxxabi.h>

std::string AST::to_string(const shared_ptr<Node> &node, int indent) {
    std::stringstream result;
    const auto spaces = std::string(indent, ' ');

    result << spaces << node->get_name();
    result << " " << std::hex << &*node << std::endl;

    for (const auto &child: *node) {
        result << AST::to_string(child, indent + 1);
    }
    return result.str();
}