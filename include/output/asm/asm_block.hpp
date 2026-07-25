#ifndef ASM_BLOCK_HPP_
#define ASM_BLOCK_HPP_

#include "../writable.hpp"

#include <cstdint>

class ASMBlock : public Writable {
    public:
        ASMBlock(uint8_t *data, size_t size, const std::string &name = "", bool isThumb = true);
        ~ASMBlock() = default;
        const std::string& getName() const;
        const uint8_t *getData() const;
        bool isThumb() const;
        void write(std::ofstream& output) const override;
    protected:
    private:
        std::string _name;
        uint8_t *_data;
        size_t _size;
        bool _isThumb;
};


#endif /* !ASM_BLOCK_HPP_ */
