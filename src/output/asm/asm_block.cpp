#include "output/asm/asm_block.hpp"
#include <iomanip>

ASMBlock::ASMBlock(uint8_t *data, size_t size, const std::string &name, SplitSection::Type type, bool isThumb, bool isFunction, bool entry) : _data(data), _size(size), _name(name), _type(type), _isThumb(isThumb), _isFunction(isFunction), _entry(entry) {}

const std::string& ASMBlock::getName() const {
    return _name;
}

const uint8_t *ASMBlock::getData() const {
    return _data;
}

bool ASMBlock::isThumb() const {
    return _isThumb;
}

bool ASMBlock::isFunction() const {
    return _isFunction;
}

#include <iostream>

void ASMBlock::write(std::ofstream& output) const {
    if (_type != SplitSection::Type::CODE) {
        output << "\n\tAREA"; 
        if (_type == SplitSection::Type::RODATA)
            output << "|rodata." << _name << "|, DATA, READONLY\n";
        else 
            output << "|data." << _name << "|, DATA, READWRITE\n";
        output << "\n";
        for (size_t i = 0; i < _size; ++i) {
            if (i % 8 == 0)
                output << (i == 0 ? "" : "\n") << "\tDCB 0x" << std::hex << static_cast<int>(_data[i]);
            else
                output << ", 0x" << std::hex << static_cast<int>(_data[i]);
        }
        output << "\n";
    } else {
        std::string prefix = _isFunction ? "func_" : "loc_";
        output << "\n\tAREA " << "|text." << prefix <<  _name << "|, CODE, READONLY\n";
        if (_isThumb)
            output << "\tTHUMB\n";
        else
            output << "\tARM\n";
        if (_entry)
            output << "\tENTRY\n";
        if (_isFunction) {
            output << "\tEXPORT func_" << _name << "\n\n";
            output << "func_" << _name << " PROC\n";
        }
        int instructionSize = _isThumb ? 2 : 4;
        int realSize = _size - (_size % instructionSize);
        int hexWidth = instructionSize * 2;
        for (size_t i = 0; i < realSize; i += instructionSize)
            output << "\t" << "DCI " << " 0x" << std::hex << std::setfill('0') << std::setw(hexWidth) << (_isThumb ? *reinterpret_cast<uint16_t*>(&_data[i]) : *reinterpret_cast<uint32_t*>(&_data[i])) << "\n";
        if (_isFunction)
            output << "\tENDP\n";
        for (size_t i = realSize; i < _size; ++i)
            output << "\tDCB 0x" << std::hex << static_cast<int>(_data[i]) << "\n";
    }
}


