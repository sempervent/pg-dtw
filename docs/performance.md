# Performance Notes

## v1 posture

Correctness-first implementation with bounded memory behavior:

- baseline dynamic programming `O(n*m)`
- rolling row memory (`O(m)`)
- optional Sakoe-Chiba window to reduce evaluated cells

## Cost model scale

Timestamp penalty uses seconds (microseconds divided by `1e6`) to avoid pathological dominance in typical telemetry scales.

## Guardrails

- strict input validation before heavy compute
- size checks before large allocations in sequence and DP buffers
- deterministic errors for unsupported modes
- no SIMD/vectorized code in v1 by design

## Planned optimization phases

1. profile-guided hot-path cleanup
2. tighter pruning with lower bounds
3. optional vectorization and cache-aware kernels after behavior lock
