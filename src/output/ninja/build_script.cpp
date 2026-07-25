#include "output/ninja/build_script.hpp"

BuildScript::BuildScript() {}

void BuildScript::write(std::ofstream& output) const {
    for (const auto& rule : _rules) {
        rule.write(output);
        output << "\n";
    }
    for (const auto& buildRule : _buildRules) {
        buildRule.write(output);
        output << "\n";
    }
}

void BuildScript::addRule(const Rule &rule) {
    _rules.push_back(rule);
}

void BuildScript::addBuildRule(const BuildRule &buildRule) {
    _buildRules.push_back(buildRule);
}