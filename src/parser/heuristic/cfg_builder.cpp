#include "parser/heuristic/cfg_builder.hpp"
#include "capstone/arm.h"
#include "capstone/capstone.h"
#include "parser/arm_parser.hpp"
#include <memory>
#include <queue>
#include <sys/stat.h>

namespace {
    std::shared_ptr<Block> getOrCreateBlock(ControlFlowGraph& cfg, uint32_t address, cs_mode mode) {
        for (const auto& block : cfg.blocks) {
            if (block->start == address && block->mode == mode)
                return block;
        }
        std::shared_ptr<Block> newBlock = std::make_shared<Block>();
        newBlock->start = address;
        newBlock->mode = mode;
        newBlock->visited = false;
        cfg.blocks.push_back(newBlock);
        return newBlock;
    }

    std::shared_ptr<ControlFlowEdge> getOrCreateEdge(ControlFlowGraph& cfg, std::shared_ptr<Block> from, std::shared_ptr<Block> to, EdgeType type, arm_cc condition) {
        for (const auto& edge : cfg.edges) {
            if (edge->from == from && edge->to == to && edge->type == type && edge->condition == condition)
                return edge;
        }
        std::shared_ptr<ControlFlowEdge> newEdge = std::make_shared<ControlFlowEdge>();
        newEdge->from = from;
        newEdge->to = to;
        newEdge->type = type;
        newEdge->condition = condition;
        from->nexts.push_back(newEdge);
        to->prevs.push_back(newEdge);
        cfg.edges.push_back(newEdge);
        return newEdge;
    }

    bool checkIfBlockExists(ControlFlowGraph& cfg, uint32_t address, cs_mode mode) {
        for (const auto& block : cfg.blocks) {
            if (block->start == address && block->mode == mode)
                return true;
        }
        return false;
    }

    void enqueueBlock(std::shared_ptr<Block> block, std::queue<std::shared_ptr<Block>>& queue) {
        if (!block->visited && !block->queued) {
            block->queued = true;
            queue.push(block);
        }
    }

    void connectBlocks(std::shared_ptr<Block> from, std::shared_ptr<Block> to, EdgeType type, arm_cc condition, ControlFlowGraph& cfg, std::queue<std::shared_ptr<Block>>& queue) {
        getOrCreateEdge(cfg, from, to, type, condition);
        if (type != EdgeType::Call)
            enqueueBlock(to, queue);
    }

    void handleReturn(std::shared_ptr<Block> currentBlock, const cs_insn& insn, ControlFlowGraph& cfg, std::queue<std::shared_ptr<Block>>& toProcess) {
        currentBlock->terminator = BlockTerminator::Return;
        if (insn.detail->arm.cc == ARM_CC_AL)
            return;
        std::shared_ptr<Block> fallthroughBlock = getOrCreateBlock(cfg, static_cast<uint32_t>(insn.address + insn.size), currentBlock->mode);
        connectBlocks(currentBlock, fallthroughBlock, EdgeType::Fallthrough, INVERTED_CONDITIONS[insn.detail->arm.cc], cfg, toProcess);
    }

    void handleBranchLink(std::shared_ptr<Block> currentBlock, const cs_insn& insn, ControlFlowGraph& cfg, std::queue<std::shared_ptr<Block>>& toProcess, std::queue<CodeEntry>* entries, cs_mode mode) {
        std::shared_ptr<Block> nextBlock = getOrCreateBlock(cfg, static_cast<uint32_t>(insn.detail->arm.operands[0].imm), mode);
        connectBlocks(currentBlock, nextBlock, EdgeType::Call, insn.detail->arm.cc, cfg, toProcess);
        if (insn.detail->arm.cc != ARM_CC_AL) {
            std::shared_ptr<Block> fallthroughBlock = getOrCreateBlock(cfg, static_cast<uint32_t>(insn.address + insn.size), currentBlock->mode);
            connectBlocks(currentBlock, fallthroughBlock, EdgeType::Fallthrough, INVERTED_CONDITIONS[insn.detail->arm.cc], cfg, toProcess);
        }
        if (entries)
            entries->push({
                static_cast<uint32_t>(insn.detail->arm.operands[0].imm),
                mode
            });
    }

    bool isReturn(const cs_insn& insn) {
        if (insn.id == ARM_INS_BX && insn.detail->arm.operands[0].reg == ARM_REG_LR)
            return true;
        if (insn.id == ARM_INS_POP) {
            for (size_t i = 0; i < insn.detail->arm.op_count; ++i) {
                if (insn.detail->arm.operands[i].reg == ARM_REG_PC)
                    return true;
            }
        }
        return false;
    }

    void handleBranch(std::shared_ptr<Block> currentBlock, const cs_insn& insn, ControlFlowGraph& cfg, std::queue<std::shared_ptr<Block>>& toProcess) {
        std::shared_ptr<Block> nextBlock = getOrCreateBlock(cfg, static_cast<uint32_t>(insn.detail->arm.operands[0].imm), currentBlock->mode);
        bool isConditional = insn.detail->arm.cc != ARM_CC_AL;
        currentBlock->terminator = isConditional ? BlockTerminator::ConditionalBranch : BlockTerminator::Branch;
        connectBlocks(currentBlock, nextBlock, isConditional ? EdgeType::ConditionalBranch : EdgeType::UnconditionalBranch, insn.detail->arm.cc, cfg, toProcess);
        if (isConditional) {
            std::shared_ptr<Block> fallthroughBlock = getOrCreateBlock(cfg, static_cast<uint32_t>(insn.address + insn.size), currentBlock->mode);
            connectBlocks(currentBlock, fallthroughBlock, EdgeType::Fallthrough, INVERTED_CONDITIONS[insn.detail->arm.cc], cfg, toProcess);
        }
    }
}

namespace ControlFlowGraphBuilder {

        void buildBlock(std::shared_ptr<Block> currentBlock, std::queue<std::shared_ptr<Block>> &toProcess, ControlFlowGraph& cfg, ArmParser &parser, std::queue<CodeEntry> *entries) {
            while (!parser.isEnd()) {
                size_t count;
                cs_insn *instructions = parser.getNextInstructions(64, &count);
                for (size_t i = 0; i < count; ++i) {
                    const cs_insn& insn = instructions[i];
                    currentBlock->end = static_cast<uint32_t>(insn.address + insn.size);
                    if (checkIfBlockExists(cfg, static_cast<uint32_t>(insn.address), currentBlock->mode))
                        return;
                    if (insn.id == ARM_INS_BL)
                        handleBranchLink(currentBlock, insn, cfg, toProcess, entries, currentBlock->mode);
                    else if (insn.id == ARM_INS_BLX) {
                        if (insn.detail->arm.operands[0].type == ARM_OP_IMM)
                            handleBranchLink(currentBlock, insn, cfg, toProcess, entries, currentBlock->mode == CS_MODE_ARM ? CS_MODE_THUMB : CS_MODE_ARM);
                    } else if (insn.id == ARM_INS_B) {
                        handleBranch(currentBlock, insn, cfg, toProcess);
                        return;
                    } else if (isReturn(insn)) {
                        handleReturn(currentBlock, insn, cfg, toProcess);
                        return;
                    }
                }
            }
        }

    ControlFlowGraph build(ArmParser parser, uint32_t address, std::queue<CodeEntry> *entries) {
        ControlFlowGraph cfg;
        std::shared_ptr<Block> currentBlock = getOrCreateBlock(cfg, address, parser.getMode());
        std::queue<std::shared_ptr<Block>> toProcess;
        cfg.entry = currentBlock;
        toProcess.push(currentBlock);
        while (!toProcess.empty()) {
            currentBlock = toProcess.front();
            toProcess.pop();
            currentBlock->queued = false;
            if (currentBlock->visited)
                continue;
            currentBlock->visited = true;
            parser.goToAddress(currentBlock->start);
            parser.changeMode(currentBlock->mode);
            buildBlock(currentBlock, toProcess, cfg, parser, entries);
        }
        return cfg;
    }

    std::vector<ControlFlowGraph> build(std::vector<uint8_t> &data, uint32_t baseAddress) {
        std::vector<ControlFlowGraph> cfgs;
        std::queue<CodeEntry> entries;
        entries.push({baseAddress, CS_MODE_ARM});
        while (!entries.empty()) {
            CodeEntry entry = entries.front();
            entries.pop();
            ControlFlowGraph cfg = build(ArmParser(data, baseAddress, entry.mode),  entry.address, &entries);
            cfgs.push_back(cfg);
        }
        return cfgs;
    }

}