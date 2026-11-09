#ifndef INPUT_SECTION_HPP_
#define INPUT_SECTION_HPP_

#include "output/writable.hpp"
#include <initializer_list>
#include <string_view>
#include <vector>

class InputSection : public Writable {
    public:
        InputSection(const std::string &selectPattern, std::initializer_list<std::string_view> attributes);
        void write(std::ofstream& output) const override;
        void addAttribute(const std::string &attribute);
        const std::vector<std::string>& getAttributes() const;
    protected:
    private:
        std::string _selectPattern;
        std::vector<std::string> _attributes;

};

#endif /* !INPUT_SECTION_HPP_ */
