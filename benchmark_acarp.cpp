#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>
#include "acarp_sort.hpp"

template <class T>
inline void DoNotOptimize(T const& value) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "g"(value) : "memory");
#else
    volatile auto* p = &value;
    (void)p;
#endif
}

int main() {
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<std::int64_t> dist(-1000000000, 1000000000);

    // Global scratch buffer to eliminate allocation costs
    const std::size_t MAX_N = 10000000;
    static std::vector<std::int64_t> global_scratch(MAX_N);

    std::vector<std::size_t> sizes = {10000, 100000, 1000000, 10000000};

    for (auto n : sizes) {
        std::vector<std::int64_t> base(n);
        for (auto& x : base) x = dist(rng);

        std::printf("--- Testing N = %zu ---\n", n);

        // Benchmark std::sort
        double std_total = 0;
        for (int i = 0; i < 20; ++i) {
            auto work = base;
            auto t0 = std::chrono::high_resolution_clock::now();
            std::sort(work.begin(), work.end());
            auto t1 = std::chrono::high_resolution_clock::now();
            std_total += std::chrono::duration<double, std::milli>(t1 - t0).count();
            DoNotOptimize(work.data());
        }
        std::printf("std::sort | %8.3f ms | %8.2f M/s\n", std_total / 20, (n / (std_total / 20 / 1000.0)) / 1e6);

        // Benchmark ACARP
        double acarp_total = 0;
        for (int i = 0; i < 20; ++i) {
            auto work = base;
            auto t0 = std::chrono::high_resolution_clock::now();
            // Pass global_scratch to avoid internal resize
            acarp::ACARP_Sort<256, 16>::sort(work.data(), work.size(), global_scratch.data());
            auto t1 = std::chrono::high_resolution_clock::now();
            acarp_total += std::chrono::duration<double, std::milli>(t1 - t0).count();
            DoNotOptimize(work.data());
        }
        std::printf("ACARP     | %8.3f ms | %8.2f M/s\n", acarp_total / 20, (n / (acarp_total / 20 / 1000.0)) / 1e6);
        std::printf("\n");
    }

    return 0;
}

