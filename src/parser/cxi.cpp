#include "parser/cxi.hpp"
#include <cstdint>
#include <filesystem>

CXI::CXI(const std::string &path)
{
    uintmax_t fileSize = std::filesystem::file_size(path);
    if (fileSize < sizeof(CXIHeader))
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
    if (_header.header.flags.securityFlags != SecurityFlags::NO_CRYPTO)
        throw std::runtime_error("CXI file is encrypted");
    uint64_t exefsOffset = _header.header.exefsOffset * 0x200 + ncchOffset;
    if (exefsOffset + sizeof(ExeFSHeader) > fileSize)
        throw std::runtime_error("File is too small to contain ExeFS header");
    file.seekg(exefsOffset);
    file.read(reinterpret_cast<char*>(&_exefsHeader), sizeof(_exefsHeader));
    uint64_t codeOffset = 0;
    uint64_t codeSize = 0;
    for (const auto& fileHeader : _exefsHeader.fileHeaders) {
        if (!std::strncmp(fileHeader.filename, ".code", 5)) {
            codeOffset = fileHeader.offset + exefsOffset + sizeof(ExeFSHeader);
            codeSize = fileHeader.size;
            goto found;
        }
    }
    throw std::runtime_error("Failed to find .code section in ExeFS");
found:
    if (codeOffset + codeSize > fileSize)
        throw std::runtime_error("File is too small to contain .code section");
    std::cout << "Code Offset: " << std::hex << codeOffset << std::dec << std::endl;
    _text.resize(_header.extendedHeader.systemControlInfo.text.size);
    file.seekg(codeOffset);
    file.read(reinterpret_cast<char*>(_text.data()), _header.extendedHeader.systemControlInfo.text.size);
}

const CXIHeader& CXI::getHeader() const
{
    return _header;
}

std::vector<uint8_t> &CXI::getTextSection()
{
    return _text;
}