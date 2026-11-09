#ifndef ASM_FILE_HPP_
#define ASM_FILE_HPP_

#include "asm_block.hpp"
#include <vector>

class ASMFile : public Writable {
    public:
        ASMFile();
        ~ASMFile() = default;
        void write(std::ofstream& output) const override;
        void addBlock(const ASMBlock &block);
        void reset();
    protected:
    private:
        std::vector<ASMBlock> _blocks;
};

#endif /* !ASM_FILE_HPP_ */

