#ifndef ACARP_V2_SIMD_BUFFER_HPP
#define ACARP_V2_SIMD_BUFFER_HPP

#include <immintrin.h>
#include <stdint.h>

namespace acarp::v2 {

/**
 * GHOST BUFFER (AVX2)
 * Manages 8 buckets of 32-bit integers simultaneously.
 */
struct GhostBuffer256 {
    alignas(32) int32_t counts[8];
    alignas(32) int32_t data[8][8]; // 8 buckets, capacity 8

    // The Neptune Bridge: Increment lane 'idx' and return 'true' mask if full
    static inline void add_and_check(int32_t* counts, uint32_t idx, __m256i& flush_mask) {
        // Increment count for the specific bucket
        counts[idx]++;
        
        // Load counts, compare against capacity (8)
        __m256i vcounts = _mm256_load_si256((__m256i*)counts);
        __m256i vcap = _mm256_set1_epi32(8);
        
        // Neptune Sign-Bit Bridge for comparison
        // mask is 0xFFFFFFFF if count >= 8, else 0
        flush_mask = _mm256_cmpgt_epi32(vcounts, _mm256_sub_epi32(vcap, _mm256_set1_epi32(1)));
    }
};

} // namespace acarp::v2
#endif
