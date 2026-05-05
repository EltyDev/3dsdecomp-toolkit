#ifndef ARM_PARSER_HPP_
#define ARM_PARSER_HPP_

#include <capstone/capstone.h>
#include <cstdint>
#include <vector>
#include <memory>

class ArmParser {
    public:
        ArmParser(const ArmParser&);
        ArmParser(const std::vector<uint8_t>& data, uint32_t baseAddress, cs_mode mode = CS_MODE_ARM);
        ~ArmParser();
        void changeMode(cs_mode mode);
        cs_insn* getNextInstruction();
        cs_insn* getNextInstructions(size_t count, size_t* outCount);
        bool isEnd() const;
        void goToAddress(uint32_t address);
        void switchMode();
        uint32_t getBaseAddress() const;
        size_t getDataSize() const;
        void close() const;
    protected:
    private:
        void create(cs_mode mode);
        const std::vector<uint8_t>& _data;
        csh _handle;
        cs_mode _mode;
        uint32_t _baseAddress;
        uint64_t _offset;
        cs_insn *_lastInstruction;
        size_t _nbInstructions;
};

#endif /* !ARM_PARSER_HPP_ */
