#include "output/scatter/input_section.hpp"

InputSection::InputSection(const std::string &selectPattern, std::initializer_list<std::string_view> attributes) : _selectPattern(selectPattern) {
    for (const auto& attribute : attributes)
        _attributes.push_back(std::string(attribute));
}

void InputSection::write(std::ofstream& output) const {
    output << _selectPattern << "(";
    for (size_t i = 0; i < _attributes.size(); ++i) {
        output << _attributes[i];
        if (i < _attributes.size() - 1)
            output << ", ";
    }
    output << ")\n";
}

void InputSection::addAttribute(const std::string &attribute) {
    _attributes.push_back(attribute);
}

const std::vector<std::string>& InputSection::getAttributes() const {
    return _attributes;
}