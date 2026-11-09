#include "output/asm/asm_file.hpp"

ASMFile::ASMFile() {}

void ASMFile::write(std::ofstream& output) const {
    for (const auto& block : _blocks) {
        block.write(output);
        output << "\n";
    }
    output << "\tEND";
}

void ASMFile::addBlock(const ASMBlock &block) {
    _blocks.push_back(block);
}

void ASMFile::reset() {
    _blocks.clear();
}