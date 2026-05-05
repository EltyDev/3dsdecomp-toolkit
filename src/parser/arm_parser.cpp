#include "parser/arm_parser.hpp"
#include "capstone/capstone.h"
#include <memory>
#include <stdexcept>

ArmParser::ArmParser(const std::vector<uint8_t>& data, uint32_t baseAddress, cs_mode mode) : _data(data), _mode(mode), _baseAddress(baseAddress), _offset(0), _lastInstruction(nullptr)
{
    create(mode);
}

void ArmParser::create(cs_mode mode)
{
    cs_open(CS_ARCH_ARM, mode, &_handle);
    cs_option(_handle, CS_OPT_DETAIL, CS_OPT_ON);
}

void ArmParser::changeMode(cs_mode mode)
{
    cs_close(&_handle);
    create(mode);
}

cs_insn* ArmParser::getNextInstruction()
{
    cs_insn *instruction;
    size_t count = cs_disasm(_handle, _data.data() + _offset, _data.size() - _offset, _baseAddress + _offset, 1, &instruction);
    if (_lastInstruction)
        cs_free(_lastInstruction, 1);
    _lastInstruction = count > 0 ? instruction : nullptr;
    if (count > 0) {
        _offset += instruction->size;
        return instruction;
    }
    _offset += _mode == CS_MODE_ARM ? 4 : 2;
    return nullptr;
}

ArmParser::~ArmParser()
{
    if (_lastInstruction)
        cs_free(_lastInstruction, 1);
    cs_close(&_handle);
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

