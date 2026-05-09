//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef IR_TYPES_HPP
#define IR_TYPES_HPP
#include <cstddef>
#include <utility>

namespace IR {
   using Label = unsigned int;
   using MemSize = size_t;
   using MemOffset = int;

   struct Register {
      enum class Type {
         Int, Ptr, Struct
      };
      std::string name;
      Type type;
      MemSize size;

      Register(std::string name, const Type type, const MemSize size) : name(std::move(name)), type(type), size(size) {}

      friend std::ostream &operator<<(std::ostream &os, const Register &reg) {
         return os << std::string("$") << reg.name;
      }
   };
}

#endif //IR_TYPES_HPP
