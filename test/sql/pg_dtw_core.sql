SELECT round(
    dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        '{}'::jsonb
    )::numeric, 6
) AS identical_cost;

SELECT round(
    dtw_distance(
        ARRAY['2025-01-01 00:00:01+00'::timestamptz, '2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[2.0, 1.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        '{}'::jsonb
    )::numeric, 6
) AS sorted_equivalent_cost;

SELECT round(
    dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 3.0, 2.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[2.0, 2.0]::double precision[],
        '{"duplicate_bucket_us": 1}'::jsonb
    )::numeric, 6
) AS duplicate_agg_cost;

SELECT round(
    dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:02+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        '{"time_weight": 0.5}'::jsonb
    )::numeric, 6
) AS time_penalty_cost;

SELECT round(
    dtw_distance_normalized(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 4.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        '{"metric":"squared"}'::jsonb
    )::numeric, 6
) AS normalized_squared_cost;

SELECT round(
    dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        '{"constraint":"sakoe_chiba","window":0}'::jsonb
    )::numeric, 6
) AS sakoe_chiba_cost;

SELECT round(
    dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamp, '2025-01-01 00:00:01+00'::timestamp],
        ARRAY[1.0, 2.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamp, '2025-01-01 00:00:01+00'::timestamp],
        ARRAY[1.0, 2.0]::double precision[],
        '{}'::jsonb
    )::numeric, 6
) AS timestamp_wrapper_cost;

SELECT round(
    dtw_distance_int8(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1, 2]::bigint[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1, 2]::bigint[],
        '{}'::jsonb
    )::numeric, 6
) AS int8_wrapper_cost;

SELECT round(
    dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0, 2.0]::double precision[],
        '{"duplicate_bucket":"1 second"}'::jsonb
    )::numeric, 6
) AS duplicate_bucket_interval_option_cost;

SELECT
    array_length(ts_out, 1) AS canon_len,
    round(vals_out[1]::numeric, 6) AS canon_v1,
    round(vals_out[2]::numeric, 6) AS canon_v2
FROM dtw_canonicalize_sequence(
    ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
    ARRAY[1.0, 3.0, 2.0]::double precision[],
    interval '1 microsecond'
);
