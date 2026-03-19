# Planner and Index Design (v1 investigation)

## Why direct DTW indexing is difficult

Exact DTW is elastic and query-dependent; similarity cannot be mapped cleanly to simple ordering keys. Most practical systems rely on filtering with lower bounds followed by exact DTW refinement.

## Near-term strategy

1. expose SQL-visible lower-bound helpers (`dtw_lb_kim` in v1)
2. add LB_Keogh envelopes and pruning primitives in later phases
3. combine with expression/materialized features for candidate reduction

## PostgreSQL index directions

- **GiST/SP-GiST**: possible for envelope/feature-space indexing, not exact DTW itself
- **BRIN**: useful for coarse partition pruning on time bounds/features
- **Custom AM**: feasible long-term but high implementation and maintenance complexity

## Planner integration opportunities

- support functions could improve costing for DTW predicates
- selectivity estimation likely requires feature statistics, not raw DTW formula
- initial production approach should keep explicit user-controlled prefilter predicates

## Honest v1 boundary

- no operator class
- no custom AM
- no claim of indexed exact DTW
- only foundational lower-bound hook and design documentation
