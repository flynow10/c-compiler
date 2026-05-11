//
// Created by Natalie Wagner on 2/17/26.
//

#include "ast.hpp"

#include <sstream>
#include <string>

std::string AST::to_string(const Node *node, const int indent) {
    std::stringstream result;
    const auto spaces = std::string(indent, '|');

    result << spaces << node->get_name();
    result << " " << std::hex << &*node << std::endl;

    for (const auto &child: *node) {
        if (child == nullptr) continue;
        result << to_string(child.get(), indent + 1);
    }
    return result.str();
}

const std::string & AST::Declarator::get_identifier() const {
    if (isAbstract) {
        throw std::runtime_error("Cannot get identifier of abstract declarator");
    }
    Node * directDecl = get_direct_declarator();
    if (isa<Declarator>(directDecl)) {
        return cast<Declarator>(directDecl)->get_identifier();
    }
    return cast<DirectDeclarator>(directDecl)->get_identifier();
}

bool AST::FunctionDecl::_has_parameter_list() const {
    return _get_parameter_list() != nullptr;
}

AST::ParameterList * AST::FunctionDecl::_get_parameter_list() const {
    auto *decl = get_declarator();
    auto *parameterizedDeclarator = cast<ParameterizedDeclarator>(decl->get_suffix());
    return parameterizedDeclarator->get_parameter_list();
}
/*
** Convert character to digit in given base,
** returning -1 for invalid bases and characters.
*/
int basedigit(char c, int base)
{
    int             i;

#if (('z' - 'a') != 25 || ('Z' - 'A') != 25)
#error Faulty Assumption
    This code assumes the code set is ASCII, ISO 646, ISO 8859, or something similar.
    #endif /* Alphabet test */
        if (base < 2 || base > 36)
            i = -1;
        else if (c >= '0' && c <= '9')
            i = c - '0';
        else if (c >= 'A' && c <= 'Z')
            i = c - 'A' + 10;
        else if (c >= 'a' && c <= 'z')
            i = c - 'a' + 10;
        else
            i = -1;
    return((i < base) ? i : -1);
}

/* Convert C Character Literal in (str..end] (excluding surrounding quotes) */
/* to character, returning converted char or -1 if string is invalid. */

/* Convert string containing C character literal to character value */
/* Returns -1 if character literal is invalid, otherwise 0x00..0xFF */
/* Does not support extension \E for ESC \033. */
/* Does not support any extension for DEL \177. */
/* Does not support control-char notation ^A for CTRL-A \001. */
/* Accepts \z as valid z when z is not otherwise special. */
/* Accepts \038 as valid CTRL-C \003; next character starts with the 8. */
/* Accepts \x3Z as valid CTRL-C \003; next character starts with the Z. */
/* Treats invalid octal escape \8 or \9 as 8 or 9 */
long cstr_to_long(const std::string &constant) {
    unsigned char u;
    int rv;
    auto str = constant.begin();
    auto end = constant.end();

    if (str >= end)
        rv = -1;    /* String contains no data */
    else if ((u = *str++) != '\\')
        rv = u;
    else if (str == end)
        rv = -1;    /* Just a backslash - invalid */
    else if ((u = *str++) == 'x')
    {
        /**
        ** Hex character constant - \xHH or \xH, where H is a hex digit.
        ** Technically, can be \xHHH too, if CHAR_BIT > 8; this nicety
        ** is being studiously ignored.
        */
        int x1;
        int x2;
        if (str == end)
            rv = -1;
        else if ((x1 = basedigit(*str++, 16)) < 0)
        {
            rv = -1;        /* Invalid hex constant */
            str--;
        }
        else if (str == end)
            rv = x1;        /* Single digit hex constant */
        else if ((x2 = basedigit(*str++, 16)) < 0)
        {
            rv = x1;        /* Single-digit hex constant */
            str--;
        }
        else
            rv = (x1 << 4) | x2;    /* Double-digit hex constant */
    }
    else if (isdigit(u))
    {
        /**
        ** Octal character constant - \O or \OO or \OOO, where O is an
        ** octal digit.  Technically, the constant extends for an
        ** indefinite number of octal digits; this nicety is being
        ** studiously ignored.  Treat \8 as 8 and \9 as 9.
        */
        int o1;
        int o2;
        int o3;
        if ((o1 = basedigit(u, 8)) < 0)
            rv = u; /* Invalid octal constant (\8 or \9) */
        else if (str == end)
            rv = o1;    /* Single-digit octal constant */
        else if ((o2 = basedigit(*str++, 8)) < 0)
        {
            rv = o1;    /* Single-digit octal constant */
            str--;
        }
        else if (str == end)
            rv = (o1 << 3) | o2;    /* Double-digit octal constant */
        else if ((o3 = basedigit(*str++, 8)) < 0)
        {
            rv = (o1 << 3) | o2;    /* Double-digit octal constant */
            str--;
        }
        else if (o1 >= 4)
            rv = -1;                /* Out of range 0x00..0xFF (\000..\377) */
        else
            rv = (((o1 << 3) | o2) << 3) | o3;
    }
    else {
        /* Presumably \a, \b, \f, \n, \r, \t, \v, \', \", \? or \\ - or an error */
        switch (u)
        {
            case 'a':
                rv = '\a';
                break;
            case 'b':
                rv = '\b';
                break;
            case 'f':
                rv = '\f';
                break;
            case 'n':
                rv = '\n';
                break;
            case 'r':
                rv = '\r';
                break;
            case 't':
                rv = '\t';
                break;
            case 'v':
                rv = '\v';
                break;
            case '\"':
                rv = '\"';
                break;
            case '\'':
                rv = '\'';
                break;
            case '\?':
                rv = '\?';
                break;
            case '\\':
                rv = '\\';
                break;
            case '\0':  /* Malformed: solitary backslash followed by NUL */
                rv = -1;
                break;
            default:
                rv = u; /* Nominally invalid: \X but X not special; return X. */
                break;
        }
    }
    return rv;
}

bool parse_long(const std::string& str, long& output, int base = 10) {
    try {
        // Ensure full string is used in converting to a number
        size_t pos;
        output = std::stol(str, &pos, base);
        if (pos != str.length()) {
            return false;
        }
        return true;
    } catch ([[maybe_unused]] std::invalid_argument& e) {
        return false;
    }
}

long AST::Constant::get_parsed_value() const {
    if (constant.starts_with("'")) {
        return cstr_to_long(constant.substr(1, constant.length() - 2));
    }
    if (constant.starts_with("0x")) {
        long value;
        if (!parse_long(constant.substr(2), value, 16)) {
            throw std::runtime_error("Invalid constant");
        }
        return value;
    }
    long value;
    if (!parse_long(constant, value)) {
        throw std::runtime_error("Invalid constant");
    }
    return value;
}
