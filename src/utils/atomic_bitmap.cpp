#include "utils/atomic_bitmap.hpp"

AtomicBitmap::AtomicBitmap(size_t size, uint32_t base) : _bits((size + 7) / 8), _base(base), _size(size) {
    for (auto& byte : _bits)
        byte.store(0, std::memory_order_relaxed);
}

bool AtomicBitmap::insert(uint32_t addr) {
    size_t bit = (addr - _base) / 2;
    uint8_t mask = 1 << (bit % 8);
    return !(_bits[bit / 8].fetch_or(mask, std::memory_order_acq_rel) & mask);
}

bool AtomicBitmap::contains(uint32_t addr) const {
    if (addr < _base || addr >= _base + _size)
        return false;
    size_t bit = (addr - _base) / 2;
    return (_bits[bit / 8].load(std::memory_order_acquire) >> (bit % 8)) & 1;
}