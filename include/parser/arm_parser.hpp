#ifndef ARM_PARSER_HPP_
#define ARM_PARSER_HPP_

#include <capstone/capstone.h>
#include <cstdint>
#include <vector>
#include <memory>

class ArmParser {
    public:
        ArmParser(const std::vector<uint8_t>& data, cs_mode mode = CS_MODE_ARM);
        ~ArmParser();
        void changeMode(cs_mode mode);
        std::unique_ptr<cs_insn> getNextInstruction();
    protected:
    private:
        void create(cs_mode mode);
        const std::vector<uint8_t>& _data;
        csh _handle;
        cs_mode _mode;
        uint64_t _offset;
};

#endif /* !ARM_PARSER_HPP_ */
