#include "output/ninja/build_rule.hpp"

BuildRule::BuildRule(const std::string &output, const std::string &ruleName) : _output(output), _ruleName(ruleName) {}

BuildRule::BuildRule(const std::string &input, const std::string &output, const std::string &ruleName) : _output(output), _ruleName(ruleName) {
    _inputs.push_back(input);
}

void BuildRule::write(std::ofstream& output) const {
    output << "build " << _output << ": " << _ruleName;
    for (const auto& input : _inputs) {
        output << " " << input;
    }
    output << "\n";
}

void BuildRule::addInput(const std::string &input) {
    _inputs.push_back(input);
}