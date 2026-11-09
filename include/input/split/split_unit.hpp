#ifndef SPLIT_UNIT_HPP_
#define SPLIT_UNIT_HPP_

#include "input/readable.hpp"
#include "split_section.hpp"
#include <cstdint>

class SplitUnit : public Readable {
    public:
        SplitUnit() = default;
        ~SplitUnit() = default;
        void read(std::ifstream& input, const std::string &line = "") override;
        const std::string &getPath() const;
        std::string getPathWithoutExtension() const;
        std::vector<SplitSection> &getSections();
    private:
        std::vector<SplitSection> _sections;
        std::string _path;
};

#endif /* !SPLIT_UNIT_HPP_ */