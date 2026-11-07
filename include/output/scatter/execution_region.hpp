#ifndef EXECUTION_REGION_HPP_
#define EXECUTION_REGION_HPP_

#include "output/scatter/input_section.hpp"
#include <string>
#include <map>
#include <utility>
#include <variant>
#include <vector>

class ExecutionRegion : public Writable {

    using Value = std::variant<uint32_t, int32_t, double, std::string>;

    public:

        enum class Attribute : uint8_t {
            NONE = 0,
            ABSOLUTE = 1,
            ALIGN = 2,
            ALIGNALL = 3,
            ANY_SIZE = 4,
            EMPTY = 5,
            FILL = 6,
            FIXED = 7,
            NO_COMPRESS = 8,
            OVERLAY = 9,
            PAD_VALUE = 10,
            PI = 11,
            SORTTYPE = 12,
            UNINIT = 13,
            ZEROPAD = 14
        };

        ExecutionRegion(const std::string &regionName, bool isAbsolute, uint32_t baseAddress, uint32_t maxSize = 0);
        void write(std::ofstream& output) const override;
        static std::string getAttributeName(Attribute attribute);
        void addAttribute(Attribute attribute);
        void addAttribute(Attribute attribute, Value value);
        void addInputSection(const InputSection& inputSection);
    private:
        static constexpr std::string_view ATTRIBUTE_NAMES[] = {
            "",
            "ABSOLUTE",
            "ALIGN",
            "ALIGNALL",
            "ANY_SIZE",
            "EMPTY",
            "FILL",
            "FIXED",
            "NOCOMPRESS",
            "OVERLAY",
            "PADVALUE",
            "PI",
            "SORTTYPE",
            "UNINIT",
            "ZEROPAD"
        };
        std::string _regionName;
        bool _isAbsolute;
        uint32_t _baseAddress;
        uint32_t _maxSize;
        std::map<Attribute, std::pair<bool, Value>> _attributes;
        std::vector<InputSection> _inputs;
};

#endif /* !EXECUTION_REGION_HPP_ */
