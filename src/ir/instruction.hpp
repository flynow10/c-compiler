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
        IT_COMPARE,
        IT_UNARY,
        IT_MOVE,
        IT_LOAD_LABEL,
        IT_LOAD_IMM,
        IT_LOAD,
        IT_STORE,
        IT_JUMP,
        IT_BREAK,
        IT_CALL,
        IT_CALL_PTR,
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
            SHIFT_LEFT,
            SHIFT_RIGHT,
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

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] const Register &get_source1() const {
            return source1;
        }

        [[nodiscard]] const Register &get_source2() const {
            return source2;
        }

        [[nodiscard]] Operation get_operation() const {
            return op;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_ARITH;
        }
    };

    class CompareInst : public Instruction {
    public:
        enum class Operation {
            EQUAL,
            NOT_EQUAL,
            LESS_THAN,
            LESS_THAN_EQUAL,
            GREATER_THAN,
            GREATER_THAN_EQUAL,
            LESS_THAN_UNSIGNED,
            LESS_THAN_EQUAL_UNSIGNED,
            GREATER_THAN_UNSIGNED,
            GREATER_THAN_EQUAL_UNSIGNED,
        };

    private:
        const Register dest, source1, source2;
        const Operation op;

    public:
        CompareInst(Register dest, Register source1, Register source2, Operation op) : Instruction(
                InstructionType::IT_COMPARE), dest(std::move(dest)), source1(std::move(source1)),
            source2(std::move(source2)), op(op) {
        }

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] const Register &get_source1() const {
            return source1;
        }

        [[nodiscard]] const Register &get_source2() const {
            return source2;
        }

        [[nodiscard]] Operation get_operation() const {
            return op;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_COMPARE;
        }
    };

    class UnaryInst : public Instruction {
    public:
        enum class Operation {
            NEGATE,
            INVERT,
        };

    private:
        Register dest, source;
        Operation op;

    public:
        UnaryInst(Register dest, Register source, const Operation op) : Instruction(InstructionType::IT_UNARY),
                                                                        dest(std::move(dest)),
                                                                        source(std::move(source)), op(op) {
        }

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

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] const Register &get_source() const {
            return source;
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

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] size_t get_value() const {
            return value;
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

        [[nodiscard]] Label get_label() const {
            return label;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_JUMP;
        }
    };


    class BranchInst : public Instruction {
        const Register condition;
        const Label label;

    public:
        BranchInst(Register condition, const Label label) : Instruction(InstructionType::IT_BREAK),
                                                            condition(std::move(condition)), label(label) {
        }

        [[nodiscard]] const Register &get_condition() const {
            return condition;
        }

        [[nodiscard]] const Label &get_label() const {
            return label;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_BREAK;
        }
    };

    class CallInst : public Instruction {
        const Register dest;
        const std::string identifier;
        const std::vector<Register> args;

    public:
        CallInst(Register dest, std::string identifier, std::vector<Register> args) : Instruction(
                InstructionType::IT_CALL), dest(std::move(dest)),
            identifier(std::move(identifier)),
            args(std::move(args)) {
        }

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] const std::string &get_identifier() const {
            return identifier;
        }

        [[nodiscard]] const std::vector<Register> &get_args() const {
            return args;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_CALL;
        }
    };

    class CallPtrInst : public Instruction {
        const Register dest;
        const Register func_ptr;
        const std::vector<Register> args;

    public:
        CallPtrInst(Register dest, Register funcPtr, std::vector<Register> args) : Instruction(
                InstructionType::IT_CALL_PTR), dest(std::move(dest)),
            func_ptr(std::move(funcPtr)),
            args(std::move(args)) {
        }

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] const Register &get_func_ptr() const {
            return func_ptr;
        }

        [[nodiscard]] const std::vector<Register> &get_args() const {
            return args;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_CALL_PTR;
        }
    };

    class ReturnInst : public Instruction {
        Register return_value;

    public:
        ReturnInst(Register returnValue) : Instruction(InstructionType::IT_RETURN),
                                           return_value(std::move(returnValue)) {
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_RETURN;
        }
    };
}


#endif //STATEMENT_HPP
