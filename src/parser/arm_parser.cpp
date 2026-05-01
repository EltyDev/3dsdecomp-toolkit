#include "parser/arm_parser.hpp"
#include "capstone/capstone.h"
#include <memory>

ArmParser::ArmParser(const std::vector<uint8_t>& data, cs_mode mode) : _data(data), _mode(mode), _offset(0)
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

std::unique_ptr<cs_insn> ArmParser::getNextInstruction()
{
    cs_insn *instruction;
    size_t count = cs_disasm(_handle, _data.data() + _offset, _data.size() - _offset, 0x1000 + _offset, 1, &instruction);
    if (count > 0) {
        _offset += instruction->size;
        return std::unique_ptr<cs_insn>(instruction);
    }
    return nullptr;
}

ArmParser::~ArmParser()
{
    cs_close(&_handle);
}

