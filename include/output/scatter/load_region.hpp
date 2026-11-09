#ifndef LOAD_REGION_HPP_
#define LOAD_REGION_HPP_

#include "output/scatter/execution_region.hpp"

class LoadRegion : public Writable {

    using Value = std::variant<uint32_t, int32_t, double, std::string>;

    public:
        enum class Attribute : uint8_t {
            NONE = 0,
            ABSOLUTE = 1,
            ALIGN = 2,
            NO_COMPRESS = 3,
            OVERLAP = 4,
            PI = 5,
            PROTECTED = 6,
            RELOCATABLE = 7
        };

        LoadRegion(const std::string &regionName, bool isAbsolute, uint32_t baseAddress, uint32_t maxSize = 0);
        void write(std::ofstream& output) const override;
        void addAttribute(Attribute attribute);
        void addAttribute(Attribute attribute, Value value);
        static std::string getAttributeName(Attribute attribute);
        void addExecutionRegion(const ExecutionRegion& executionRegion);
        ExecutionRegion& getNearestExecutionRegion(uint32_t address);
        std::vector<ExecutionRegion>& getExecutionRegions();
    private:
        static constexpr std::string ATTRIBUTE_NAMES[] = {
            "",
            "ABSOLUTE",
            "ALIGN",
            "NOCOMPRESS",
            "OVERLAY",
            "PI",
            "PROTECTED",
            "RELOC"
        };
        std::string _regionName;
        bool _isAbsolute;
        uint32_t _baseAddress;
        uint32_t _maxSize;
        std::map<Attribute, std::pair<bool, Value>> _attributes;
        std::vector<ExecutionRegion> _executionRegions;
};

#endif /* !LOAD_REGION_HPP_ */
