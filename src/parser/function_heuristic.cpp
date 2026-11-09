#include "parser/function_heuristic.hpp"
#include "parser/arm_parser.hpp"
#include <capstone/capstone.h>
#include <memory>
#include <vector>

struct Block {
    uint32_t start;
};

struct ConditionalBlock : public Block {
    arm_cc condition;

    ConditionalBlock(uint32_t startAddress, arm_cc cond) : Block{startAddress}, condition{cond} {}
};

struct Entry {
    std::vector<std::unique_ptr<Block>> blocks;
};

namespace {


std::unique_ptr<Block> analyseFlow(const std::vector<uint8_t>& data, uint32_t baseAddress, uint32_t startAddress)
{
    ArmParser parser(data, baseAddress);
    parser.goToAddress(startAddress);
    while (!parser.isEnd()) {
        size_t count;
        cs_insn *instructions = parser.getNextInstructions(64, &count);
        for (size_t i = 0; i < count; ++i) {
            const cs_insn& insn = instructions[i];
            if (insn.id == ARM_INS_B) {
                return std::make_unique<ConditionalBlock>(insn.address, insn.detail->arm.cc);
            }
        }
    }
    return nullptr;
}

void analyzeFunction(const std::vector<uint8_t>& data, uint32_t baseAddress, uint32_t startAddress)
{
    Entry entry;
    entry.blocks.push_back(std::make_unique<Block>(startAddress));
    ArmParser parser(data, baseAddress);
    parser.goToAddress(startAddress);
    while (!parser.isEnd()) {
        size_t count;
        cs_insn *instructions = parser.getNextInstructions(64, &count);
        for (size_t i = 0; i < count; ++i) {
            const cs_insn& insn = instructions[i];
            if (insn.id == ARM_INS_B) {
                entry.blocks.push_back(std::make_unique<ConditionalBlock>(insn.address, insn.detail->arm.cc));
            }
        }
    }

}

}


std::vector<Function> function_heuristic::findFunctions(const std::vector<uint8_t>& data, uint32_t baseAddress)
{
    analyzeFunction(data, baseAddress, 0x141AC0);
    exit(1);
}