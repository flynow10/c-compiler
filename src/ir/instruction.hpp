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
        IT_ALLOCATE,
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
        Instruction(const Instruction &inst) = default;

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
        const Register dest;
        const std::unique_ptr<InstArgument> source1, source2;
        const Operation op;

    public:
        ArithInst(Register dest, std::unique_ptr<InstArgument> source1, std::unique_ptr<InstArgument> source2,
                  const Operation op) : Instruction(InstructionType::IT_ARITH),
                                        dest(std::move(dest)),
                                        source1(std::move(source1)),
                                        source2(std::move(source2)), op(op) {
        }

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] const InstArgument *get_source1() const {
            return source1.get();
        }

        [[nodiscard]] const InstArgument *get_source2() const {
            return source2.get();
        }

        [[nodiscard]] Operation get_operation() const {
            return op;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_ARITH;
        }

        static std::unique_ptr<ArithInst> create(Register dest, std::unique_ptr<InstArgument> source1, std::unique_ptr<InstArgument> source2,
                  const Operation op) {
            return std::make_unique<ArithInst>(std::move(dest), std::move(source1), std::move(source2), op);
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
        const Register dest;
        std::unique_ptr<InstArgument> source1, source2;
        const Operation op;

    public:
        CompareInst(Register dest, std::unique_ptr<InstArgument> source1, std::unique_ptr<InstArgument> source2, Operation op) : Instruction(
                InstructionType::IT_COMPARE), dest(std::move(dest)), source1(std::move(source1)),
            source2(std::move(source2)), op(op) {
        }

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] InstArgument *get_source1() const {
            return source1.get();
        }

        [[nodiscard]] InstArgument *get_source2() const {
            return source2.get();
        }

        [[nodiscard]] Operation get_operation() const {
            return op;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_COMPARE;
        }

        static std::unique_ptr<CompareInst> create(Register dest, std::unique_ptr<InstArgument> source1, std::unique_ptr<InstArgument> source2, Operation op) {
            return std::make_unique<CompareInst>(std::move(dest), std::move(source1), std::move(source2), op);
        }
    };

    class UnaryInst : public Instruction {
    public:
        enum class Operation {
            NEGATE,
            INVERT,
        };

    private:
        const Register dest;
        const std::unique_ptr<InstArgument> source;
        const Operation op;

    public:
        UnaryInst(Register dest, std::unique_ptr<InstArgument>source, const Operation op) : Instruction(InstructionType::IT_UNARY),
                                                                        dest(std::move(dest)),
                                                                        source(std::move(source)), op(op) {
        }

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] InstArgument *get_source() const {
            return source.get();
        }

        [[nodiscard]] Operation get_operation() const {
            return op;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_UNARY;
        }

        static std::unique_ptr<UnaryInst> create(Register dest, std::unique_ptr<InstArgument> source, Operation op) {
            return std::make_unique<UnaryInst>(std::move(dest), std::move(source), op);
        }
    };

    /**
     * Allocates space on the stack for the destination register.
     * The destination register must be of type pointer
     * @param size the number of bytes to allocate
     */
    class AllocateInst : public Instruction {
        const Register dest;
        const size_t size;

    public:
        AllocateInst(Register dest, size_t size) : Instruction(InstructionType::IT_ALLOCATE), dest(std::move(dest)), size(size) {}

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] const size_t &get_size() const {
            return size;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_ALLOCATE;
        }

        static std::unique_ptr<AllocateInst> create(Register dest, size_t size) {
            return std::make_unique<AllocateInst>(std::move(dest), size);
        }
    };

    /**
     * Loads from memory the value stored in the source ptr.
     * @param source must be a register of type ptr or a global
     */
    class LoadInst : public Instruction {
        const Register dest;
        const std::unique_ptr<InstArgument> source;

    public:
        LoadInst(Register dest, std::unique_ptr<InstArgument> source) : Instruction(
            InstructionType::IT_LOAD), dest(std::move(dest)), source(std::move(source)) {
        }

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] InstArgument *get_source() const {
            return source.get();
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_LOAD;
        }

        static std::unique_ptr<LoadInst> create(Register dest, std::unique_ptr<InstArgument> source) {
            return std::make_unique<LoadInst>(std::move(dest), std::move(source));
        }
    };

    /**
     * Stores in memory the value specified by source
     * @param dest must be of type ptr
     * @param source is either a register or an immediate
     */
    class StoreInst : public Instruction {
        const Register dest;
        const std::unique_ptr<InstArgument> source;
    public:
        StoreInst(Register dest, std::unique_ptr<InstArgument> source) : Instruction(InstructionType::IT_STORE), dest(std::move(dest)), source(std::move(source)) {
        }

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] InstArgument *get_source() const {
            return source.get();
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_STORE;
        }

        static std::unique_ptr<StoreInst> create(Register dest, std::unique_ptr<InstArgument> source) {
            return std::make_unique<StoreInst>(std::move(dest), std::move(source));
        }
    };

    /**
     * Jumps to the given label or pointer
     * @param jump_point must be either a label or a register of type ptr
     */
    class JumpInst : public Instruction {
        const std::unique_ptr<InstArgument> jump_point;

    public:
        explicit JumpInst(std::unique_ptr<InstArgument> jumpPoint) : Instruction(InstructionType::IT_JUMP), jump_point(std::move(jumpPoint)) {
        }

        [[nodiscard]] InstArgument *get_jump_point() const {
            return jump_point.get();
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_JUMP;
        }

        static std::unique_ptr<JumpInst> create(std::unique_ptr<InstArgument> jumpPoint) {
            return std::make_unique<JumpInst>(std::move(jumpPoint));
        }
    };

    /**
     * Conditionally branches to the jump point the condition register has a value of zero
     * @param condition must be a register or an immediate
     * @param branch_point must be a label or a register of type ptr
     */
    class BranchInst : public Instruction {
        const Register condition;
        std::unique_ptr<InstArgument> branch_point;

    public:
        BranchInst(Register condition, std::unique_ptr<InstArgument> branchPoint) : Instruction(InstructionType::IT_BREAK),
                                                            condition(std::move(condition)), branch_point(std::move(branchPoint)) {
        }

        [[nodiscard]] const Register &get_condition() const {
            return condition;
        }

        [[nodiscard]] InstArgument *get_branch_point() const {
            return branch_point.get();
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_BREAK;
        }

        static std::unique_ptr<BranchInst> create(Register condition, std::unique_ptr<InstArgument> branchPoint) {
            return std::make_unique<BranchInst>(std::move(condition), std::move(branchPoint));
        }
    };

    /**
     * Call a function at the given ptr
     * @param func_ptr must be either a @link FunctionPtrArgument or a Register of type ptr
     */
    class CallInst : public Instruction {
    public:
        using CallArgs = std::vector<std::unique_ptr<InstArgument>>;
    private:
        const Register dest;
        const std::unique_ptr<InstArgument> func_ptr;
        const CallArgs args;

    public:
        CallInst(Register dest, std::unique_ptr<InstArgument> funcPtr, CallArgs args) : Instruction(
                InstructionType::IT_CALL), dest(std::move(dest)),
            func_ptr(std::move(funcPtr)),
            args(std::move(args)) {
        }

        [[nodiscard]] const Register &get_dest() const {
            return dest;
        }

        [[nodiscard]] InstArgument *get_func_ptr() const {
            return func_ptr.get();
        }

        [[nodiscard]] const CallArgs &get_args() const {
            return args;
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_CALL;
        }

        static std::unique_ptr<CallInst> create(Register dest, std::unique_ptr<InstArgument> func_ptr, CallArgs args) {
            return std::make_unique<CallInst>(std::move(dest), std::move(func_ptr), std::move(args));
        }
    };

    class ReturnInst : public Instruction {
        std::unique_ptr<InstArgument> return_value;

    public:
        ReturnInst(std::unique_ptr<InstArgument> returnValue) : Instruction(InstructionType::IT_RETURN),
                                           return_value(std::move(returnValue)) {
        }

        [[nodiscard]] InstArgument *get_return_value() const {
            return return_value.get();
        }

        [[nodiscard]] std::string print() const override;

        static bool classof(const Instruction *inst) {
            return inst->get_type() == InstructionType::IT_RETURN;
        }

        static std::unique_ptr<ReturnInst> create(std::unique_ptr<InstArgument> returnValue) {
            return std::make_unique<ReturnInst>(std::move(returnValue));
        }
    };
}


#endif //STATEMENT_HPP
