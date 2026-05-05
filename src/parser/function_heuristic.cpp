#include "parser/function_heuristic.hpp"
#include "capstone/arm.h"
#include "capstone/capstone.h"
#include "parser/arm_parser.hpp"
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <memory>
#include <queue>
#include <vector>
#include <iostream>

namespace
{
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

    bool isNewFunction(cs_insn& instruction)
    {
        if (!strcmp(instruction.mnemonic, "bl") || !strcmp(instruction.mnemonic, "blx"))
            return true;
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

    cs_mode getNextMode(cs_insn& instruction, cs_mode currentMode)
    {
        if (!strcmp(instruction.mnemonic, "blx") && instruction.detail->arm.operands[0].type == ARM_OP_IMM) {
            cs_mode targetMode = instruction.detail->arm.operands[0].imm & 1 ? CS_MODE_THUMB : CS_MODE_ARM;
            return targetMode;
        }
        return currentMode;
    }

    std::vector<Function> linearScan(const std::vector<uint8_t>& data, uint32_t baseAddress, cs_mode mode)
    {
        std::vector<Function> functions;
        ArmParser parser(data, baseAddress, mode);
        uint32_t start = 0;
        bool inFunction = false;
        while (!parser.isEnd()) {
            cs_insn *ins = parser.getNextInstruction();
            if (!ins)
                continue;
            if (!inFunction && isFunctionStart(*ins)) {
                start = ins->address;
                inFunction = true;
            }
            if (inFunction && isFunctionEnd(*ins)) {
                functions.emplace_back(mode, start, ins->address);
                inFunction = false;
                start = 0;
            }
        }
        std::cout << "RAM usage after linear scan: " << functions.size() * sizeof(Function) << " bytes\n";
        return functions;
    }

}

std::vector<Function> function_heuristic::findFunctions(const std::vector<uint8_t>& data, uint32_t baseAddress)
{
    std::vector<Function> functions;
    auto armFunctions = linearScan(data, baseAddress, CS_MODE_ARM);
    auto thumbFunctions = linearScan(data, baseAddress, CS_MODE_THUMB);
    functions.insert(functions.end(), armFunctions.begin(), armFunctions.end());
    functions.insert(functions.end(), thumbFunctions.begin(), thumbFunctions.end());
    return functions;
    std::queue<HeuristicEntry> toProcess({{baseAddress, CS_MODE_ARM, false}});
    for (const auto& func : functions)
        toProcess.push({func.start, func.mode, true});
    std::vector<uint32_t> processed;
    ArmParser parser(data, baseAddress);
    while (!toProcess.empty()) {
        HeuristicEntry entry = toProcess.front();
        parser.goToAddress(entry.address);
        parser.changeMode(entry.mode);
        toProcess.pop();
        if (std::find(processed.begin(), processed.end(), entry.address) != processed.end())
            continue;
        processed.push_back(entry.address);
        while (!parser.isEnd()) {
            cs_insn *ins = parser.getNextInstruction();
            if (!ins)
                continue;
            if (isNewFunction(*ins)) {
                uint32_t target = ins->detail->arm.operands[0].type == ARM_OP_IMM ? static_cast<uint32_t>(ins->detail->arm.operands[0].imm) & ~1u : 0;
                if (target == 0 || target < baseAddress || target >= baseAddress + data.size())
                    continue;
                toProcess.push({static_cast<uint32_t>(ins->detail->arm.operands[0].imm) & ~1u, getNextMode(*ins, entry.mode), false});
            } if (isFunctionEnd(*ins)) {
                if (entry.foundBefore)
                    functions.emplace_back(entry.mode, entry.address, ins->address);
                break;
            }
            if (!strcmp(ins->mnemonic, "b") && ins->detail->arm.operands[0].type == ARM_OP_IMM) {
                uint32_t target = ins->detail->arm.operands[0].imm;
                if (std::find(processed.begin(), processed.end(), target) != processed.end()) {
                    if (entry.foundBefore)
                        functions.emplace_back(entry.mode, entry.address, ins->address);
                    break;
                }
            }
        }
    }
    return functions;
}