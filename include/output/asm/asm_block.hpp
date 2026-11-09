#ifndef ASM_BLOCK_HPP_
#define ASM_BLOCK_HPP_

#include "../writable.hpp"

#include <cstdint>
#include "input/split/split_section.hpp"

class ASMBlock : public Writable {
    public:
        ASMBlock(uint8_t *data, size_t size, const std::string &name = "", SplitSection::Type type = SplitSection::Type::CODE, bool isThumb = true, bool isFunction = true, bool entry = false);
        ~ASMBlock() = default;
        const std::string& getName() const;
        const uint8_t *getData() const;
        bool isThumb() const;
        bool isFunction() const;
        void write(std::ofstream& output) const override;
    protected:
    private:
        std::string _name;
        SplitSection::Type _type;
        uint8_t *_data;
        size_t _size;
        bool _isThumb;
        bool _isFunction;
        bool _entry;
};


#endif /* !ASM_BLOCK_HPP_ */
