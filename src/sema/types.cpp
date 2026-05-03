//
// Created by Natalie Wagner on 4/26/26.
//
#include "types.hpp"

#include "sema.hpp"

bool is_integer_type(PrimitiveType type) {
    return type == Char ||
           type == Long ||
           type == Int ||
           type == Short ||
           type == SignedChar ||
           type == UnsignedLong ||
           type == UnsignedInt ||
           type == UnsignedShort ||
           type == UnsignedChar;
}

bool Type::is_void_type() const {
    return type == Void;
}

bool Type::is_integer() const {
    return is_integer_type(type);
}

bool Type::is_signed() const {
    return type == SignedChar || type == Short || type == Int || type == Long;
}

bool Type::is_pointer() const {
    return type == PrimitiveType::Pointer;
}

bool Type::is_struct() const {
    return type == Struct || type == StructRef;
}

bool Type::is_function() const {
    return type == Function;
}

bool Type::is_array() const {
    return type == Array;
}

Type * Type::get(Sema &ctx, PrimitiveType type) {
    if (type == Void) {
        return &ctx.void_type;
    }
    if (is_integer_type(type)) {
        return IntegerType::get(ctx, type);
    }
    throw std::runtime_error("Cannot use Type::get with this primitive type");
}

IntegerType * IntegerType::get(Sema &ctx, PrimitiveType stype) {
    if (const auto type = ctx.integer_types.find(stype); type != ctx.integer_types.end()) {
        return &type->second;
    }
    return &ctx.integer_types.emplace(stype, IntegerType(stype)).first->second;
}

PointerType * PointerType::get(Sema &ctx, QualType pointedType) {
    PointerType pType{pointedType};
    size_t hash = std::hash<PointerType>{}(pType);
    if (const auto type = ctx.pointer_types.find(hash); type != ctx.pointer_types.end()) {
        return &type->second;
    }
    return &ctx.pointer_types.emplace(hash, pType).first->second;
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

StructType * StructRefType::get_complete_type(Sema &ctx) const {
    SymbolTable::Entry * entry = ctx.local_table->findStruct(this->identifier);
    if (!entry->is_complete) {
        throw std::runtime_error("Couldn't find complete type of struct \"" + this->identifier + "\"");
    }
    Type *type = entry->type.type;
    assert(isa<StructType>(type));
    return cast<StructType>(type);
}

StructRefType * StructRefType::get(Sema &ctx, const std::string &identifier) {
    StructRefType sType{identifier};
    size_t hash = std::hash<StructRefType>{}(sType);
    if (const auto type = ctx.struct_ref_types.find(hash); type != ctx.struct_ref_types.end()) {
        return &type->second;
    }
    return &ctx.struct_ref_types.emplace(hash, sType).first->second;
}

ArrayType * ArrayType::get(Sema &ctx, QualType elementType, size_t num_elements) {
    ArrayType aType{elementType, num_elements};
    size_t hash = std::hash<ArrayType>{}(aType);
    if (const auto type = ctx.array_types.find(hash); type != ctx.array_types.end()) {
        return &type->second;
    }
    return &ctx.array_types.emplace(hash, aType).first->second;
}

FunctionType * FunctionType::get(Sema &ctx, const QualType &return_type, const ArgList &argument_types) {
    FunctionType fType{return_type, argument_types};
    size_t hash = std::hash<FunctionType>{}(fType);
    if (const auto type = ctx.function_types.find(hash); type != ctx.function_types.end()) {
        return &type->second;
    }
    return &ctx.function_types.emplace(hash, fType).first->second;
}
