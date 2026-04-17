#ifndef PARSE_H
#define PARSE_H

#include "ast.hpp"
#include "lexer.hpp"
#include <istream>
#include <memory>

using std::shared_ptr;

class Parser {
public:
  Parser();

  shared_ptr<AST::Node> parse(std::istream &stream);
private:
  Lexer lexer;
  bool is_type_def = false;

  shared_ptr<AST::TranslationUnit> parse_translation_unit();
  shared_ptr<AST::FunctionDecl> parse_function_decl();
  shared_ptr<AST::Decl> parse_decl();
  shared_ptr<AST::DeclSpecifiers> parse_decl_specifiers();
  shared_ptr<AST::StorageClass> parse_storage_class();
  shared_ptr<AST::TypeSpecifier> parse_type_specifier();
  shared_ptr<AST::StructSpecifier> parse_struct_specifier();
  shared_ptr<AST::StructDeclList> parse_struct_decl_list();
  shared_ptr<AST::StructDecl> parse_struct_decl();
  shared_ptr<AST::TypeQualifier> parse_type_qualifier();
  shared_ptr<AST::Declarator> parse_declarator(bool isAbstract = false);
  shared_ptr<AST::Pointer> parse_pointer();
  shared_ptr<AST::Node> parse_direct_declarator(bool isAbstract = false);
  shared_ptr<AST::IndexDeclarator> parse_index_declarator();
  shared_ptr<AST::ParameterizedDeclarator> parse_parameterized_declarator();
  shared_ptr<AST::ParameterList> parse_parameter_list();
  shared_ptr<AST::Parameter> parse_parameter();
  shared_ptr<AST::InitDeclarator> parse_init_declarator();
  shared_ptr<AST::TypeName> parse_type_name();
  shared_ptr<AST::InitializerList> parse_initializer_list();
  shared_ptr<AST::CompoundStatement> parse_compound_statement();
  shared_ptr<AST::Statement> parse_statement();
  shared_ptr<AST::SelectionStatement> parse_selection_statement();
  shared_ptr<AST::WhileStatement> parse_while_statement();
  shared_ptr<AST::DoStatement> parse_do_statement();
  shared_ptr<AST::ForStatement> parse_for_statement();
  shared_ptr<AST::ControlStatement> parse_control_statement();
  shared_ptr<AST::Expression> parse_expression();
  shared_ptr<AST::Expression> parse_expression_list(bool requireSurround);
  shared_ptr<AST::Expression> parse_assignment_expression();
  shared_ptr<AST::Expression> parse_logical_or_expression();
  shared_ptr<AST::Expression> parse_logical_and_expression();
  shared_ptr<AST::Expression> parse_inclusive_or_expression();
  shared_ptr<AST::Expression> parse_exclusive_or_expression();
  shared_ptr<AST::Expression> parse_and_expression();
  shared_ptr<AST::Expression> parse_equality_expression();
  shared_ptr<AST::Expression> parse_relation_expression();
  shared_ptr<AST::Expression> parse_shift_expression();
  shared_ptr<AST::Expression> parse_additive_expression();
  shared_ptr<AST::Expression> parse_multiplicative_expression();
  shared_ptr<AST::Expression> parse_cast_expression();
  shared_ptr<AST::Expression> parse_unary_expression();
  shared_ptr<AST::Expression> parse_postfix_expression();
  shared_ptr<AST::Expression> parse_primary_expression();

};
#endif // !PARSE_H
