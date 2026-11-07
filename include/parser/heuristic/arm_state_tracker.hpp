#ifndef ARM_STATE_TRACKER_HPP_
#define ARM_STATE_TRACKER_HPP_

#include "capstone/arm.h"
#include <cstdint>
#include <capstone/capstone.h>
#include <unordered_map>
#include <optional>

struct CPSRFlags {
    std::optional<bool> N; // Negative
    std::optional<bool> Z; // Zero
    std::optional<bool> C; // Carry
    std::optional<bool> V; // Overflow
}; 

enum class OperationType : uint8_t {
    ADD,
    SUB
};

struct ShiftResult {
    uint32_t value;
    std::optional<bool> carry;
};

class ArmStateTracker {
    public:
        ArmStateTracker(std::vector<uint8_t> memoryData, uint32_t baseAddress);
        std::optional<uint32_t> readMemory32(uint32_t address) const;
        std::optional<uint32_t> readMemory(uint32_t address, uint8_t size) const;
        std::optional<uint32_t> getRegisterValue(uint8_t reg) const;
        void setRegisterValue(uint8_t reg, uint32_t value);
        void setRegisterValue(uint8_t reg, std::optional<uint32_t> value);
        void handleInstruction(const cs_insn& insn, csh handle);
        std::optional<uint32_t> getMemoryValue(uint32_t address) const;
        void setMemoryValue(uint32_t address, uint32_t value);
    private:
        void invalidateFlags();
        std::optional<uint32_t> getMemoryAddress(const cs_arm_op& operand) const;
        void invalidateWrittenRegisters(csh handle, const cs_insn& insn);
        std::optional<bool> evaluateCondition(arm_cc condition) const;
        std::optional<ShiftResult> getOperandValue(const cs_arm_op& operand) const;
        void updateNZ(uint32_t result);
        void updateLogicalFlags(uint32_t result, std::optional<bool> shifterCarry);
        void updateArithmeticFlags(uint32_t a, uint32_t b, uint32_t result, OperationType op);
        std::optional<ShiftResult> applyShift(uint32_t value, arm_shifter shift, uint32_t amount) const;
        std::unordered_map<uint8_t, std::optional<uint32_t>> _registers;
        CPSRFlags _flags;
        std::unordered_map<uint32_t, uint32_t> _memory;
        std::vector<uint8_t> _memoryData;
        uint32_t _baseAddress;
};

#endif /* !ARM_STATE_TRACKER_HPP_ */