//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef STATEMENT_HPP
#define STATEMENT_HPP
#include "types.hpp"

namespace IR {
    enum class InstructionType {
        IT_ARITH,
        IT_MOVE,
        IT_LOAD_LABEL,
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
    public:
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

    private:
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

    class MoveInst : public Instruction {
        const Register dest, source;

    public:
        MoveInst(Register dest, Register source) : Instruction(InstructionType::IT_MOVE), dest(dest), source(source) {
        }

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_MOVE;
        }
    };

    class LoadLabelInst : public Instruction {
        const Register dest;
        Label label = 0;

    public:
        LoadLabelInst(Register dest, Label label) : Instruction(InstructionType::IT_LOAD_LABEL), dest(dest), label(label) {};

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_LOAD_LABEL;
        }
    };

    class LoadImmInst : public Instruction {
        const Register dest;
        size_t value;
    public:
        LoadImmInst(Register dest, size_t value) : Instruction(InstructionType::IT_LOAD_IMM), dest(dest), value(value) {}

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_LOAD_IMM;
        }
    };

    class LoadInst : public Instruction {
        const Register dest, source;
        const MemOffset offset;

    public:
        LoadInst(const Register dest, const Register source, const MemOffset offset = 0) : Instruction(
            InstructionType::IT_LOAD), dest(dest), source(source), offset(offset) {
        }

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_LOAD;
        }
    };

    class StoreInst : public Instruction {
    public:
        StoreInst() : Instruction(InstructionType::IT_STORE) {
        }

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_STORE;
        }
    };

    class JumpInst : public Instruction {
    public:
        JumpInst() : Instruction(InstructionType::IT_JUMP) {
        }

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_JUMP;
        }
    };


    class BreakInst : public Instruction {
    public:
        BreakInst() : Instruction(InstructionType::IT_BREAK) {
        }

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_BREAK;
        }
    };

    class CallInst : public Instruction {
    public:
        CallInst() : Instruction(InstructionType::IT_CALL) {
        }

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_CALL;
        }
    };

    class ReturnInst : public Instruction {
    public:
        ReturnInst() : Instruction(InstructionType::IT_RETURN) {
        }

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_RETURN;
        }
    };
}


#endif //STATEMENT_HPP
