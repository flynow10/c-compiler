//
// Created by Natalie Wagner on 2/17/26.
//

#include "ast.hpp"

#include <sstream>
#include <string>

std::string AST::to_string(const Node *node, const int indent) {
    std::stringstream result;
    const auto spaces = std::string(indent, '|');

    result << spaces << node->get_name();
    result << " " << std::hex << &*node << std::endl;

    for (const auto &child: *node) {
        if (child == nullptr) continue;
        result << to_string(child.get(), indent + 1);
    }
    return result.str();
}