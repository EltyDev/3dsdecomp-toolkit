#ifndef FUNCTION_HEURISTIC_HPP_
#define FUNCTION_HEURISTIC_HPP_

#include <atomic>
#include <stdint.h>
#include <vector>
#include <capstone/capstone.h>
#include "utils/concurrent_queue.hpp"
#include "utils/concurrent_vector.hpp"
#include "utils/atomic_bitmap.hpp"

struct Function
{
    cs_mode mode;
    uint32_t start;
    uint32_t end;
};

struct HeuristicEntry
{
    uint32_t address;
    cs_mode mode;
    bool foundBefore;
};

struct HeuristicContext
{
    std::atomic_int32_t activeThreads;
    ConcurrentQueue<HeuristicEntry> &toProcess;
    AtomicBitmap processed;
    ConcurrentVector<Function> &functions;
    AtomicBitmap knownFunctions;
};

namespace function_heuristic
{
    std::vector<Function> findFunctions(const std::vector<uint8_t>& data, uint32_t baseAddress);
}


#endif /* !FUNCTION_HEURISTIC_HPP_ */
