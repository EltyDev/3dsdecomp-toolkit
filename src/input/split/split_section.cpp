#include "input/split/split_section.hpp"
#include <sstream>
#include <algorithm>

void SplitSection::read(std::ifstream& input, const std::string &line) {
    std::string actualLine = line;
    if (actualLine.empty())
        std::getline(input, actualLine);
    std::istringstream iss(actualLine);
    std::string word;
    bool missings[4] = {true, true, true, true};
    for (int i = 0; iss >> word; ++i) {
        if (i == 0) {
            missings[0] = false;
            _name = word;
            continue;
        }
        std::string key = word.substr(0, word.find(':'));
        std::string value = word.substr(word.find(':') + 1);
        if (key == "type") {
            _type = stringToType(value);
            missings[1] = false;
        } else if (key == "start") {
            _start = std::stoul(value, nullptr, 0);
            missings[2] = false;
        } else if (key == "end") {
            _end = std::stoul(value, nullptr, 0);
            missings[3] = false;
        }
    }
    std:: ostringstream missingFields;
    if (missings[0])
        missingFields << "name ";
    if (missings[1])
        missingFields << "type ";
    if (missings[2])
        missingFields << "start ";
    if (missings[3])
        missingFields << "end ";
    if (missingFields.str().length() > 0)
        throw std::runtime_error("Missing fields in split section: " + missingFields.str());
}

SplitSection::Type SplitSection::stringToType(const std::string& typeStr) {
    std::string lowerTypeStr = typeStr;
    std::transform(lowerTypeStr.begin(), lowerTypeStr.end(), lowerTypeStr.begin(), ::tolower);
    auto it = STRING_TO_TYPE.find(lowerTypeStr);
    if (it != STRING_TO_TYPE.end())
        return it->second;
    throw std::invalid_argument("Invalid section type: " + typeStr);
}

const std::string &SplitSection::getName() const {
    return _name;
}

SplitSection::Type SplitSection::getType() const {
    return _type;
}

uint32_t SplitSection::getStart() const {
    return _start;
}

uint32_t SplitSection::getEnd() const {
    return _end;
}

void SplitSection::setType(Type type) {
    _type = type;
}

void SplitSection::setName(const std::string& name) {
    _name = name;
}

void SplitSection::setStart(uint32_t start) {
    _start = start;
}

void SplitSection::setEnd(uint32_t end) {
    _end = end;
}