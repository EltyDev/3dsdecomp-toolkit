#include "input/split/split_file.hpp"

void SplitFile::read(std::ifstream& input, const std::string &line) {
    std::string actuaLine = line;
    if (actuaLine.empty())
        std::getline(input, actuaLine);
    bool readingSections = false;
    bool readingUnits = false;
    do {
        if (actuaLine.empty()) {
            if (readingSections)
                readingUnits = true;
            readingSections = false;
            continue;
        }
        if (readingSections) {
            SplitSection &section = _sections.emplace_back();
            section.read(input, actuaLine);
        } else if (readingUnits) {
            SplitUnit &unit = _units.emplace_back();
            unit.read(input, actuaLine);
            for (auto& section : unit.getSections()) {
                auto it = std::find_if(_sections.begin(), _sections.end(), [&section](const SplitSection& s) {
                    return s.getName() == section.getName();
                });
                if (it == _sections.end())
                    throw std::runtime_error("Section " + section.getName() + " not found for unit " + unit.getPath());
                section.setType(it->getType());
            }
        } else if (actuaLine == "Sections:") {
            readingSections = true;
            continue;
        }
    } while (std::getline(input, actuaLine));
}

const std::vector<SplitSection>& SplitFile::getSections() const {
    return _sections;
}

std::vector<SplitUnit>& SplitFile::getUnits() {
    return _units;
}