CREATE EXTENSION IF NOT EXISTS pg_dtw;

WITH s1 AS (
    SELECT array_agg(ts ORDER BY ts) AS ts, array_agg(v ORDER BY ts)::float8[] AS vals
    FROM (
        SELECT
            ('2025-01-01 00:00:00+00'::timestamptz + (i || ' seconds')::interval) AS ts,
            sin(i / 10.0) AS v
        FROM generate_series(1, 200) AS i
    ) q
),
s2 AS (
    SELECT array_agg(ts ORDER BY ts) AS ts, array_agg(v ORDER BY ts)::float8[] AS vals
    FROM (
        SELECT
            ('2025-01-01 00:00:00+00'::timestamptz + (i || ' seconds')::interval) AS ts,
            sin(i / 10.0) + 0.05 AS v
        FROM generate_series(1, 200) AS i
    ) q
)
SELECT dtw_distance(
    s1.ts, s1.vals, s2.ts, s2.vals,
    '{"metric":"absolute","time_weight":0.1}'::jsonb
)
FROM s1, s2;
