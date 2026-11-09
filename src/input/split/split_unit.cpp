#include "input/split/split_unit.hpp"
#include <string>
#include <sstream>

void SplitUnit::read(std::ifstream& input, const std::string &line) {
    std::string actualLine = line;
    if (actualLine.empty())
        std::getline(input, actualLine);
    std::istringstream iss(actualLine);
    std::string word;
    iss >> word;
    size_t pos = word.find(':');
    if (pos == std::string::npos)
        throw std::runtime_error("Invalid format in split unit: " + word);
    _path = word.substr(0, pos);
    while (std::getline(input, actualLine) && !actualLine.empty()) {
        iss = std::istringstream(actualLine);
        bool missings[2] = {true, true};
        SplitSection section;
        for (int i = 0; iss >> word; ++i) {
            if (i == 0) {
                section.setName(word);
                continue;
            }
            std::string key = word.substr(0, word.find(':'));
            std::string value = word.substr(word.find(':') + 1);
            if (key == "start") {
                section.setStart(std::stoul(value, nullptr, 0));
                missings[0] = false;
            } else if (key == "end") {
                section.setEnd(std::stoul(value, nullptr, 0));
                missings[1] = false;
            }
        }
        _sections.push_back(section);
        std::ostringstream missingFields;
        if (missings[0])
            missingFields << "start ";
        if (missings[1])
            missingFields << "end ";
        if (missingFields.str().length() > 0)
            throw std::runtime_error("Missing fields in split unit: " + missingFields.str());
    }
}

const std::string &SplitUnit::getPath() const {
    return _path;
}

std::string SplitUnit::getPathWithoutExtension() const {
    std::string pathWithoutExtension = _path;
    size_t lastDotPos = pathWithoutExtension.find_last_of('.');
    if (lastDotPos != std::string::npos)
        pathWithoutExtension = pathWithoutExtension.substr(0, lastDotPos);
    return pathWithoutExtension;
}

std::vector<SplitSection> &SplitUnit::getSections() {
    return _sections;
}