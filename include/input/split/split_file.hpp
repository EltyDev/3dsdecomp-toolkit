#ifndef SPLIT_FILE_HPP_
#define SPLIT_FILE_HPP_

#include "split_section.hpp"
#include "split_unit.hpp"

class SplitFile : public Readable {
    public:
        SplitFile() = default;
        ~SplitFile() = default;
        void read(std::ifstream& input, const std::string &line = "") override;
        const std::vector<SplitSection>& getSections() const;
        std::vector<SplitUnit>& getUnits();
    private:
        std::vector<SplitSection> _sections;
        std::vector<SplitUnit> _units;
};

#endif /* !SPLIT_FILE_HPP_ */