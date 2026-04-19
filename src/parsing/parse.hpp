#ifndef PARSE_H
#define PARSE_H

#include "ast.hpp"
#include "../lexer.hpp"
#include <istream>
#include <memory>

using std::unique_ptr;

class Parser {
public:
  Parser();

  unique_ptr<AST::Node> parse(std::istream &stream);
private:
  Lexer lexer;
  bool is_type_def = false;

  unique_ptr<AST::TranslationUnit> parse_translation_unit();
  unique_ptr<AST::FunctionDecl> parse_function_decl();
  unique_ptr<AST::Decl> parse_decl();
  unique_ptr<AST::DeclSpecifiers> parse_decl_specifiers();
  unique_ptr<AST::StorageClass> parse_storage_class();
  unique_ptr<AST::TypeSpecifier> parse_type_specifier();
  unique_ptr<AST::StructSpecifier> parse_struct_specifier();
  unique_ptr<AST::StructDeclList> parse_struct_decl_list();
  unique_ptr<AST::StructDecl> parse_struct_decl();
  unique_ptr<AST::TypeQualifier> parse_type_qualifier();
  unique_ptr<AST::Declarator> parse_declarator(bool isAbstract = false);
  unique_ptr<AST::Pointer> parse_pointer();
  unique_ptr<AST::Node> parse_direct_declarator(bool isAbstract = false);
  unique_ptr<AST::IndexDeclarator> parse_index_declarator(unique_ptr<AST::Node> prevSuffix);
  unique_ptr<AST::ParameterizedDeclarator> parse_parameterized_declarator(unique_ptr<AST::Node> prevSuffix);
  unique_ptr<AST::ParameterList> parse_parameter_list();
  unique_ptr<AST::Parameter> parse_parameter();
  unique_ptr<AST::InitDeclarator> parse_init_declarator();
  unique_ptr<AST::TypeName> parse_type_name();
  unique_ptr<AST::InitializerList> parse_initializer_list();
  unique_ptr<AST::CompoundStatement> parse_compound_statement();
  unique_ptr<AST::Statement> parse_statement();
  unique_ptr<AST::SelectionStatement> parse_selection_statement();
  unique_ptr<AST::WhileStatement> parse_while_statement();
  unique_ptr<AST::DoStatement> parse_do_statement();
  unique_ptr<AST::ForStatement> parse_for_statement();
  unique_ptr<AST::ControlStatement> parse_control_statement();
  unique_ptr<AST::Expression> parse_expression();
  unique_ptr<AST::Expression> parse_expression_list(bool requireSurround);
  unique_ptr<AST::Expression> parse_assignment_expression();
  unique_ptr<AST::Expression> parse_logical_or_expression();
  unique_ptr<AST::Expression> parse_logical_and_expression();
  unique_ptr<AST::Expression> parse_inclusive_or_expression();
  unique_ptr<AST::Expression> parse_exclusive_or_expression();
  unique_ptr<AST::Expression> parse_and_expression();
  unique_ptr<AST::Expression> parse_equality_expression();
  unique_ptr<AST::Expression> parse_relation_expression();
  unique_ptr<AST::Expression> parse_shift_expression();
  unique_ptr<AST::Expression> parse_additive_expression();
  unique_ptr<AST::Expression> parse_multiplicative_expression();
  unique_ptr<AST::Expression> parse_cast_expression();
  unique_ptr<AST::Expression> parse_unary_expression();
  unique_ptr<AST::Expression> parse_postfix_expression();
  unique_ptr<AST::Expression> parse_primary_expression();

};
#endif // !PARSE_H
