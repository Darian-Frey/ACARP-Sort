#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>
#include <iomanip>
#include "include/acarp/v2/acarp_v2_distribution.hpp"
#include "include/acarp/v2/acarp_v2_simd_ghost.hpp"

/**
 * ACARP V1 vs V2 Benchmark
 * Comparing Branchy Scalar Buffers vs. Sign-Bit SIMD State Machines
 */

using namespace std;
using namespace std::chrono;

const size_t N = 10000000; // 10 Million elements
const int BUCKETS = 8;     // 8 buckets for AVX2 test

// Mock V1 Distribution (Branchy)
void distribute_v1(const vector<uint32_t>& input, uint32_t* output, uint32_t* counts, uint32_t* write_ptrs) {
    for (uint32_t val : input) {
        uint32_t bucket = val % BUCKETS;
        uint32_t pos = write_ptrs[bucket]++;
        output[pos] = val;
        
        // The "Branchy" Bottleneck
        if (counts[bucket]++ == 8) {
            counts[bucket] = 0;
            // In a real V1, this would trigger a flush
            asm volatile("" : : : "memory");
        }
    }
}

// Mock V2 Distribution (Sign-Bit / Ghost)
void distribute_v2(const vector<uint32_t>& input, uint32_t* output, uint32_t* write_ptrs) {
    // We'll use a simplified version of our GhostStateMachine for AVX2
    alignas(32) int32_t counts[8] = {0};
    __m256i v_cap = _mm256_set1_epi32(8);
    
    for (uint32_t val : input) {
        uint32_t bucket = val % BUCKETS;
        uint32_t pos = write_ptrs[bucket]++;
        output[pos] = val;

        // SIMD Count Update (No Branch)
        counts[bucket]++;
        __m256i v_counts = _mm256_load_si256((__m256i*)counts);
        __m256i v_temp = _mm256_sub_epi32(v_counts, v_cap);
        __m256i v_mask = _mm256_cmpgt_epi32(v_counts, _mm256_set1_epi32(7)); // Trigger
        
        // Branchless Reset
        v_counts = _mm256_andnot_si256(v_mask, v_counts);
        _mm256_store_si256((__m256i*)counts, v_counts);
    }
}

int main() {
    vector<uint32_t> data(N);
    vector<uint32_t> output(N);
    uint32_t counts[BUCKETS] = {0};
    uint32_t write_ptrs[BUCKETS];
    for(int i=0; i<BUCKETS; ++i) write_ptrs[i] = i * (N/BUCKETS);

    mt19937 rng(42);
    for(size_t i=0; i<N; ++i) data[i] = rng();

    cout << "--- ACARP Performance Evolution Audit ---" << endl;
    cout << "Dataset: 10 Million Elements" << endl;

    // Benchmark V1
    auto start1 = high_resolution_clock::now();
    distribute_v1(data, output.data(), counts, write_ptrs);
    auto end1 = high_resolution_clock::now();
    duration<double> d1 = end1 - start1;

    // Reset pointers
    for(int i=0; i<BUCKETS; ++i) write_ptrs[i] = i * (N/BUCKETS);

    // Benchmark V2
    auto start2 = high_resolution_clock::now();
    distribute_v2(data, output.data(), write_ptrs);
    auto end2 = high_resolution_clock::now();
    duration<double> d2 = end2 - start2;

    cout << fixed << setprecision(4);
    cout << "V1 (Branchy): " << d1.count() << "s" << endl;
    cout << "V2 (Ghost):   " << d2.count() << "s" << endl;
    cout << "Improvement:  " << (d1.count() / d2.count() - 1.0) * 100.0 << "%" << endl;

    return 0;
}
