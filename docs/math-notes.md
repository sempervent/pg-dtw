# Math Notes

## Classical DTW

For sequences `A` length `n` and `B` length `m`, DTW minimizes cumulative path cost through an `n x m` grid with predecessor transitions `(i-1,j-1)`, `(i-1,j)`, `(i,j-1)`.

## Time-aware local cost

v1 local cost:

`c(i,j) = value_weight * metric(a_i, b_j) + time_weight * |ts_i - ts_j|_seconds`

- `metric` is absolute or squared value difference
- timestamp difference is converted from microseconds to seconds to keep scales practical

## Normalization

- raw accumulated cost
- path-length normalized cost (`accumulated / path_length`)

## Constraints

- v1 implemented: unconstrained, Sakoe-Chiba band
- parsed but deferred: Itakura and other variants

## Duplicate timestamps

Before DTW, each input is sorted and bucketed by `duplicate_bucket_us`; points in the same bucket are aggregated with average value.

## Why DTW indexing is hard

DTW is elastic and non-linear; direct exact indexing is difficult because ordering is warped and triangle inequality can fail under variants. Practical acceleration usually relies on lower bounds and candidate pruning.
