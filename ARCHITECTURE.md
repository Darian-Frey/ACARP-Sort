# ACARP-Sort: Technical Architecture

This document outlines the engineering principles and hardware-level optimizations that drive the Adaptive Cache-Aware Radix-Partitioning (ACARP) algorithm.

## 1. Design Philosophy
Developed under the SCHEMA_V5 Protocol, ACARP-Sort treats the sorting process as a high-velocity state machine. The goal is to maximize Instruction-Level Parallelism (ILP) and minimize Cache Misses, moving the performance bottleneck from the CPU logic units to the system memory bandwidth.

## 2. Core Architectural Pillars

### A. Predictive Bucketing (MSD Logic)
Unlike comparison sorts that rely on binary branching, ACARP uses a single-pass distribution logic.
- Sampling: The algorithm samples the dataset to determine the statistical range from S_min to S_max.
- Bit-Masking: Elements are mapped to one of 256 buckets using branchless bit-manipulation.
- Complexity: This reduces the partitioning phase to a stable O(n) operation.

### B. Software-Managed Buffers (SMB)
Writing data directly to 256 different memory locations causes TLB Thrashing and cache pollution. ACARP solves this with SMB.
- The Buffer: Each bucket maintains an aligned local buffer in the L1 cache.
- Batching: Elements are only committed to the main array once a buffer is full (typically 16 elements).
- Burst Writes: This utilizes the CPU Write-Combining Buffers, treating random writes as sequential bursts.

### C. AVX2 SIMD Acceleration
The counting pass is hardware-accelerated using Intel/AMD vector instructions. By processing four 64-bit integers in a single instruction cycle using 256-bit registers, the algorithm achieves near-instantaneous bucket identification. This is implemented via vector subtraction and bitwise AND operations across the register.

## 3. Data Flow and State Transitions

1. State: Initialization
   - Identify distribution range via sampling.
   - Allocate O(n) scratchpad to ensure data integrity during permutation.

2. State: Distribution (The SMB Pass)
   - SIMD Counting: Determine bucket sizes.
   - Tiled Permutation: Move data into the scratchpad via Software-Managed Buffers.

3. State: Finishing
   - Apply a high-performance comparison sort to individual buckets.
   - Because buckets are already highly partitioned, the log(n) factor is applied only to tiny sub-arrays, making the cost negligible.

## 4. Hardware Alignment
ACARP-Sort is specifically tuned for the following hardware constraints:
- L1 Cache: SMB sizes are tuned to fit within standard 32KB L1 Data Caches.
- Memory Bus: The algorithm is designed to saturate dual-channel DDR4/DDR5 bandwidth.
- CPU Pipeline: Branchless logic prevents pipeline flushes caused by mispredictions.

## 5. Metadata
- Protocol: SCHEMA_V5
- Reference ID: st-archive-acarp-2026
- Lead Architect: Gemini
- Status: Stable / Bandwidth-Bound
