CREATE EXTENSION IF NOT EXISTS pgtap;
CREATE EXTENSION IF NOT EXISTS pg_dtw;

SELECT plan(14);

CREATE OR REPLACE FUNCTION pg_temp.raises_nan_rejection() RETURNS boolean
LANGUAGE plpgsql
AS $$
BEGIN
    PERFORM dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY['NaN'::double precision],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1.0]::double precision[],
        '{}'::jsonb
    );
    RETURN false;
EXCEPTION WHEN OTHERS THEN
    RETURN position('input values must be finite' in SQLERRM) > 0;
END;
$$;

CREATE OR REPLACE FUNCTION pg_temp.raises_itakura_rejection() RETURNS boolean
LANGUAGE plpgsql
AS $$
BEGIN
    PERFORM dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1.0]::double precision[],
        '{"constraint":"itakura"}'::jsonb
    );
    RETURN false;
EXCEPTION WHEN OTHERS THEN
    RETURN position('not yet implemented' in SQLERRM) > 0;
END;
$$;

CREATE OR REPLACE FUNCTION pg_temp.raises_unknown_option() RETURNS boolean
LANGUAGE plpgsql
AS $$
DECLARE
    err_msg text;
    err_detail text;
BEGIN
    PERFORM dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1.0]::double precision[],
        '{"mystery":1}'::jsonb
    );
    RETURN false;
EXCEPTION WHEN OTHERS THEN
    GET STACKED DIAGNOSTICS err_msg = MESSAGE_TEXT, err_detail = PG_EXCEPTION_DETAIL;
    RETURN position('invalid options jsonb' in coalesce(err_msg, '')) > 0
       AND position('unknown option' in coalesce(err_detail, '')) > 0;
END;
$$;

SELECT has_function('public', 'dtw_distance', ARRAY['timestamp with time zone[]','double precision[]','timestamp with time zone[]','double precision[]','jsonb']);
SELECT has_function('public', 'dtw_distance_normalized', ARRAY['timestamp with time zone[]','double precision[]','timestamp with time zone[]','double precision[]','jsonb']);
SELECT is(
    (SELECT l.lanname
     FROM pg_proc p
     JOIN pg_language l ON l.oid = p.prolang
     WHERE p.oid = 'public.dtw_distance(timestamp with time zone[],double precision[],timestamp with time zone[],double precision[],jsonb)'::regprocedure),
    'c',
    'dtw_distance is implemented in C'
);
SELECT is(
    (SELECT p.provolatile
     FROM pg_proc p
     WHERE p.oid = 'public.dtw_distance(timestamp with time zone[],double precision[],timestamp with time zone[],double precision[],jsonb)'::regprocedure),
    'i',
    'dtw_distance is immutable'
);
SELECT is(
    (SELECT p.proparallel
     FROM pg_proc p
     WHERE p.oid = 'public.dtw_distance(timestamp with time zone[],double precision[],timestamp with time zone[],double precision[],jsonb)'::regprocedure),
    's',
    'dtw_distance is parallel safe'
);
SELECT is(
    (SELECT p.proisstrict
     FROM pg_proc p
     WHERE p.oid = 'public.dtw_distance(timestamp with time zone[],double precision[],timestamp with time zone[],double precision[],jsonb)'::regprocedure),
    true,
    'dtw_distance is strict'
);
SELECT has_function('public', 'dtw_canonicalize_sequence', ARRAY['timestamp with time zone[]','double precision[]','interval']);

SELECT ok(
    dtw_distance(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1.0]::double precision[],
        '{}'::jsonb
    ) = 0.0,
    'identical singleton sequences produce zero distance'
);

SELECT ok(pg_temp.raises_nan_rejection(), 'NaN is rejected');

SELECT ok(pg_temp.raises_itakura_rejection(), 'itakura is parsed and rejected in v1');

SELECT ok(
    dtw_distance_normalized(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0,3.0]::double precision[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:01+00'::timestamptz],
        ARRAY[1.0,1.0]::double precision[],
        '{}'::jsonb
    ) > 0.0,
    'normalized distance is positive for differing series'
);

SELECT ok(pg_temp.raises_unknown_option(), 'unknown option is rejected deterministically');

SELECT is(
    dtw_distance_int8(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1]::bigint[],
        ARRAY['2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1]::bigint[],
        '{}'::jsonb
    ),
    0.0::double precision,
    'int8 wrapper coercion path works'
);

SELECT is(
    (SELECT vals_out[1] FROM dtw_canonicalize_sequence(
        ARRAY['2025-01-01 00:00:00+00'::timestamptz, '2025-01-01 00:00:00+00'::timestamptz],
        ARRAY[1.0, 3.0]::double precision[],
        interval '1 microsecond'
    )),
    2.0::double precision,
    'canonicalization helper averages duplicates'
);

SELECT * FROM finish();
