#ifndef BUILDSCCRIPT_HPP_
#define BUILDSCCRIPT_HPP_

#include "build_rule.hpp"
#include "rule.hpp"

#include <vector>

class BuildScript : public Writable {
    public:
        BuildScript();
        ~BuildScript() = default;
        void write(std::ofstream& output) const override;
        void addRule(const Rule &rule);
        void addBuildRule(const BuildRule &buildRule);
    protected:
    private:
        std::vector<Rule> _rules;
        std::vector<BuildRule> _buildRules;
};

#endif /* !BUILDSCCRIPT_HPP_ */
