CREATE EXTENSION IF NOT EXISTS pg_dtw;

-- Example table-oriented ingestion pattern using ordered aggregates.
CREATE TABLE IF NOT EXISTS sensor_series (
    series_id text NOT NULL,
    ts timestamptz NOT NULL,
    value double precision NOT NULL
);

-- Compute DTW between two series ids.
WITH lhs AS (
    SELECT
        array_agg(ts ORDER BY ts) AS ts_arr,
        array_agg(value ORDER BY ts)::float8[] AS val_arr
    FROM sensor_series
    WHERE series_id = 'series_a'
),
rhs AS (
    SELECT
        array_agg(ts ORDER BY ts) AS ts_arr,
        array_agg(value ORDER BY ts)::float8[] AS val_arr
    FROM sensor_series
    WHERE series_id = 'series_b'
)
SELECT dtw_distance(
    lhs.ts_arr,
    lhs.val_arr,
    rhs.ts_arr,
    rhs.val_arr,
    '{"metric":"absolute","constraint":"sakoe_chiba","window":10}'::jsonb
)
FROM lhs, rhs;
