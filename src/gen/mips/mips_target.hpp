//
// Created by Natalie Wagner on 4/18/26.
//

#ifndef C_COMPILER_MIPSTARGET_HPP
#define C_COMPILER_MIPSTARGET_HPP
#include "../ASM_gen.hpp"


class mips_target : ASM_gen {
public:
    explicit mips_target(std::ostream &output)
        : ASM_gen(output) {
    }
};



#endif //C_COMPILER_MIPSTARGET_HPP
