#include "output/scatter/load_region.hpp"
#include "algorithm"

LoadRegion::LoadRegion(const std::string &regionName, bool isAbsolute, uint32_t baseAddress, uint32_t maxSize) :
    _regionName(regionName),
    _isAbsolute(isAbsolute),
    _baseAddress(baseAddress),
    _maxSize(maxSize) {}

void LoadRegion::write(std::ofstream& output) const {
    std::string upperRegionName = _regionName;
    std::transform(upperRegionName.begin(), upperRegionName.end(), upperRegionName.begin(), ::toupper);
    output << "LR_" << upperRegionName << " ";
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
    for (const auto& executionRegion : _executionRegions)
        executionRegion.write(output);
    output << "}\n";
}

void LoadRegion::addAttribute(Attribute attribute, Value value) {
    _attributes[attribute] = std::make_pair(true, value);
}

void LoadRegion::addAttribute(Attribute attribute) {
    _attributes[attribute] = std::make_pair(false, Value{});
}

std::string LoadRegion::getAttributeName(Attribute attribute) {
    return std::string(ATTRIBUTE_NAMES[static_cast<uint8_t>(attribute)]);
}

void LoadRegion::addExecutionRegion(const ExecutionRegion& executionRegion) {
    _executionRegions.push_back(executionRegion);
}