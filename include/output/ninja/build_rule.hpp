#ifndef BUILD_RULE_HPP_
#define BUILD_RULE_HPP_

#include "../writable.hpp"
#include <vector>

class BuildRule : public Writable {
    public:
        BuildRule(const std::string &input, const std::string &output, const std::string &ruleName);
        BuildRule(const std::string &output, const std::string &ruleName);
        ~BuildRule() = default;
        void write(std::ofstream& output) const override;
        void addInput(const std::string &input);
    protected:
    private:
        std::string _output;
        std::string _ruleName;
        std::vector<std::string> _inputs;
};


#endif /* !BUILD_RULE_HPP_ */
