#ifndef LDSECTION_HPP_
#define LDSECTION_HPP_

#include "../writable.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

class LDSection : public Writable {
    public:
        LDSection(uint32_t address, const std::string &name, size_t size = 0);
        ~LDSection() = default;
        void write(std::ofstream& output) const override;
        void addFile(const std::string &file, uint32_t address = 0);
        uint32_t getBaseAddress() const;
        const std::string& getName() const;
        std::vector<std::string> &getFiles();
        size_t getSize() const;
    protected:
    private:
        uint32_t _address;
        std::string _name;
        size_t _size;
        std::vector<std::string> _files;
        std::vector<uint32_t> _addresses;
};


#endif /* !LDSECTION_HPP_ */
