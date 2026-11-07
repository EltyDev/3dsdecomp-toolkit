#ifndef ATOMIC_BITMAP_HPP_
#define ATOMIC_BITMAP_HPP_

#include "capstone/capstone.h"
#include <vector>
#include <atomic>

class AtomicBitmap
{
    public:
        AtomicBitmap(size_t size, uint32_t base = 0);

        bool insert(uint32_t addr, cs_mode mode);
        bool contains(uint32_t addr, cs_mode mode) const;

    private:
        std::vector<std::atomic<uint8_t>> _bitsArm;
        std::vector<std::atomic<uint8_t>> _bitsThumb;
        uint32_t _base;
        size_t _size;
};

#endif /* !ATOMIC_BITMAP_HPP_ */
