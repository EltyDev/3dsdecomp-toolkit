#include "parser/function_heuristic.hpp"
#include "capstone/arm.h"
#include "capstone/capstone.h"
#include "parser/arm_parser.hpp"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <cstdint>
#include <vector>
#include <iostream>
#include <thread>

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
            if (instruction.detail->arm.cc == ARM_CC_AL && instruction.detail->arm.operands[0].reg == ARM_REG_LR)
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
        if (!strcmp(instruction.mnemonic, "ldr") &&
            instruction.detail->arm.operands[0].type == ARM_OP_REG &&
            instruction.detail->arm.operands[0].reg == ARM_REG_PC)
            return true;
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
            size_t count;
            cs_insn *block = parser.getNextInstructions(64, &count);
            if (!block)
                continue;
            for (size_t i = 0; i < count; i++) {
                cs_insn& ins = block[i];
                if (!inFunction && isFunctionStart(ins)) {
                    start = ins.address;
                    inFunction = true;
                }
                if (inFunction && isFunctionEnd(ins)) {
                    functions.emplace_back(mode, start, ins.address + ins.size);
                    inFunction = false;
                    start = 0;
                }
            }
        }
        std::cout << "Found " << functions.size() << " functions in " << (mode == CS_MODE_ARM ? "ARM" : "THUMB") << " mode." << std::endl;
        return functions;
    }

    void threadHeuristic(HeuristicContext &context, ArmParser parser) {
    size_t dataSize = parser.getDataSize();
    uint32_t baseAddress = parser.getBaseAddress();
    std::vector<Function> localFunctions;

    while (true) {
        HeuristicEntry entry;

        if (!context.toProcess.try_pop(entry)) {
            if (context.pendingTasks.load(std::memory_order_acquire) == 0)
                break;
            std::this_thread::yield();
            continue;
        }

        parser.goToAddress(entry.address);
        parser.changeMode(entry.mode);

        if (!context.processed.insert(entry.address)) {
            context.pendingTasks.fetch_sub(1, std::memory_order_release);
            continue;
        }

        context.knownFunctions.insert(entry.address);

        while (!parser.isEnd()) {
            size_t count;
            cs_insn *block = parser.getNextInstructions(64, &count);
            if (!block)
                continue;

            for (size_t i = 0; i < count; i++) {
                cs_insn& ins = block[i];

                if (ins.address != entry.address && context.knownFunctions.contains(ins.address)) {
                    if (!entry.foundBefore)
                        localFunctions.emplace_back(entry.mode, entry.address, ins.address);
                    goto nextEntry;
                }

                if (isNewFunction(ins)) {
                    uint32_t target =
                        ins.detail->arm.operands[0].type == ARM_OP_IMM ?
                        static_cast<uint32_t>(ins.detail->arm.operands[0].imm) & ~1u :
                        0;

                    if (target == 0 || target < baseAddress || target >= baseAddress + dataSize)
                        continue;

                    if (context.scheduled.insert(target)) {
                        context.pendingTasks.fetch_add(1, std::memory_order_release);
                        context.toProcess.push({
                            target,
                            getNextMode(ins, entry.mode),
                            false
                        });
                    }
                }

                if (isFunctionEnd(ins)) {
                    if (!entry.foundBefore)
                        localFunctions.emplace_back(entry.mode, entry.address, ins.address + ins.size);
                    goto nextEntry;
                }

                if (!strcmp(ins.mnemonic, "b") &&
                    ins.detail->arm.operands[0].type == ARM_OP_IMM) {

                    uint32_t target = ins.detail->arm.operands[0].imm & ~1u;

                    if (target == 0 || target < baseAddress || target >= baseAddress + dataSize)
                        continue;

                    bool isKnownFunction = context.knownFunctions.contains(target);
                    bool isTailCall = isKnownFunction || target < entry.address;

                    if (context.scheduled.insert(target)) {
                        context.pendingTasks.fetch_add(1, std::memory_order_release);
                        context.toProcess.push({
                            target,
                            entry.mode,
                            !isKnownFunction
                        });
                    }

                    if (isTailCall) {
                        if (!entry.foundBefore)
                            localFunctions.emplace_back(entry.mode, entry.address, ins.address + ins.size);
                        goto nextEntry;
                    }
                }
            }
        }

nextEntry:
        context.pendingTasks.fetch_sub(1, std::memory_order_release);
    }

    context.functions.insert(localFunctions.begin(), localFunctions.end());
}
}

std::vector<Function> function_heuristic::findFunctions(const std::vector<uint8_t>& data, uint32_t baseAddress)
{
    ConcurrentVector<Function> functions;
    auto armFunctions = linearScan(data, baseAddress, CS_MODE_ARM);
    auto thumbFunctions = linearScan(data, baseAddress, CS_MODE_THUMB);
    functions.insert(functions.end(), armFunctions.begin(), armFunctions.end());
    functions.insert(functions.end(), thumbFunctions.begin(), thumbFunctions.end());
    ConcurrentQueue<HeuristicEntry> toProcess;
    AtomicBitmap knownFunctions(data.size(), baseAddress);
    toProcess.push({baseAddress, cs_mode::CS_MODE_ARM, false});
    for (const auto& func : functions) {
        toProcess.push({func.start, func.mode, true});
        knownFunctions.insert(func.start);
    }
    ConcurrentVector<uint32_t> processed;
    ArmParser parser(data, baseAddress);
    std::cout << "Starting heuristic function search with " << toProcess.size() << " initial entries." << std::endl;
    std::vector<std::thread> threads(std::thread::hardware_concurrency());
    HeuristicContext context{std::atomic_int32_t(toProcess.size()), toProcess,AtomicBitmap(data.size(), baseAddress), AtomicBitmap(data.size(), baseAddress), functions, std::move(knownFunctions)};
    for (auto& thread : threads)
        thread = std::thread(threadHeuristic, std::ref(context), ArmParser(data, baseAddress));
    for (auto& thread : threads)
        thread.join();
    std::sort(context.functions.begin(), context.functions.end(), [](const Function& a, const Function& b) {
        return a.start < b.start;
    });
    std::cout << "Found " << context.functions.size() << " functions after heuristic search." << std::endl;
    std::cout << "Removing duplicates from function list." << std::endl;
    (*context.functions).erase(std::unique(context.functions.begin(), context.functions.end(), [](const Function& a, const Function& b) {
        return a.start == b.start;
    }), context.functions.end());
    std::cout << "Removing overlapping functions from function list." << std::endl;
    std::vector<Function *> toDelete;
    for (size_t i = 0; i + 1 < context.functions.size(); i++) {
        Function& curr = context.functions[i];
        Function& next = context.functions[i + 1];
        if (curr.end > next.start) {
            curr.end = next.start;
        }
        size_t size = curr.end - curr.start;
        size_t instructionSize = curr.mode == CS_MODE_ARM ? 4 : 2;
        if (size % instructionSize != 0)
            curr.mode = (curr.mode == CS_MODE_ARM ? CS_MODE_THUMB : CS_MODE_ARM);
        if (size <= instructionSize && curr.end == next.start) {
            curr.end = next.end;
            curr.mode = next.mode;
            toDelete.push_back(&next);
        }
    }
    for (Function* func : toDelete) {
        (*context.functions).erase(std::remove_if(context.functions.begin(), context.functions.end(), [&](const Function& f) {
            return &f == func;
        }), context.functions.end());
    }
    auto sorted = *context.functions;
    std::cout << "Removing functions that are fully contained in other functions from function list." << std::endl;
    (*context.functions).erase(std::remove_if(context.functions.begin(), context.functions.end(), [&](const Function& f) {
        auto it = std::lower_bound(sorted.begin(), sorted.end(), f, [](const Function& a, const Function& b) {
            return a.start < b.start;
        });
        if (it != sorted.begin()) {
            --it;
            if (f.start >= it->start && f.end <= it->end && f.start != it->start)
                return true;
        }
        return false;
    }), context.functions.end());
    std::cout << "Heuristic function search completed. Found " << context.functions.size() << " functions." << std::endl;
    return *context.functions;
}