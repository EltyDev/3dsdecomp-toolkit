#ifndef IDA_EXPORT_HPP_
#define IDA_EXPORT_HPP_

#include "readable.hpp"
#include <capstone/capstone.h>
#include <cstdint>
#include <vector>

struct Location {
    uint32_t start;
    uint32_t end;
    cs_mode mode;
};

struct IDAFunction {
    Location location;
    std::vector<Location> locations;
}; 

class IDAExport : public Readable {
    public:
        IDAExport() = default;
        ~IDAExport() = default;
        void read(std::ifstream& input, const std::string &line = "") override;
        const std::vector<IDAFunction>& getFunctions() const;
    private:
        std::vector<IDAFunction> _functions;
};

#endif /* !IDA_EXPORT_HPP_ */