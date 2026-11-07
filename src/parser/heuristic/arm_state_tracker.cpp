#include "parser/heuristic/arm_state_tracker.hpp"
#include "capstone/arm.h"
#include <cstdint>

ArmStateTracker::ArmStateTracker(std::vector<uint8_t> memoryData, uint32_t baseAddress) : _memoryData(std::move(memoryData)), _baseAddress(baseAddress) {
    for (int i = ARM_REG_R0; i <= ARM_REG_R12; ++i)
        _registers[i] = std::nullopt;
    for (int i = ARM_REG_R14; i <= ARM_REG_R13; ++i)
        _registers[i] = std::nullopt;  
}

std::optional<uint32_t> ArmStateTracker::getRegisterValue(uint8_t reg) const {
    auto regIt = _registers.find(reg);
    if (regIt != _registers.end())
        return regIt->second;
    return std::nullopt;
}

void ArmStateTracker::setRegisterValue(uint8_t reg, uint32_t value) {
    _registers[reg] = value;
}

void ArmStateTracker::setRegisterValue(uint8_t reg, std::optional<uint32_t> value) {
    _registers[reg] = value;
}

void ArmStateTracker::updateNZ(uint32_t result) {
    _flags.N = (result & 0x80000000) != 0;
    _flags.Z = (result == 0);
}

void ArmStateTracker::updateLogicalFlags(uint32_t result, std::optional<bool> shifterCarry) {
    updateNZ(result);
    if (shifterCarry.has_value())
        _flags.C = shifterCarry.value();
}

void ArmStateTracker::updateArithmeticFlags(uint32_t a, uint32_t b, uint32_t result, OperationType op) {
    updateNZ(result);
    if (op == OperationType::ADD) {
        uint64_t wideResult = static_cast<uint64_t>(a) + static_cast<uint64_t>(b);
        _flags.C = (wideResult >> 32) != 0;
        _flags.V = (~(a ^ b) & (a ^ result) & 0x80000000u) != 0;
    }
    else if (op == OperationType::SUB) {
        _flags.C = a >= b;
        _flags.V = ((a ^ b) & (a ^ result) & 0x80000000u) != 0;
    }
}

void ArmStateTracker::invalidateWrittenRegisters(csh handle, const cs_insn& insn) {
    cs_regs regs_read;
    cs_regs regs_write;
    uint8_t regs_read_count = 0;
    uint8_t regs_write_count = 0;
    if (cs_regs_access(handle, &insn, regs_read, &regs_read_count, regs_write, &regs_write_count) != CS_ERR_OK)
        return;
    for (uint8_t i = 0; i < regs_write_count; ++i)
        setRegisterValue(regs_write[i], std::nullopt);
}

std::optional<ShiftResult> ArmStateTracker::applyShift(uint32_t value, arm_shifter shift, uint32_t amount) const {
    ShiftResult result {value, std::nullopt};
    switch (shift) {
        case ARM_SFT_INVALID:
            return result;
        case ARM_SFT_LSL:
            if (amount == 0)
                return result;
            if (amount < 32) {
                result.value = value << amount;
                result.carry = ((value >> (32 - amount)) & 1) != 0;
            } else if (amount == 32) {
                result.value = 0;
                result.carry = (value & 1) != 0;
            } else {
                result.value = 0;
                result.carry = false;
            }
            return result;
        case ARM_SFT_LSR:
            if (amount == 0)
                amount = 32;
            if (amount < 32) {
                result.value = value >> amount;
                result.carry = ((value >> (amount - 1)) & 1) != 0;
            } else {
                result.value = 0;
                result.carry = ((value >> 31) & 1) != 0;
            }
            return result;

        case ARM_SFT_ASR:
            if (amount == 0)
                amount = 32;
            if (amount >= 32) {
                bool sign = (value & 0x80000000u) != 0;
                result.value = sign ? 0xFFFFFFFFu : 0;
                result.carry = sign;
            } else {
                result.value = static_cast<uint32_t>(static_cast<int32_t>(value) >> amount);
                result.carry = ((value >> (amount - 1)) & 1) != 0;
            }
            return result;
        case ARM_SFT_ROR:
            if (amount == 0)
                return applyShift(value, ARM_SFT_RRX, 0);
            amount %= 32;
            if (amount == 0) {
                result.value = value;
                result.carry = (value >> 31) != 0;
            } else {
                result.value =
                    (value >> amount) | (value << (32 - amount));
                result.carry = (result.value >> 31) != 0;
            }
            return result;

        case ARM_SFT_RRX:
            if (!_flags.C.has_value())
                return std::nullopt;
            result.value = (static_cast<uint32_t>(*_flags.C) << 31) | (value >> 1);
            result.carry = (value & 1) != 0;
            return result;
        default:
            return std::nullopt;
    }
}

std::optional<ShiftResult> ArmStateTracker::getOperandValue(const cs_arm_op& operand) const {
    uint32_t value;
    switch (operand.type) {
        case ARM_OP_REG: {
            std::optional<uint32_t> regValue = getRegisterValue(operand.reg);
            if (!regValue.has_value())
                return std::nullopt;
            value = regValue.value();
            break;
        }
        case ARM_OP_IMM:
            value = static_cast<uint32_t>(operand.imm);
            break;
        default:
            return std::nullopt;
    }
    if (operand.shift.type == ARM_SFT_INVALID)
        return ShiftResult{value, std::nullopt};
    return applyShift(value, operand.shift.type, operand.shift.value);
}

void ArmStateTracker::invalidateFlags() {
    _flags.N = std::nullopt;
    _flags.Z = std::nullopt;
    _flags.C = std::nullopt;
    _flags.V = std::nullopt;
}


std::optional<uint32_t> ArmStateTracker::getMemoryValue(uint32_t address) const {
    auto it = _memory.find(address);
    if (it == _memory.end())
        return std::nullopt;
    return it->second;
}

void ArmStateTracker::setMemoryValue(uint32_t address, uint32_t value) {
    _memory[address] = value;
    cs_arm_op a;
}

std::optional<uint32_t> ArmStateTracker::readMemory32(uint32_t address) const {
    std::optional<uint32_t> value = getMemoryValue(address);
    if (value.has_value())
        return value.value();
    if (address < _baseAddress)
        return std::nullopt;
    uint32_t offset = address - _baseAddress;
    if (offset > _memoryData.size() || _memoryData.size() - offset < 4)
        return std::nullopt;
    uint32_t result = static_cast<uint32_t>(_memoryData[offset])
                      |(static_cast<uint32_t>(_memoryData[offset + 1]) << 8)
                      |(static_cast<uint32_t>(_memoryData[offset + 2]) << 16)
                      |(static_cast<uint32_t>(_memoryData[offset + 3]) << 24);
    return result;
}

std::optional<uint32_t> ArmStateTracker::readMemory(uint32_t address, uint8_t size) const {
    if (size != 1 && size != 2 && size != 4)
        return std::nullopt;
    if (size == 4)
        return readMemory32(address);
    if (address < _baseAddress)
        return std::nullopt;
    uint32_t offset = address - _baseAddress;
    if (offset > _memoryData.size() ||
        _memoryData.size() - offset < size)
        return std::nullopt;
    uint32_t result = 0;
    for (uint8_t i = 0; i < size; ++i)
        result |= static_cast<uint32_t>(_memoryData[offset + i]) << (8 * i);
    return result;
}

std::optional<bool> ArmStateTracker::evaluateCondition(arm_cc condition) const {
    std::optional<bool> N = _flags.N;
    std::optional<bool> Z = _flags.Z;
    std::optional<bool> C = _flags.C;
    std::optional<bool> V = _flags.V;
    switch (condition) {
        case ARM_CC_INVALID:
        case ARM_CC_AL:
            return true;
        case ARM_CC_EQ:
            return Z;
        case ARM_CC_NE:
            return Z.has_value() ? std::optional<bool>(!Z.value()) : std::nullopt;
        case ARM_CC_HS:
            return C;
        case ARM_CC_LO:
            return C.has_value() ? std::optional<bool>(!C.value()) : std::nullopt;
        case ARM_CC_MI:
            return N;
        case ARM_CC_PL:
            return N.has_value() ? std::optional<bool>(!N.value()) : std::nullopt;
        case ARM_CC_VS:
            return V;
        case ARM_CC_VC:
            return V.has_value() ? std::optional<bool>(!V.value()) : std::nullopt;
        case ARM_CC_HI:
            if (!C.has_value() || !Z.has_value())
                return std::nullopt;
            return C.value() && !Z.value();
        case ARM_CC_LS:
            if (!C.has_value() || !Z.has_value())
                return std::nullopt;
            return !C.value() || Z.value();
        case ARM_CC_GE:
            if (!N.has_value() || !V.has_value())
                return std::nullopt;
            return N.value() == V.value();
        case ARM_CC_LT:
            if (!N.has_value() || !V.has_value())
                return std::nullopt;
            return N.value() != V.value();
        case ARM_CC_GT:
            if (!Z.has_value() || !N.has_value() || !V.has_value())
                return std::nullopt;
            return !Z.value() && N.value() == V.value();
        case ARM_CC_LE:
            if (!Z.has_value() || !N.has_value() || !V.has_value())
                return std::nullopt;
            return Z.value() || N.value() != V.value();
        default:
            return std::nullopt;
    }
}

std::optional<uint32_t> ArmStateTracker::getMemoryAddress(const cs_arm_op& operand) const {
    if (operand.type != ARM_OP_MEM)
        return std::nullopt;
    const arm_op_mem& mem = operand.mem;
    std::optional<uint32_t> base = getRegisterValue(mem.base);
    if (!base.has_value())
        return std::nullopt;
    uint32_t offset = 0;
    if (mem.index != ARM_REG_INVALID) {
        std::optional<uint32_t> index = getRegisterValue(mem.index);
        if (!index.has_value())
            return std::nullopt;
        uint32_t shiftedIndex = index.value();
        if (mem.lshift != 0)
            shiftedIndex <<= mem.lshift;
        offset = static_cast<uint32_t>(mem.scale) * shiftedIndex;
    }
    offset += static_cast<uint32_t>(mem.disp);
    uint32_t address = base.value();
    if (operand.subtracted)
        address -= offset;
    else
        address += offset;
    return address;
}

void ArmStateTracker::handleInstruction(const cs_insn& insn, csh handle) {
    if (!insn.detail)
        return;
    const cs_arm& arm = insn.detail->arm;
    const cs_arm_op& dst = arm.operands[0];
    if (arm.cc != ARM_CC_AL && arm.cc != ARM_CC_INVALID) {
        std::optional<bool> condition = evaluateCondition(arm.cc);
        if (condition == false)
            return;
        if (condition == std::nullopt) {
            invalidateWrittenRegisters(handle, insn);
            if (arm.update_flags)
                invalidateFlags();
            return;
        }
    }
    switch (insn.id) {
        case ARM_INS_MOV:
        case ARM_INS_MVN: {
            if (arm.op_count < 2 || dst.type != ARM_OP_REG) {
                invalidateWrittenRegisters(handle, insn);
                if (arm.update_flags)
                    invalidateFlags();
                break;
            }
            std::optional<ShiftResult> operand = getOperandValue(arm.operands[1]);
            if (!operand.has_value()) {
                setRegisterValue(dst.reg, std::nullopt);
                if (arm.update_flags) {
                    _flags.N = std::nullopt;
                    _flags.Z = std::nullopt;
                    _flags.C = std::nullopt;
                }
                break;
            }
            uint32_t result = insn.id == ARM_INS_MOV ? operand->value : ~operand->value;
            setRegisterValue(dst.reg, result);
            if (arm.update_flags)
                updateLogicalFlags(result, operand->carry);
            break;
        }
        case ARM_INS_ADD:
        case ARM_INS_SUB: {
            if (arm.op_count < 3 || dst.type != ARM_OP_REG)
                break;
            std::optional<ShiftResult> a = getOperandValue(arm.operands[1]);
            std::optional<ShiftResult> b = getOperandValue(arm.operands[2]);
            if (!a.has_value() || !b.has_value()) {
                setRegisterValue(dst.reg, std::nullopt);
                if (arm.update_flags)
                    invalidateFlags();
                break;
            }
            uint32_t result;
            OperationType operation;
            if (insn.id == ARM_INS_ADD) {
                result = a->value + b->value;
                operation = OperationType::ADD;
            } else {
                result = a->value - b->value;
                operation = OperationType::SUB;
            }
            setRegisterValue(dst.reg, result);
            if (arm.update_flags)
                updateArithmeticFlags(a->value, b->value, result, operation);
            break;
        }
        case ARM_INS_CMP:
        case ARM_INS_CMN: {
            if (arm.op_count < 2) {
                invalidateFlags();
                break;
            }
            std::optional<ShiftResult> a = getOperandValue(arm.operands[0]);
            std::optional<ShiftResult> b = getOperandValue(arm.operands[1]);
            if (!a.has_value() || !b.has_value()) {
                invalidateFlags();
                break;
            }
            uint32_t result;
            OperationType operation;
            if (insn.id == ARM_INS_CMP) {
                result = a->value - b->value;
                operation = OperationType::SUB;
            } else {
                result = a->value + b->value;
                operation = OperationType::ADD;
            }
            updateArithmeticFlags(a->value, b->value, result, operation);
            break;
        }
        case ARM_INS_TST:
        case ARM_INS_TEQ: {
            if (arm.op_count < 2) {
                _flags.N = std::nullopt;
                _flags.Z = std::nullopt;
                _flags.C = std::nullopt;
                break;
            }
            std::optional<ShiftResult> a = getOperandValue(arm.operands[0]);
            std::optional<ShiftResult> b = getOperandValue(arm.operands[1]);
            if (!a.has_value() || !b.has_value()) {
                _flags.N = std::nullopt;
                _flags.Z = std::nullopt;
                _flags.C = std::nullopt;
                break;
            }
            uint32_t result = insn.id == ARM_INS_TST ? (a->value & b->value) :(a->value ^ b->value);
            updateLogicalFlags(result, b->carry);
            break;
        }
        case ARM_INS_AND:
        case ARM_INS_ORR:
        case ARM_INS_EOR:
        case ARM_INS_BIC: {
            if (arm.op_count < 3 || dst.type != ARM_OP_REG)
                break;
            std::optional<ShiftResult> a = getOperandValue(arm.operands[1]);
            std::optional<ShiftResult> b = getOperandValue(arm.operands[2]);
            if (!a.has_value() || !b.has_value()) {
                setRegisterValue(dst.reg, std::nullopt);
                if (arm.update_flags) {
                    _flags.N = std::nullopt;
                    _flags.Z = std::nullopt;
                    _flags.C = std::nullopt;
                }
                break;
            }
            uint32_t result;
            switch (insn.id) {
                case ARM_INS_AND:
                    result = a->value & b->value;
                    break;
                case ARM_INS_ORR:
                    result = a->value | b->value;
                    break;
                case ARM_INS_EOR:
                    result = a->value ^ b->value;
                    break;
                case ARM_INS_BIC:
                    result = a->value & ~b->value;
                    break;
                default:
                    break;
            }
            setRegisterValue(dst.reg, result);
            if (arm.update_flags)
                updateLogicalFlags(result, b->carry);
            break;
        }
        case ARM_INS_MOVW: {
            if (arm.op_count < 2 || dst.type != ARM_OP_REG ||
                arm.operands[1].type != ARM_OP_IMM) {
                invalidateWrittenRegisters(handle, insn);
                break;
            }
            uint32_t result = static_cast<uint32_t>(arm.operands[1].imm) & 0xFFFF;
            setRegisterValue(dst.reg, result);
            break;
        }
        case ARM_INS_MOVT: {
            if (arm.op_count < 2 || dst.type != ARM_OP_REG ||
                arm.operands[1].type != ARM_OP_IMM) {
                invalidateWrittenRegisters(handle, insn);
                break;
            }
            std::optional<uint32_t> previous = getRegisterValue(dst.reg);
            if (!previous.has_value()) {
                setRegisterValue(dst.reg, std::nullopt);
                break;
            }
            uint32_t result = (static_cast<uint32_t>(arm.operands[1].imm) & 0xFFFF) << 16;
            result |= previous.value() & 0xFFFF;
            setRegisterValue(dst.reg, result);
            break;
        }
        case ARM_INS_ADR: {
            if (arm.op_count < 2 || dst.type != ARM_OP_REG ||
                arm.operands[1].type != ARM_OP_IMM) {
                invalidateWrittenRegisters(handle, insn);
                break;
            }

            setRegisterValue(dst.reg, static_cast<uint32_t>(arm.operands[1].imm));
            break;
        }
        case ARM_INS_LDR: {
            if (arm.op_count < 2 || dst.type != ARM_OP_REG ||
                arm.operands[1].type != ARM_OP_MEM) {
                invalidateWrittenRegisters(handle, insn);
                break;
            }
            std::optional<uint32_t> address = getMemoryAddress(arm.operands[1]);
            if (!address.has_value()) {
                setRegisterValue(dst.reg, std::nullopt);
                break;
            }
            std::optional<uint32_t> value = readMemory32(address.value());
            setRegisterValue(dst.reg, value);
            break;
        }
        default:
            invalidateWrittenRegisters(handle, insn);
            if (arm.update_flags)
                invalidateFlags();
            break;
    }
}