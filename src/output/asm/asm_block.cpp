#include "output/asm/asm_block.hpp"

ASMBlock::ASMBlock(uint8_t *data, size_t size, const std::string &name, bool isThumb) : _data(data), _size(size), _name(name), _isThumb(isThumb) {}

const std::string& ASMBlock::getName() const {
    return _name;
}

const uint8_t *ASMBlock::getData() const {
    return _data;
}

bool ASMBlock::isThumb() const {
    return _isThumb;
}

void ASMBlock::write(std::ofstream& output) const {
    if (_name == "") {
        for (size_t i = 0; i < _size; ++i) {
            if (i % 8 == 0)
                output << (i == 0 ? "" : "\n") << ".byte 0x" << std::hex << static_cast<int>(_data[i]);
            else
                output << ", 0x" << std::hex << static_cast<int>(_data[i]);
        }
        output << "\n";
    } else {
        if (_isThumb) {
            output << ".thumb\n\n";
            output << ".thumb_func\n";
        } else
            output << ".arm\n\n";
        output << ".global func_" << _name << "\n";
        output << ".type func_" << _name << ", %function\n";
        output << "func_" << _name << ":\n";
        int instructionSize = _isThumb ? 2 : 4;
        std::string prefix = _isThumb ? ".inst.n" : ".inst";
        for (size_t i = 0; i < _size; i += instructionSize) {
            output << "\t" << prefix << " 0x" << std::hex << (_isThumb ? *reinterpret_cast<uint16_t*>(&_data[i]) : *reinterpret_cast<uint32_t*>(&_data[i])) << "\n";
        }
    }
}


