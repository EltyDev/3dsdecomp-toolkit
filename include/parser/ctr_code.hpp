#ifndef CTR_CODE_HPP_
#define CTR_CODE_HPP_

#include <cstdint>
#include <vector>
#include <string>

struct Section
{
    uint32_t address;
    std::vector<uint8_t> data;
};

class CtrCode {
    public:
        CtrCode(const std::string &codePath, const std::string &headerPath);
        ~CtrCode() = default;
    protected:
    private:
        Section _text;
        Section _rodata;
        Section _data;
        Section _bss;

};


#endif /* !CTR_CODE_HPP_ */
