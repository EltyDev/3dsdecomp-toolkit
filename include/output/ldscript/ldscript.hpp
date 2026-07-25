#ifndef LDSCRIPT_HPP_
#define LDSCRIPT_HPP_

#include "ldsection.hpp"

class LDScript : public Writable {
    public:
        LDScript(const std::string entrypoint);
        ~LDScript() = default;
        void write(std::ofstream& output) const override;
        std::vector<LDSection> &getSections();
        void addSection(const LDSection &section);
        const std::string& getEntrypoint() const;
    protected:
    private:
        std::string _entrypoint;
        std::vector<LDSection> _sections;
};


#endif /* !LDSCRIPT_HPP_ */
