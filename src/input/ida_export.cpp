#include "input/ida_export.hpp"
#include <cstdint>
#include <string>

namespace {

    std::vector<std::string> splitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(delimiter);
        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find(delimiter, start);
        }
        tokens.push_back(str.substr(start));
        return tokens;
    }

}

void IDAExport::read(std::ifstream& input, const std::string &line) {
    std::string actualLine = line;
    if (actualLine.empty())
        std::getline(input, actualLine);
    IDAFunction *currentFunction = nullptr;
    do {
        std::vector<std::string> tokens = splitString(actualLine, ',');
        if (tokens.size() < 4)
            throw std::runtime_error("Invalid line format in IDA export: " + actualLine);
        uint32_t start = std::stoul(tokens[0], nullptr, 0);
        uint32_t end = std::stoul(tokens[1], nullptr, 0);
        cs_mode mode = tokens[2] == "arm" ? CS_MODE_ARM : CS_MODE_THUMB;
        std::string kind = tokens[3];
        if (kind == "main") {
            currentFunction = &_functions.emplace_back();
            currentFunction->location = {start, end, mode};
            currentFunction->locations.clear();
        }
        else if (kind == "tail") {
            currentFunction->locations.push_back({start, end, mode});
        }
    } while (std::getline(input, actualLine));
}

const std::vector<IDAFunction>& IDAExport::getFunctions() const {
    return _functions;
}