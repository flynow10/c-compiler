//
// Created by Natalie Wagner on 4/18/26.
//

#ifndef C_COMPILER_ASMGEN_HPP
#define C_COMPILER_ASMGEN_HPP
#include <iosfwd>
#include <ostream>

#include "../parsing/ast.hpp"

class ASMGen {
protected:
    std::ostream& output;
public:
    explicit ASMGen(std::ostream& output) : output(output) {
    }

    virtual ~ASMGen() = default;
    virtual void gen_ast(AST::TranslationUnit *translation_unit) = 0;
};



#endif //C_COMPILER_ASMGEN_HPP
