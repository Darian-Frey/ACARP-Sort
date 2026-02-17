#ifndef ACARP_V2_DISTRIBUTION_HPP
#define ACARP_V2_DISTRIBUTION_HPP

#include <immintrin.h>
#include <stdint.h>

namespace acarp::v2 {

/**
 * SIGN-BIT POINTER CLAMP
 * Satures the write pointer at the bucket boundary without branches.
 */
static inline uint64_t clamp_ptr(uint64_t current_pos, uint64_t bucket_end) {
    int64_t temp = (int64_t)(current_pos - bucket_end);
    int64_t mask = temp >> 63;
    return (uint64_t)(temp + (mask & (int64_t)bucket_end));
}

/**
 * BRANCHLESS DISTRIBUTION PASS
 * Uses the Neptune Sign-Bit Bridge to saturate bucket pointers.
 */
template <typename T>
void distribute_branchless(T* input, T* output, uint64_t* write_ptrs, uint64_t* ends, size_t n, uint32_t shift, uint32_t mask_val) {
    for (size_t i = 0; i < n; ++i) {
        T val = input[i];
        uint32_t bucket = (val >> shift) & mask_val;
        
        uint64_t pos = write_ptrs[bucket];
        output[pos] = val;
        
        // Neptune Sign-Bit Bridge: Increment and Clamp
        write_ptrs[bucket] = clamp_ptr(pos + 1, ends[bucket]);
    }
}

} // namespace acarp::v2
#endif
