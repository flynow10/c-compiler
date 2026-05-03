//
// Created by Natalie Wagner on 4/26/26.
//

#ifndef TYPES_HPP
#define TYPES_HPP
#include <map>
#include <utility>

#include "../hash_combine.hpp"

// ----------------------------
// Expression Types
// ----------------------------

class Sema;

enum PrimitiveType {
    Void,
    Char,
    Long,
    Int,
    Short,
    SignedChar,
    UnsignedLong,
    UnsignedInt,
    UnsignedShort,
    UnsignedChar,
    Struct,
    Array,
    Pointer,
    Function,
};

struct Type {
    PrimitiveType type = Void;

protected:
    friend Sema;
    explicit Type(const PrimitiveType type) : type(type) {
    }

public:
    [[nodiscard]] bool is_void_type() const;

    [[nodiscard]] bool is_integer() const;

    [[nodiscard]] bool is_signed() const;

    [[nodiscard]] bool is_pointer() const;

    static bool classof(const Type *type) {
        return true;
    }

    static Type *get(Sema &ctx, PrimitiveType type);
};

struct QualType {
    Type *type = nullptr;
    bool is_const = false;

    [[nodiscard]] bool is_void_type() const { return type->is_void_type(); }

    [[nodiscard]] bool is_integer() const { return type->is_integer(); }

    [[nodiscard]] bool is_signed() const { return type->is_signed(); }

    [[nodiscard]] bool is_pointer() const { return type->is_pointer(); }

    bool operator==(const QualType &other) const {
        if (this->type != nullptr && this->type == other.type && this->is_const == other.is_const) {
            return true;
        }
        return false;
    }
};

struct IntegerType : Type {
    explicit IntegerType(PrimitiveType type) : Type(type) {
    }

    static IntegerType *get(Sema &ctx, PrimitiveType type);

    static bool classof(const Type *type) {
        return type->is_integer();
    }
};

struct PointerType : Type {
    QualType pointed_type;

private:
    PointerType(const QualType pointedType) : Type(Pointer), pointed_type(pointedType) {
    }

public:
    static PointerType *get(Sema &ctx, QualType type);

    static bool classof(const Type *type) {
        return type->type == Pointer;
    }
};

struct StructType : Type {
    using MemberMap = std::map<std::string, QualType>;

    MemberMap members;

private:
    StructType(MemberMap members) : Type(Struct), members(std::move(members)) {
    }

public:
    static StructType *get(Sema &ctx, const MemberMap &members);

    static bool classof(const Type *type) {
        return type->type == Struct;
    }
};

struct ArrayType : Type {
    QualType element_type;
    size_t num_elements;

private:
    ArrayType(QualType elementType, size_t numElements) : Type(Array), element_type(elementType),
                                                          num_elements(numElements) {}

public:
    static ArrayType *get(Sema &ctx, QualType elementType, size_t num_elements);

    static bool classof(const Type *type) {
        return type->type == Array;
    }
};

struct FunctionType : Type {
    QualType return_type;
    std::vector<QualType> argument_types;

private:
    FunctionType() : Type(Function) {
    }

public:
    static FunctionType *get(Sema &ctx, QualType return_type, std::vector<QualType> &argument_types);

    static bool classof(const Type *type) {
        return type->type == Function;
    }
};

// Type hashing

template<>
struct std::hash<QualType> {
    std::size_t operator()(QualType const &type) const noexcept {
        size_t result = 0xa9c223f;
        hash_combine(result, type.type, type.is_const);
        return result;
    }
};

template<>
struct std::hash<IntegerType> {
    std::size_t operator()(IntegerType const &type) const noexcept {
        return std::hash<int>{}(type.type);
    }
};

template<>
struct std::hash<PointerType> {
    std::size_t operator()(PointerType const &type) const noexcept {
        size_t result = 0;
        hash_combine(result, type.type, type.pointed_type);
        return result;
    }
};

template<>
struct std::hash<StructType> {
    std::size_t operator()(StructType const &structType) const noexcept {
        size_t result = 0;
        hash_combine(result, structType.type);
        for (const auto &[id, type]: structType.members) {
            hash_combine(result, id, type);
        }
        return result;
    }
};

template<>
struct std::hash<ArrayType> {
    std::size_t operator()(ArrayType const &arrayType) const noexcept {
        size_t result = 0;
        hash_combine(result, arrayType.element_type, arrayType.num_elements);
        return result;
    }
};

template<>
struct std::hash<FunctionType> {
    std::size_t operator()(FunctionType const &type) const noexcept {
        size_t result = 0;
        hash_combine(result, type.type, type.return_type);
        for (const auto &argument_type: type.argument_types) {
            hash_combine(result, argument_type);
        }
        return result;
    }
};

#endif //TYPES_HPP
