# Algorithm Design

## 1. Parsing and validation

Inputs are accepted as arrays and validated for:

- one-dimensional shape
- expected element types in core functions (`timestamptz[]`, `float8[]`)
- equal lengths and non-empty sequences
- non-null elements
- finite numeric values

## 2. Canonicalization

Each sequence is converted to an internal buffer and canonicalized:

1. stable sort by timestamp
2. duplicate bucketing by configured bucket (`duplicate_bucket_us` or `duplicate_bucket`)
3. bucket aggregation (v1: average)

This gives deterministic behavior for unsorted and duplicate timestamps.

## 3. DP kernel

The kernel computes DTW with rolling rows:

- time complexity: `O(n*m)`
- memory complexity: `O(m)` rolling rows for cost and path length
- optional Sakoe-Chiba pruning by `window`

For normalized DTW, path length is tracked in parallel DP state.

## 4. Error semantics

Errors are deterministic and explicit with stable prefixes (`pg_dtw:`). Unsupported options produce "not yet implemented" instead of silent fallback.
Unknown option keys are treated as errors.

## 5. Extensibility points

- `DtwOptions` contains planned option hooks
- `lower_bounds.c` reserves pruning helpers
- SQL wrappers show migration path toward richer ingestion APIs
