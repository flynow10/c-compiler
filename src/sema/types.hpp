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

enum class PrimitiveType {
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
    StructRef,
    Array,
    Pointer,
    Function,
};

std::string to_string(PrimitiveType type);

struct Type {
    PrimitiveType type = PrimitiveType::Void;

protected:
    friend Sema;
    explicit Type(const PrimitiveType type) : type(type) {
    }

public:
    virtual ~Type() = default;

    [[nodiscard]] bool is_void_type() const;

    [[nodiscard]] bool is_integer() const;

    [[nodiscard]] bool is_signed() const;

    [[nodiscard]] bool is_pointer() const;

    [[nodiscard]] bool is_struct() const;

    [[nodiscard]] bool is_function() const;

    [[nodiscard]] bool is_array() const;

    [[nodiscard]] virtual size_t get_size() const;

    [[nodiscard]] virtual std::string to_string() const;

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

    [[nodiscard]] bool is_struct() const { return type->is_struct(); }

    [[nodiscard]] bool is_function() const { return type->is_function(); }

    [[nodiscard]] bool is_array() const { return type->is_array(); }

    [[nodiscard]] size_t get_size() const { return type->get_size(); }

    [[nodiscard]] std::string to_string() const { return type->to_string(); }

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

    std::string to_string() const override;

    static IntegerType *get(Sema &ctx, PrimitiveType type);

    static bool classof(const Type *type) {
        return type->is_integer();
    }
};

struct PointerType : Type {
    QualType pointed_type;

private:
    PointerType(const QualType pointedType) : Type(PrimitiveType::Pointer), pointed_type(pointedType) {
    }

public:
    static PointerType *get(Sema &ctx, QualType type);

    size_t get_size() const override;
    std::string to_string() const override;

    static bool classof(const Type *type) {
        return type->type == PrimitiveType::Pointer;
    }
};

struct StructType : Type {
    using MemberMap = std::map<std::string, QualType>;

    MemberMap members;

private:
    StructType(MemberMap members) : Type(PrimitiveType::Struct), members(std::move(members)) {
    }

public:
    std::string to_string() const override;

    static StructType *get(Sema &ctx, const MemberMap &members);
    static StructType *convert(Sema &ctx, Type *structOrRef);

    size_t get_size() const override;

    static bool classof(const Type *type) {
        return type->type == PrimitiveType::Struct;
    }
};

struct StructRefType : Type {
    std::string identifier;

private:
    StructRefType(std::string identifier) : Type(PrimitiveType::StructRef), identifier(std::move(identifier)) {}

public:
    StructType *get_complete_type(Sema& ctx) const;

    std::string to_string() const override;

    size_t get_size() const override;

    static StructRefType *get(Sema& ctx, const std::string &identifier);

    static bool classof(const Type *type) {
        return type->type == PrimitiveType::StructRef;
    }
};

struct ArrayType : Type {
    static constexpr size_t UNKNOWN_SIZE = 0;
    QualType element_type;
    size_t num_elements;

private:
    ArrayType(QualType elementType, size_t numElements) : Type(PrimitiveType::Array), element_type(elementType),
                                                          num_elements(numElements) {}

public:
    std::string to_string() const override;

    size_t get_size() const override;

    static ArrayType *get(Sema &ctx, QualType elementType, size_t num_elements);

    static bool classof(const Type *type) {
        return type->type == PrimitiveType::Array;
    }
};

struct FunctionArg {
    QualType type;
    bool is_named;
    std::string identifier;

    FunctionArg(const QualType type) : type(type), is_named(false) {}
    FunctionArg(const QualType type, std::string identifier) : type(type), is_named(true), identifier(std::move(identifier)) {}

    std::string to_string() const;
};

struct FunctionType : Type {
    using ArgList = std::vector<FunctionArg>;
    QualType return_type;
    ArgList argument_types;

private:
    FunctionType(const QualType &returnType, ArgList arguments) : Type(PrimitiveType::Function), return_type(returnType), argument_types(std::move(arguments)) {
    }

public:
    std::string to_string() const override;

    static FunctionType *get(Sema &ctx, const QualType &return_type, const ArgList &argument_types);

    static bool classof(const Type *type) {
        return type->type == PrimitiveType::Function;
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
        return std::hash<PrimitiveType>{}(type.type);
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

template <>
struct std::hash<StructRefType> {
    std::size_t operator()(StructRefType const &type) const noexcept {
        size_t result = 0;
        hash_combine(result, type.type, type.identifier);
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
struct std::hash<FunctionArg> {
    std::size_t operator()(FunctionArg const &functionArg) const noexcept {
        size_t result = 0;
        hash_combine(result, functionArg.type);
        // TODO: Identifier should not be considered in hash since it prevents comparison between equivalent function types
        // if (functionArg.is_named) {
        //     hash_combine(result, functionArg.identifier);
        // }
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
