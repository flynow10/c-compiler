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

const std::string & AST::Declarator::get_identifier() const {
    if (isAbstract) {
        throw std::runtime_error("Cannot get identifier of abstract declarator");
    }
    Node * directDecl = get_direct_declarator();
    if (isa<Declarator>(directDecl)) {
        return cast<Declarator>(directDecl)->get_identifier();
    }
    return cast<DirectDeclarator>(directDecl)->get_identifier();
}

bool AST::FunctionDecl::_has_parameter_list() const {
    return _get_parameter_list() != nullptr;
}

AST::ParameterList * AST::FunctionDecl::_get_parameter_list() const {
    auto *decl = get_declarator();
    auto *parameterizedDeclarator = cast<ParameterizedDeclarator>(decl->get_suffix());
    return parameterizedDeclarator->get_parameter_list();
}
