#ifndef SPLIT_SECTION_HPP_
#define SPLIT_SECTION_HPP_

#include "input/readable.hpp"
#include <string>
#include <cstdint>
#include <string_view>
#include <unordered_map>

class SplitSection : public Readable {
    public:
        enum class Type: uint8_t {
            CODE,
            RODATA,
            DATA,
            BSS
        };
        SplitSection() = default;
        ~SplitSection() = default;
        void read(std::ifstream& input, const std::string &line = "") override;
        const std::string &getName() const;
        Type getType() const;
        uint32_t getStart() const;
        uint32_t getEnd() const;
        static Type stringToType(const std::string& typeStr);
        void setName(const std::string& name);
        void setType(Type type);
        void setStart(uint32_t start);
        void setEnd(uint32_t end);
    private:
        std::string _name;
        Type _type;
        uint32_t _start;
        uint32_t _end;
        inline static const std::unordered_map<std::string_view, Type> STRING_TO_TYPE = {
            {"code", Type::CODE},
            {"rodata", Type::RODATA},
            {"data", Type::DATA},
            {"bss", Type::BSS}
        };
};

#endif /* !SPLIT_SECTION_HPP_ */