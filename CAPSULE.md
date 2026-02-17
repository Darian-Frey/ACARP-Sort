# SCHEMA_V5 PROJECT CAPSULE: ACARP-Sort
**ID:** ST-FINAL-ACARP-2026-02-17
**STATUS:** SEALED / ARCHIVED
**PROTOCOL:** SCHEMA_V5 (Integration Layer)

---

## 1. PROJECT GENESIS
- **Hypothesis:** Comparison-based sorts (O(n log n)) are logic-heavy and hardware-inefficient on modern superscalar CPUs due to branch misprediction.
- **Solution:** Implement a Hybrid Radix-Distribution engine utilizing Software-Managed Buffers (SMB) to transform random writes into cache-aligned bursts.

## 2. STATE TRANSITION LOG (ST)
- **ST[1-3]: FOUNDATION:** Established predictive bucketing logic and O(1) mapping functions.
- **ST[4-7]: HARDENING:** Integrated Software-Managed Buffers (SMB) to stabilize the Write-Combining Buffer throughput. Addressed memory corruption edge cases via tile-based permutation.
- **ST[8-9]: BANDWIDTH OPTIMIZATION:** Transitioned from stack-local to static-scratch-buffer architecture to eliminate OS-level allocation taxes.
- **ST[10-11]: HARDWARE ACCELERATION:** Implemented AVX2 SIMD counting pass, reaching the "Bandwidth Bound" performance ceiling.

## 3. CORE ARCHITECTURAL ARTIFACTS
- **Engine:** acarp_sort.hpp (Header-only, SIMD-enabled, Cache-aware).
- **Harness:** benchmark_acarp.cpp (High-precision telemetry).
- **Theory:** WHITE_PAPER.md & ARCHITECTURE.md (Hardware-software co-design documentation).

## 4. COGNITIVE ANCHORS (IMMUTABLE RULES)
- **Bucket Stability:** K must remain a power of 2 to ensure branchless bit-masking.
- **Cache Alignment:** SMB capacity must not exceed L1-D cache limits (typically 32KB).
- **State Integrity:** All permutations must occur via scratch-space or validated swap-cycles to prevent data clobbering.

## 5. RECONSTITUTION DATA
To resume development on the "In-Place" or "AVX-512" branch, initialize SCHEMA_V5 Architect Mode and reference REF_ID: st-archive-acarp-2026.

---

**ARCHIVE SEALED BY:** Gemini (SCHEMA_V5 Senior Architect)
**DATE:** 2026-02-17
