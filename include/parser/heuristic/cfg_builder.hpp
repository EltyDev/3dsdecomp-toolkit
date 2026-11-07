#ifndef CFG_BUILDER_HPP_
#define CFG_BUILDER_HPP_

#include <cstdint>
#include <memory>
#include <vector>
#include <queue>
#include <capstone/capstone.h>
#include "parser/arm_parser.hpp"

struct ControlFlowEdge;

enum class BlockTerminator {
    None,
    Branch,
    ConditionalBranch,
    Return,
    IndirectBranch
};

struct Block {
    uint32_t start;
    uint32_t end;
    cs_mode mode;
    bool visited;
    bool queued;
    BlockTerminator terminator = BlockTerminator::None;
    std::vector<std::shared_ptr<ControlFlowEdge>> prevs;
    std::vector<std::shared_ptr<ControlFlowEdge>> nexts;
};

enum class EdgeType : uint8_t {
    Call,
    Fallthrough,
    ConditionalBranch,
    UnconditionalBranch,
    JumpTable,
    IndirectBranch
};

struct ControlFlowEdge {
    std::shared_ptr<Block> from;
    std::shared_ptr<Block> to;
    EdgeType type;
    arm_cc condition;
};

struct ControlFlowGraph {
    std::shared_ptr<Block> entry;
    std::vector<std::shared_ptr<Block>> blocks;
    std::vector<std::shared_ptr<ControlFlowEdge>> edges;
};

struct CodeEntry {
    uint32_t address;
    cs_mode mode;
};

constexpr arm_cc INVERTED_CONDITIONS[] = {
    ARM_CC_INVALID,
    ARM_CC_NE,
    ARM_CC_EQ,
    ARM_CC_LO,
    ARM_CC_HS,
    ARM_CC_PL,
    ARM_CC_MI,
    ARM_CC_VC,
    ARM_CC_VS,
    ARM_CC_LS,
    ARM_CC_HI,
    ARM_CC_LT,
    ARM_CC_GE,
    ARM_CC_LE,
    ARM_CC_GT,
    ARM_CC_AL
};

namespace ControlFlowGraphBuilder {

    ControlFlowGraph build(ArmParser parser, uint32_t address, std::queue<CodeEntry> *entries = nullptr);
    std::vector<ControlFlowGraph> buildAll(std::vector<uint8_t> &data, uint32_t baseAddress);
}

#endif /* !CFG_BUILDER_HPP_ */
