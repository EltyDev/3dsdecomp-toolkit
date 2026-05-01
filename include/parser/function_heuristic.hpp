#ifndef FUNCTION_HEURISTIC_HPP_
#define FUNCTION_HEURISTIC_HPP_

#include <stdint.h>
#include <vector>
#include <capstone/capstone.h>

struct Function
{
    cs_mode mode;
    uint32_t start;
    uint32_t end;
};

namespace function_heuristic
{
    std::vector<Function> findFunctions(const std::vector<uint8_t>& data);
}

namespace
{
    bool isFunctionEnd(cs_insn& instruction);
}


#endif /* !FUNCTION_HEURISTIC_HPP_ */
