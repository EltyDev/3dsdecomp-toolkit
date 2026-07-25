#include "output/ldscript/ldsection.hpp"
#include <cstdint>

LDSection::LDSection(uint32_t address, const std::string &name, size_t size) : _address(address), _name(name), _size(size) {}

std::vector<std::string> &LDSection::getFiles() {
    return _files;
}

const std::string& LDSection::getName() const {
    return _name;
}

uint32_t LDSection::getBaseAddress() const {
    return _address;
}

size_t LDSection::getSize() const {
    return _size;
}

void LDSection::addFile(const std::string &file, uint32_t address) {
    _files.push_back(file);
    _addresses.push_back(address);
}

void  LDSection::write(std::ofstream& output) const {
    output << ". = 0x" << std::hex << _address << ";\n";
    output << "." << _name << " :\n";
    output << "{\n";
    for (size_t i = 0; i < _files.size(); ++i) {
        const auto& file = _files[i];
        uint32_t address = _addresses[i];
        if (address == 0)
            output << "\t" << file << ".o(." << _name << ")\n";
        else {
            output << "\t. = 0x" << std::hex << address << ";\n";
            output << "\t" << file << ".o(." << _name << ")\n";
        }
    }
    if (_size > 0)
        output << "\t. = . + 0x" << std::hex << _size << ";\n";
    output << "}\n";
}