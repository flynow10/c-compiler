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
  shared_ptr<AST::StorageClass> parse_storage_class();
  shared_ptr<AST::TypeSpecifier> parse_type_specifier();
  shared_ptr<AST::StructSpecifier> parse_struct_specifier();
  shared_ptr<AST::EnumSpecifier> parse_enum_specifier();
  shared_ptr<AST::TypeQualifier> parse_type_qualifier();
  shared_ptr<AST::InitDeclarator> parse_init_declarator();
  shared_ptr<AST::Declarator> parse_declarator();
  shared_ptr<AST::CompoundStatement> parse_compound_statement();
  shared_ptr<AST::Statement> parse_statement();
  shared_ptr<AST::SelectionStatement> parse_selection_statement();
  shared_ptr<AST::WhileStatement> parse_while_statement();
  shared_ptr<AST::DoStatement> parse_do_statement();
  shared_ptr<AST::ForStatement> parse_for_statement();
  shared_ptr<AST::ControlStatement> parse_control_statement();
  shared_ptr<AST::Expression> parse_expression();
  shared_ptr<AST::ExpressionList> parse_expression_list();

};
#endif // !PARSE_H
