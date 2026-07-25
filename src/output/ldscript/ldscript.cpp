#include "output/ldscript/ldscript.hpp"

LDScript::LDScript(const std::string entrypoint) : _entrypoint(entrypoint) {}

void LDScript::write(std::ofstream& output) const {
    output << "ENTRY(" << _entrypoint << ");\n";
    output << "SECTIONS\n";
    output << "{\n";
    for (const auto& section : _sections) {
        section.write(output);
    }
    output << "}\n";
}

std::vector<LDSection> &LDScript::getSections() {
    return _sections;
}

void LDScript::addSection(const LDSection &section) {
    _sections.push_back(section);
}

const std::string& LDScript::getEntrypoint() const {
    return _entrypoint;
}
