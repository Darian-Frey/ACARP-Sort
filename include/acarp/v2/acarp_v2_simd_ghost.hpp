#ifndef ACARP_V2_SIMD_GHOST_HPP
#define ACARP_V2_SIMD_GHOST_HPP

#include <immintrin.h>
#include <stdint.h>

namespace acarp::v2 {

/**
 * GHOST STATE MACHINE (AVX-512)
 * Manages 16 bucket counters and flush-logic entirely in registers.
 */
struct GhostStateMachine {
    __m512i counts;        // 16 x 32-bit counts
    __m512i lane_ids;      // [0, 1, ..., 15]
    __m512i capacity;      // All lanes = 8 (or 16)
    
    GhostStateMachine() {
        counts = _mm512_setzero_si512();
        lane_ids = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        capacity = _mm512_set1_epi32(8); // BUF_CAP
    }

    /**
     * PROCESS ELEMENT (Branchless)
     * 1. Updates counts for the active bucket.
     * 2. Returns a mask of buckets that need flushing.
     */
    inline __mmask16 increment_and_trigger(int bucket_idx) {
        __m512i v_key = _mm512_set1_epi32(bucket_idx);
        
        // Identify active lane
        __mmask16 k_active = _mm512_cmpeq_epi32_mask(lane_ids, v_key);
        
        // Increment only the active lane
        counts = _mm512_mask_add_epi32(counts, k_active, counts, _mm512_set1_epi32(1));
        
        // Neptune Sign-Bit Bridge Trigger:
        // temp = count - capacity. If temp >= 0, sign bit is 0.
        __m512i temp = _mm512_sub_epi32(counts, capacity);
        __mmask16 k_lt = _mm512_movepi32_mask(temp); // Mask where temp < 0
        __mmask16 k_full = ~k_lt;                   // Mask where count >= capacity
        
        return k_full;
    }

    inline void reset_lanes(__mmask16 k_full) {
        counts = _mm512_mask_mov_epi32(counts, k_full, _mm512_setzero_si512());
    }
};

} // namespace acarp::v2
#endif
