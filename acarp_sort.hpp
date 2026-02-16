#pragma once
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <cstring>
#include <vector>

namespace acarp {

struct BucketPlan {
    std::uint64_t bucket_mask;
    std::uint8_t  bucket_shift;
    int           num_buckets;
    std::int64_t  s_min;
};

struct BucketLayout {
    int           num_buckets;
    int          *counts;   
    std::size_t  *offsets; 
};

inline int ceil_log2(std::uint32_t x) {
    if (x <= 1) return 0;
    int r = 0;
    while ((std::uint32_t(1) << r) < x) { ++r; }
    return r;
}

inline void insertion_sort(std::int64_t* first, std::int64_t* last) {
    for (std::int64_t* it = first + 1; it < last; ++it) {
        std::int64_t v = *it;
        std::int64_t* jt = it;
        while (jt > first && v < *(jt - 1)) {
            *jt = *(jt - 1);
            --jt;
        }
        *jt = v;
    }
}

// Turbo ACARP with K=256 and Scratch-Buffer Support
template<int K = 256, int BUF_CAP = 16>
struct ACARP_Sort {
    static_assert((K & (K - 1)) == 0, "K must be power of 2");

    static void sort(std::int64_t* data, std::size_t n, std::int64_t* scratch_buffer = nullptr) {
        if (n <= 64) {
            insertion_sort(data, data + n);
            return;
        }

        // Manage scratch memory
        std::vector<std::int64_t> internal_scratch;
        std::int64_t* scratch = scratch_buffer;
        if (!scratch) {
            internal_scratch.resize(n);
            scratch = internal_scratch.data();
        }
        
        // Initial copy to scratch
        std::memcpy(scratch, data, n * sizeof(std::int64_t));

        int counts[K] = {0};
        std::size_t offsets[K] = {0};

        // 1. Plan
        BucketPlan plan = build_bucket_plan(scratch, n);
        BucketLayout layout{plan.num_buckets, counts, offsets};

        // 2. Count
        const std::uint64_t mask  = plan.bucket_mask;
        const std::uint8_t  shift = plan.bucket_shift;
        const std::int64_t  s_min = plan.s_min;

        for (std::size_t i = 0; i < n; ++i) {
            std::uint64_t u = static_cast<std::uint64_t>(scratch[i] - s_min);
            std::uint32_t key = static_cast<std::uint32_t>((u & mask) >> shift);
            if (key >= (uint32_t)plan.num_buckets) key = plan.num_buckets - 1;
            ++layout.counts[key];
        }

        // 3. Offsets
        std::size_t curr_off = 0;
        for (int j = 0; j < layout.num_buckets; ++j) {
            layout.offsets[j] = curr_off;
            curr_off += layout.counts[j];
        }

        // 4. Permute (SMB)
        std::size_t write_ptr[K];
        for (int j = 0; j < layout.num_buckets; ++j) write_ptr[j] = layout.offsets[j];

        alignas(64) std::int64_t buffer[K][BUF_CAP];
        std::uint8_t buf_size[K] = {0};

        for (std::size_t i = 0; i < n; ++i) {
            std::int64_t v = scratch[i];
            std::uint64_t u = static_cast<std::uint64_t>(v - s_min);
            std::uint32_t key = static_cast<std::uint32_t>((u & mask) >> shift);
            if (key >= (uint32_t)plan.num_buckets) key = plan.num_buckets - 1;

            std::uint8_t b_idx = buf_size[key];
            buffer[key][b_idx] = v;
            buf_size[key] = b_idx + 1;

            if (buf_size[key] == BUF_CAP) {
                std::memcpy(data + write_ptr[key], buffer[key], BUF_CAP * sizeof(std::int64_t));
                write_ptr[key] += BUF_CAP;
                buf_size[key] = 0;
            }
        }

        for (int key = 0; key < layout.num_buckets; ++key) {
            if (buf_size[key] > 0) {
                std::memcpy(data + write_ptr[key], buffer[key], buf_size[key] * sizeof(std::int64_t));
            }
        }

        // 5. Finish
        for (int j = 0; j < layout.num_buckets; ++j) {
            if (layout.counts[j] > 1) {
                std::sort(data + layout.offsets[j], data + layout.offsets[j] + layout.counts[j]);
            }
        }
    }

private:
    static BucketPlan build_bucket_plan(const std::int64_t* a, std::size_t n) {
        const std::size_t sample_size = std::min<std::size_t>(n, 512);
        const std::size_t stride = n / sample_size;

        std::int64_t s_min = a[0], s_max = a[0];
        for (std::size_t i = 0; i < n; i += stride) {
            if (a[i] < s_min) s_min = a[i];
            if (a[i] > s_max) s_max = a[i];
        }

        int b = ceil_log2(K);
        std::uint8_t shift = 64 - static_cast<std::uint8_t>(b);
        std::uint64_t mask = ((std::uint64_t(1) << b) - 1) << shift;

        return {mask, shift, K, s_min};
    }
};

} // namespace acarp
