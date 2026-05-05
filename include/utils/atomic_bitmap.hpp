#ifndef ATOMIC_BITMAP_HPP_
#define ATOMIC_BITMAP_HPP_

#include <vector>
#include <atomic>

class AtomicBitmap
{
    public:
        AtomicBitmap(size_t size, uint32_t base = 0);

        bool insert(uint32_t addr);
        bool contains(uint32_t addr) const;

    private:
        std::vector<std::atomic<uint8_t>> _bits;
        uint32_t _base;
        size_t _size;
};

#endif /* !ATOMIC_BITMAP_HPP_ */
