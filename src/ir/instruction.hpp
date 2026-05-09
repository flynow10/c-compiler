//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef STATEMENT_HPP
#define STATEMENT_HPP
#include <string>
#include <utility>

#include "types.hpp"

namespace IR {
    enum class InstructionType {
        IT_ARITH,
        IT_UNARY,
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

        virtual ~Instruction() = default;

        [[nodiscard]] InstructionType get_type() const { return type; }

        [[nodiscard]] virtual std::string print() const { return ""; }

        friend std::ostream &operator<<(std::ostream &os, const Instruction &inst);
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
        ArithInst(Register dest, Register source1, Register source2,
                  const Operation op) : Instruction(InstructionType::IT_ARITH),
                                        dest(std::move(dest)),
                                        source1(std::move(source1)),
                                        source2(std::move(source2)), op(op) {
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_ARITH;
        }
    };

    class UnaryInst : public Instruction {
    public:
        enum class Operation {
            NEGATE,
            INVERT
        };
    private:
        Register dest, source;
        Operation op;
    public:
        UnaryInst(Register dest, Register source, const Operation op) : Instruction(InstructionType::IT_UNARY), dest(std::move(dest)), source(std::move(source)), op(op) {}

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_UNARY;
        }
    };

    class MoveInst : public Instruction {
        const Register dest, source;

    public:
        MoveInst(Register dest, Register source) : Instruction(InstructionType::IT_MOVE), dest(dest), source(source) {
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_MOVE;
        }
    };

    class LoadLabelInst : public Instruction {
        const Register dest;
        Label label = 0;

    public:
        LoadLabelInst(Register dest, Label label) : Instruction(InstructionType::IT_LOAD_LABEL), dest(dest),
                                                    label(label) {
        };

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_LOAD_LABEL;
        }
    };

    class LoadImmInst : public Instruction {
        const Register dest;
        size_t value;

    public:
        LoadImmInst(Register dest, size_t value) : Instruction(InstructionType::IT_LOAD_IMM), dest(dest), value(value) {
        }

        [[nodiscard]] std::string print() const override;

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

        [[nodiscard]] std::string print() const override;

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
        const Label label;
    public:
        explicit JumpInst(const Label label) : Instruction(InstructionType::IT_JUMP), label(label) {
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_JUMP;
        }
    };


    class BreakInst : public Instruction {
        const Register condition;
        const Label label;
    public:
        BreakInst(Register condition, const Label label) : Instruction(InstructionType::IT_BREAK), condition(std::move(condition)), label(label) {
        }

        [[nodiscard]] std::string print() const override;

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
        Register return_value;
    public:
        ReturnInst(Register returnValue) : Instruction(InstructionType::IT_RETURN), return_value(returnValue) {
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_RETURN;
        }
    };
}


#endif //STATEMENT_HPP
