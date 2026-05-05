#include "parser/arm_parser.hpp"
#include "capstone/capstone.h"
#include <memory>
#include <stdexcept>

ArmParser::ArmParser(const std::vector<uint8_t>& data, uint32_t baseAddress, cs_mode mode) : _data(data), _mode(mode), _baseAddress(baseAddress), _offset(0), _lastInstruction(nullptr), _nbInstructions(0)
{
    create(mode);
}

ArmParser::ArmParser(const ArmParser& other) : _data(other._data), _mode(other._mode), _baseAddress(other._baseAddress), _offset(other._offset), _lastInstruction(nullptr), _nbInstructions(other._nbInstructions)
{
    other.close();
    create(_mode);
}

void ArmParser::close() const
{
    if (_lastInstruction)
        cs_free(_lastInstruction, _nbInstructions);
    cs_close(const_cast<csh*>(&_handle));
}

void ArmParser::create(cs_mode mode)
{
    cs_open(CS_ARCH_ARM, mode, &_handle);
    cs_option(_handle, CS_OPT_DETAIL, CS_OPT_ON);
}

void ArmParser::changeMode(cs_mode mode)
{
    cs_close(&_handle);
    _mode = mode;
    create(mode);
}

cs_insn* ArmParser::getNextInstruction()
{
    cs_insn *instruction;
    size_t count = cs_disasm(_handle, _data.data() + _offset, _data.size() - _offset, _baseAddress + _offset, 1, &instruction);
    if (_lastInstruction)
        cs_free(_lastInstruction, 1);
    _nbInstructions = count;
    _lastInstruction = count > 0 ? instruction : nullptr;
    if (count > 0) {
        _offset += instruction->size;
        return instruction;
    }
    _offset += _mode == CS_MODE_ARM ? 4 : 2;
    return nullptr;
}

cs_insn* ArmParser::getNextInstructions(size_t count, size_t* outCount)
{
    cs_insn *instructions;
    size_t actualCount = cs_disasm(_handle, _data.data() + _offset, _data.size() - _offset, _baseAddress + _offset, count, &instructions);
    if (_lastInstruction)
        cs_free(_lastInstruction, _nbInstructions);
    _nbInstructions = actualCount;
    _lastInstruction = actualCount > 0 ? instructions : nullptr;
    if (outCount)
        *outCount = actualCount;
    if (actualCount > 0) {
        cs_insn& last = instructions[actualCount - 1];
        _offset = (last.address + last.size) - _baseAddress;
        return instructions;
    }
    _offset += _mode == CS_MODE_ARM ? 4 : 2;
    return nullptr;
}

ArmParser::~ArmParser()
{
    close();
}

bool ArmParser::isEnd() const
{
    return _offset >= _data.size();
}

void ArmParser::goToAddress(uint32_t address)
{
    if (address < _baseAddress || address >= _baseAddress + _data.size())
        throw std::runtime_error("Address out of bounds");
    _offset = address - _baseAddress;
}

void ArmParser::switchMode()
{
    _mode = _mode == CS_MODE_ARM ? CS_MODE_THUMB : CS_MODE_ARM;
    changeMode(_mode);
}

uint32_t ArmParser::getBaseAddress() const
{
    return _baseAddress;
}

size_t ArmParser::getDataSize() const
{
    return _data.size();
}