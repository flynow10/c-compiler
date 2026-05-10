//
// Created by Natalie Wagner on 4/26/26.
//
#include "types.hpp"

#include <ranges>
#include <sstream>

#include "sema.hpp"

bool is_integer_type(PrimitiveType type) {
    return type == PrimitiveType::Char ||
           type == PrimitiveType::Long ||
           type == PrimitiveType::Int ||
           type == PrimitiveType::Short ||
           type == PrimitiveType::SignedChar ||
           type == PrimitiveType::UnsignedLong ||
           type == PrimitiveType::UnsignedInt ||
           type == PrimitiveType::UnsignedShort ||
           type == PrimitiveType::UnsignedChar;
}

std::string primitive_to_string(PrimitiveType type) {
    static const auto strings = [] {
        std::map<PrimitiveType,std::string> result;
#define INSERT_ELEMENT(p) result.emplace(p, #p);
        INSERT_ELEMENT(PrimitiveType::Void)
        INSERT_ELEMENT(PrimitiveType::Char)
        INSERT_ELEMENT(PrimitiveType::Long)
        INSERT_ELEMENT(PrimitiveType::Int)
        INSERT_ELEMENT(PrimitiveType::Short)
        INSERT_ELEMENT(PrimitiveType::SignedChar)
        INSERT_ELEMENT(PrimitiveType::UnsignedLong)
        INSERT_ELEMENT(PrimitiveType::UnsignedInt)
        INSERT_ELEMENT(PrimitiveType::UnsignedShort)
        INSERT_ELEMENT(PrimitiveType::UnsignedChar)
        INSERT_ELEMENT(PrimitiveType::Struct)
        INSERT_ELEMENT(PrimitiveType::StructRef)
        INSERT_ELEMENT(PrimitiveType::Array)
        INSERT_ELEMENT(PrimitiveType::Pointer)
        INSERT_ELEMENT(PrimitiveType::Function)
        return result;
    }();
    return strings.at(type);
}

bool Type::is_void_type() const {
    return type == PrimitiveType::Void;
}

bool Type::is_integer() const {
    return is_integer_type(type);
}

bool Type::is_signed() const {
    return type == PrimitiveType::SignedChar || type == PrimitiveType::Short || type == PrimitiveType::Int || type == PrimitiveType::Long;
}

bool Type::is_pointer() const {
    return type == PrimitiveType::Pointer;
}

bool Type::is_struct() const {
    return type == PrimitiveType::Struct || type == PrimitiveType::StructRef;
}

bool Type::is_function() const {
    return type == PrimitiveType::Function;
}

bool Type::is_array() const {
    return type == PrimitiveType::Array;
}

size_t Type::get_size() const {
    switch (type) {
        case PrimitiveType::Void:
            return 0;
        case PrimitiveType::Char:
        case PrimitiveType::SignedChar:
        case PrimitiveType::UnsignedChar:
            return 1;
        case PrimitiveType::Short:
        case PrimitiveType::UnsignedShort:
            return 2;
        case PrimitiveType::Int:
        case PrimitiveType::UnsignedInt:
        case PrimitiveType::Long:
        case PrimitiveType::UnsignedLong:
            return 4;
        default:
            throw std::runtime_error{"Could not get size of type"};
    }
}

std::string Type::to_string() const {
    std::stringstream ss;
    ss << "Type(" << primitive_to_string(this->type) << ")";
    return ss.str();
}

Type * Type::get(Sema &ctx, PrimitiveType type) {
    if (type == PrimitiveType::Void) {
        return &ctx.void_type;
    }
    if (is_integer_type(type)) {
        return IntegerType::get(ctx, type);
    }
    throw std::runtime_error("Cannot use Type::get with this primitive type");
}

std::string IntegerType::to_string() const {
    std::stringstream ss;
    ss << "IntegerType(" << primitive_to_string(type) << ")";
    return ss.str();
}

IntegerType * IntegerType::get(Sema &ctx, PrimitiveType stype) {
    IntegerType iType{stype};
    size_t hash = std::hash<IntegerType>{}(iType);
    if (const auto type = ctx.integer_types.find(hash); type != ctx.integer_types.end()) {
        return &type->second;
    }
    return &ctx.integer_types.emplace(hash, iType).first->second;
}

PointerType * PointerType::get(Sema &ctx, QualType pointedType) {
    PointerType pType{pointedType};
    size_t hash = std::hash<PointerType>{}(pType);
    if (const auto type = ctx.pointer_types.find(hash); type != ctx.pointer_types.end()) {
        return &type->second;
    }
    return &ctx.pointer_types.emplace(hash, pType).first->second;
}

size_t PointerType::get_size() const {
    return 4;
}

std::string PointerType::to_string() const {
    std::stringstream ss;
    ss << "PointerType(" << pointed_type.to_string() << ")";
    return ss.str();
}

std::string StructType::to_string() const {
    std::stringstream ss;
    ss << "StructType(" << std::endl;
    for (const auto &[memberName, type] : this->members) {
        ss << memberName << ": " << type.to_string() << "," << std::endl;
    }
    ss << ")";
    return ss.str();
}

StructType * StructType::get(Sema &ctx, const std::map<std::string, QualType> &members) {
    StructType sType{members};
    size_t hash = std::hash<StructType>{}(sType);
    if (const auto type = ctx.struct_types.find(hash); type != ctx.struct_types.end()) {
        return &type->second;
    }
    return &ctx.struct_types.emplace(hash, sType).first->second;
}

StructType * StructType::convert(Sema &ctx, Type *structOrRef) {
    assert(isa<StructType>(structOrRef) || isa<StructRefType>(structOrRef));
    StructType * type;
    if (isa<StructType>(structOrRef)) {
        type = cast<StructType>(structOrRef);
    } else {
        type = cast<StructRefType>(structOrRef)->get_complete_type(ctx);
    }
    return type;
}

size_t StructType::get_size() const {
    size_t size = 0;
    size_t alignment = 0;
    for (auto type: members | std::views::values) {
        const size_t memberSize = type.get_size();
        size += memberSize;
        if (memberSize != 0 && memberSize < 4) {
            alignment += 4 - memberSize;
        }
    }
    return size + alignment;
}

StructType * StructRefType::get_complete_type(Sema &ctx) const {
    Entry * entry = ctx.local_table->findStruct(this->identifier);
    if (!entry->is_complete) {
        throw std::runtime_error("Couldn't find complete type of struct \"" + this->identifier + "\"");
    }
    Type *type = entry->type.type;
    assert(isa<StructType>(type));
    return cast<StructType>(type);
}

std::string StructRefType::to_string() const {
    std::stringstream ss;
    ss << "StructRefType(" << this->identifier << ")";
    return ss.str();
}

size_t StructRefType::get_size() const {
    throw std::runtime_error("Cannot get size of reference to struct type");
}

StructRefType * StructRefType::get(Sema &ctx, const std::string &identifier) {
    StructRefType sType{identifier};
    size_t hash = std::hash<StructRefType>{}(sType);
    if (const auto type = ctx.struct_ref_types.find(hash); type != ctx.struct_ref_types.end()) {
        return &type->second;
    }
    return &ctx.struct_ref_types.emplace(hash, sType).first->second;
}

std::string ArrayType::to_string() const {
    std::stringstream ss;
    ss << "ArrayType(" << this->element_type.to_string() << ", " << this->num_elements << ")";
    return ss.str();
}

size_t ArrayType::get_size() const {
    size_t elementSize = element_type.get_size();
    return elementSize * num_elements;
}

ArrayType * ArrayType::get(Sema &ctx, QualType elementType, size_t num_elements) {
    ArrayType aType{elementType, num_elements};
    size_t hash = std::hash<ArrayType>{}(aType);
    if (const auto type = ctx.array_types.find(hash); type != ctx.array_types.end()) {
        return &type->second;
    }
    return &ctx.array_types.emplace(hash, aType).first->second;
}

std::string FunctionArg::to_string() const {
    std::stringstream ss;
    ss << "(" << type.to_string();
    if (is_named) {
        ss << " " << identifier;
    }
    ss << ")";
    return ss.str();
}

std::string FunctionType::to_string() const {
    std::stringstream ss;
    ss << "FunctionType(" << this->return_type.to_string() << ",";
    for (const auto & argument_type : this->argument_types) {
        ss << argument_type.to_string() << ",";
    }

    return ss.str();
}

FunctionType * FunctionType::get(Sema &ctx, const QualType &return_type, const ArgList &argument_types) {
    FunctionType fType{return_type, argument_types};
    size_t hash = std::hash<FunctionType>{}(fType);
    if (const auto type = ctx.function_types.find(hash); type != ctx.function_types.end()) {
        return &type->second;
    }
    return &ctx.function_types.emplace(hash, fType).first->second;
}
