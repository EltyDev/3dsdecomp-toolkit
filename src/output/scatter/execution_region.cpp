#include "output/scatter/execution_region.hpp"
#include "algorithm"

ExecutionRegion::ExecutionRegion(const std::string &regionName, bool isAbsolute, uint32_t baseAddress, uint32_t maxSize)
    : _regionName(regionName), _isAbsolute(isAbsolute), _baseAddress(baseAddress), _maxSize(maxSize) {}


void ExecutionRegion::write(std::ofstream& output) const {
    output << _regionName << " ";
    if (_isAbsolute)
        output << "0x" << std::hex << _baseAddress << " ";
    else
        output << "+" << _baseAddress << " ";
    for (const auto& [attribute, value] : _attributes) {
        output << getAttributeName(attribute);
        if (value.first) {
            std::visit([&output](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t>)
                    output << "0x" << std::hex << arg;
                else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, std::string>)
                    output << arg;
            }, value.second);
        }
        output << " ";
    }
    if (_maxSize != 0)
        output << "0x" << std::hex << _maxSize;
    output << "\n{\n";
    for (const auto& input : _inputs)
        input.write(output);
    output << "}\n";
}

std::string ExecutionRegion::getAttributeName(Attribute attribute) {
    return ATTRIBUTE_NAMES[static_cast<uint8_t>(attribute)];
}

void ExecutionRegion::addAttribute(Attribute attribute, Value value) {
    _attributes[attribute] = std::make_pair(true, value);
}

void ExecutionRegion::addAttribute(Attribute attribute) {
    _attributes[attribute] = std::make_pair(false, Value{});
}

void ExecutionRegion::addInputSection(const InputSection& inputSection) {
    _inputs.push_back(inputSection);
}

const uint32_t ExecutionRegion::getBaseAddress() const {
    return _baseAddress;
}

#include <iostream>

void  ExecutionRegion::sortInputSectionsByStartAddress() {
    std::sort(_inputs.begin(), _inputs.end(), [](const InputSection& a, const InputSection& b) {
        std::vector<std::string> aAttributes = a.getAttributes();
        std::vector<std::string> bAttributes = b.getAttributes();
        if (aAttributes.empty() || bAttributes.empty())
            return false;
        if (aAttributes.size() > 1 || bAttributes.size() > 1)
            return false;
        std::string valueA = aAttributes[0].substr(aAttributes[0].find('_') + 1);
        std::string valueB = bAttributes[0].substr(bAttributes[0].find('_') + 1);
        uint32_t aStart = std::stoul(valueA, nullptr, 16);  
        uint32_t bStart = std::stoul(valueB, nullptr, 16);
        return aStart < bStart;
    });
}