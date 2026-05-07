//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef GLOBAL_HPP
#define GLOBAL_HPP
#include <string>

#include "types.hpp"

namespace IR {

    struct Global {
        std::string identifier;
        Label label;
        MemSize size;
    };

}

#endif //GLOBAL_HPP
