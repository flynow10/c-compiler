//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef IR_TYPES_HPP
#define IR_TYPES_HPP
#include <iostream>
#include <string>
#include <utility>

namespace IR {
   using Label = unsigned int;
   using MemSize = size_t;
   using MemOffset = int;

   struct Immediate {
      long value;
      MemSize size;
   };

   struct RegOrImmediate;
   struct Register {
      enum class Type {
         Int, Ptr, Struct
      };
      std::string name;
      Type type;
      MemSize size;

      Register(std::string name, const Type type, const MemSize size) : name(std::move(name)), type(type), size(size) {}

      // operator RegOrImmediate() const;

      friend std::ostream &operator<<(std::ostream &os, const Register &reg);
   };

   struct InstArgument {
      enum class Type {
         REGISTER,
         IMMEDIATE,
         LABEL,
         FUNCTION_PTR,
         GLOBAL,
      };
   private:
      Type type;
   protected:
      explicit InstArgument(const Type type) : type(type) {}

      [[nodiscard]] virtual std::string print() const { return ""; }
   public:
      virtual ~InstArgument() = default;
      [[nodiscard]] Type get_type() const { return type; }

      friend std::ostream &operator<<(std::ostream &os, const InstArgument &arg) {
         return os << arg.print();
      }
   };

   struct RegisterArgument : InstArgument {
      Register reg;
      explicit RegisterArgument(Register reg) : InstArgument(Type::REGISTER), reg(std::move(reg)) {}

   protected:
      [[nodiscard]] std::string print() const override;

   public:
      static bool classof(const InstArgument *arg) {
         return arg->get_type() == Type::REGISTER;
      }

      static std::unique_ptr<RegisterArgument> create(const Register &reg) {
         return std::make_unique<RegisterArgument>(reg);
      }
   };

   struct ImmediateArgument : InstArgument {
      Immediate imm;
      ImmediateArgument(const Immediate imm) : InstArgument(Type::IMMEDIATE), imm(imm) {}

   protected:
      [[nodiscard]] std::string print() const override;

   public:
      static bool classof(const InstArgument *arg) {
         return arg->get_type() == Type::IMMEDIATE;
      }

      static std::unique_ptr<InstArgument> create(const long value, const MemSize size) {
         return create(Immediate(value, size));
      }
      static std::unique_ptr<ImmediateArgument> create(const Immediate imm) {
         return std::make_unique<ImmediateArgument>(imm);
      }
   };

   struct LabelArgument : InstArgument {
      Label label;
      explicit LabelArgument(const Label label) : InstArgument(Type::LABEL), label(label) {}

   protected:
      [[nodiscard]] std::string print() const override;

   public:
      static bool classof(const InstArgument *arg) {
         return arg->get_type() == Type::LABEL;
      }

      static std::unique_ptr<LabelArgument> create(const Label &label) {
         return std::make_unique<LabelArgument>(label);
      }
   };

   struct FunctionPtrArgument : InstArgument {
      std::string func_name;
      explicit FunctionPtrArgument(std::string funcName) : InstArgument(Type::FUNCTION_PTR), func_name(std::move(funcName)) {}

   protected:
      [[nodiscard]] std::string print() const override;

   public:
      static bool classof(const InstArgument *arg) {
         return arg->get_type() == Type::FUNCTION_PTR;
      }

      static std::unique_ptr<FunctionPtrArgument> create(const std::string &func_name) {
         return std::make_unique<FunctionPtrArgument>(func_name);
      }
   };

   struct GlobalArgument : InstArgument {
      std::string global_id;
      explicit GlobalArgument(std::string global_id) : InstArgument(Type::GLOBAL), global_id(std::move(global_id)) {}
   protected:
      [[nodiscard]] std::string print() const override;
   public:
      static bool classof(const InstArgument *arg) {
         return arg->get_type() == Type::GLOBAL;
      }

      static std::unique_ptr<InstArgument> create(const std::string &global_id) {
         return std::make_unique<GlobalArgument>(global_id);
      }
   };

   struct RegOrImmediate {
      std::variant<Register, Immediate> value;
      explicit RegOrImmediate(Register reg) : value(std::move(reg)) {}
      explicit RegOrImmediate(Immediate imm) : value(imm) {}

      [[nodiscard]] const Register &get_as_register() const { return std::get<Register>(value); }
      [[nodiscard]] const Immediate &get_as_immediate() const { return std::get<Immediate>(value); }

      [[nodiscard]] MemSize get_size() const {
         if (std::holds_alternative<Register>(value)) {
            return std::get<Register>(value).size;
         }
         return std::get<Immediate>(value).size;
      }

      [[nodiscard]] std::unique_ptr<InstArgument> to_argument() const {
         if (std::holds_alternative<Register>(value)) {
            return RegisterArgument::create(std::get<Register>(value));
         }
         // TODO: Convert immediates type to struct
         return ImmediateArgument::create(std::get<Immediate>(value));
      }
   };
}

#endif //IR_TYPES_HPP
