#include "parser/function_heuristic.hpp"
#include "parser/arm_parser.hpp"
#include <cstring>

bool isFunctionEnd(cs_insn& instruction)
{
    if (!strcmp(instruction.mnemonic, "pop")) {
        for (auto& op : instruction.detail->arm.operands) {
            if (op.type == ARM_OP_REG && op.reg == ARM_REG_PC)
                return true;
        }
    } if (!strcmp(instruction.mnemonic, "bx")) {
        if (instruction.detail->arm.operands[0].reg == ARM_REG_LR)
            return true;
    }
    return false;
}

bool isFunctionStart(cs_insn& instruction)
{
    if (!strcmp(instruction.mnemonic, "push")) {
        for (auto& op : instruction.detail->arm.operands) {
            if (op.type == ARM_OP_REG && op.reg == ARM_REG_LR)
                return true;
        }
    }
    return false;
}

std::vector<Function> function_heuristic::findFunctions(const std::vector<uint8_t>& data)
{
    std::vector<Function> functions;
    ArmParser parser(data);
    return functions;
}