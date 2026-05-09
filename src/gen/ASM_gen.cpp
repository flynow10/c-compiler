//
// Created by Natalie Wagner on 4/18/26.
//

#include "ASM_gen.hpp"

void ASMGen::gen(const IR::IRContext &ir) {
    gen_preamble();
    for (int i = 0; i < ir.get_num_functions(); ++i) {
        gen_function(ir, ir.get_function(i));
    }
    output << std::endl;
}

void ASMGen::add_label(const std::string &label) {
    output << label << ":\n";
}

void ASMGen::add_instruction(const std::string &instruction) {
    output << "\t" << instruction << "\n";
}
