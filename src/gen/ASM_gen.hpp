//
// Created by Natalie Wagner on 4/18/26.
//

#ifndef C_COMPILER_ASMGEN_HPP
#define C_COMPILER_ASMGEN_HPP
#include <iosfwd>
#include <ostream>

class ASM_gen {
protected:
    std::ostream& output;
public:
    explicit ASM_gen(std::ostream& output) : output(output) {
    }

    virtual ~ASM_gen() = default;
};



#endif //C_COMPILER_ASMGEN_HPP
