#include "parser/cxi.hpp"
#include <filesystem>

CXI::CXI(const std::string &path)
{
    if (std::filesystem::file_size(path) < sizeof(CXIHeader))
        throw std::runtime_error("File is too small to be a valid CXI");
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open() || !file.good())
        throw std::runtime_error("Failed to open file " + path + ": " + std::strerror(errno));
    char magic[4];
    uint64_t ncchOffset = 0;
    file.seekg(0x100);
    file.read(magic, 4);
    if (!std::strncmp(magic, "NCSD", 4))
    {
        uint32_t partOffset;
        file.seekg(0x120);
        file.read(reinterpret_cast<char*>(&partOffset), sizeof(partOffset));
        ncchOffset = partOffset * 0x200;
    }
    else if (std::strncmp(magic, "NCCH", 4) != 0)
        throw std::runtime_error("Invalid CXI file: magic is not NCSD or NCCH");
    file.seekg(ncchOffset);
    file.read(reinterpret_cast<char*>(&_header), sizeof(_header));
}

const CXIHeader& CXI::getHeader() const
{
    return _header;
}