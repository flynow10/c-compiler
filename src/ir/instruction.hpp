//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef STATEMENT_HPP
#define STATEMENT_HPP
#include "types.hpp"

namespace IR {
    enum class InstructionType {
        IT_ARITH,
        IT_LOAD_IMM,
        IT_LOAD,
        IT_STORE,
        IT_JUMP,
        IT_BREAK,
        IT_CALL,
        IT_RETURN,
    };

    class Instruction {
        const InstructionType type;

    public:
        explicit Instruction(InstructionType type) : type(type) {
        }

        InstructionType get_type() const { return type; }
    };

    class ArithInst : public Instruction {
        enum class Operation {
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            MODULO,
            OR,
            AND,
            XOR,
        };

        Register dest, source1, source2;
        Operation op;

    public:
        ArithInst(const Register dest, const Register source1, const Register source2,
                  const Operation op) : Instruction(InstructionType::IT_ARITH),
                                        dest(dest),
                                        source1(source1),
                                        source2(source2), op(op) {
        }

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_ARITH;
        }
    };
}


#endif //STATEMENT_HPP
