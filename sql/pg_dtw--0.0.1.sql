CREATE FUNCTION dtw_distance(
    ts1 timestamptz[],
    vals1 double precision[],
    ts2 timestamptz[],
    vals2 double precision[],
    options jsonb DEFAULT '{}'::jsonb
) RETURNS double precision
AS 'MODULE_PATHNAME', 'dtw_distance'
LANGUAGE C
IMMUTABLE
PARALLEL SAFE
STRICT;

CREATE FUNCTION dtw_distance_normalized(
    ts1 timestamptz[],
    vals1 double precision[],
    ts2 timestamptz[],
    vals2 double precision[],
    options jsonb DEFAULT '{}'::jsonb
) RETURNS double precision
AS 'MODULE_PATHNAME', 'dtw_distance_normalized'
LANGUAGE C
IMMUTABLE
PARALLEL SAFE
STRICT;

CREATE FUNCTION dtw_lb_kim(
    vals1 double precision[],
    vals2 double precision[]
) RETURNS double precision
AS 'MODULE_PATHNAME', 'dtw_lb_kim'
LANGUAGE C
IMMUTABLE
PARALLEL SAFE
STRICT;

CREATE FUNCTION dtw_distance(
    ts1 timestamp[],
    vals1 double precision[],
    ts2 timestamp[],
    vals2 double precision[],
    options jsonb DEFAULT '{}'::jsonb
) RETURNS double precision
LANGUAGE SQL
IMMUTABLE
PARALLEL SAFE
RETURNS NULL ON NULL INPUT
AS $$
    SELECT dtw_distance(
        ARRAY(SELECT t::timestamptz FROM unnest(ts1) WITH ORDINALITY AS u(t, ord) ORDER BY ord),
        vals1,
        ARRAY(SELECT t::timestamptz FROM unnest(ts2) WITH ORDINALITY AS u(t, ord) ORDER BY ord),
        vals2,
        options
    );
$$;

CREATE FUNCTION dtw_distance_normalized(
    ts1 timestamp[],
    vals1 double precision[],
    ts2 timestamp[],
    vals2 double precision[],
    options jsonb DEFAULT '{}'::jsonb
) RETURNS double precision
LANGUAGE SQL
IMMUTABLE
PARALLEL SAFE
RETURNS NULL ON NULL INPUT
AS $$
    SELECT dtw_distance_normalized(
        ARRAY(SELECT t::timestamptz FROM unnest(ts1) WITH ORDINALITY AS u(t, ord) ORDER BY ord),
        vals1,
        ARRAY(SELECT t::timestamptz FROM unnest(ts2) WITH ORDINALITY AS u(t, ord) ORDER BY ord),
        vals2,
        options
    );
$$;

CREATE FUNCTION dtw_distance_int8(
    ts1 timestamptz[],
    vals1 bigint[],
    ts2 timestamptz[],
    vals2 bigint[],
    options jsonb DEFAULT '{}'::jsonb
) RETURNS double precision
LANGUAGE SQL
IMMUTABLE
PARALLEL SAFE
RETURNS NULL ON NULL INPUT
AS $$
    SELECT dtw_distance(
        ts1,
        ARRAY(SELECT x::double precision FROM unnest(vals1) WITH ORDINALITY AS u(x, ord) ORDER BY ord),
        ts2,
        ARRAY(SELECT x::double precision FROM unnest(vals2) WITH ORDINALITY AS u(x, ord) ORDER BY ord),
        options
    );
$$;

CREATE FUNCTION dtw_distance_numeric(
    ts1 timestamptz[],
    vals1 numeric[],
    ts2 timestamptz[],
    vals2 numeric[],
    options jsonb DEFAULT '{}'::jsonb
) RETURNS double precision
LANGUAGE SQL
IMMUTABLE
PARALLEL SAFE
RETURNS NULL ON NULL INPUT
AS $$
    SELECT dtw_distance(
        ts1,
        ARRAY(SELECT x::double precision FROM unnest(vals1) WITH ORDINALITY AS u(x, ord) ORDER BY ord),
        ts2,
        ARRAY(SELECT x::double precision FROM unnest(vals2) WITH ORDINALITY AS u(x, ord) ORDER BY ord),
        options
    );
$$;

CREATE FUNCTION dtw_canonicalize_sequence(
    ts_in timestamptz[],
    vals_in double precision[],
    duplicate_bucket interval DEFAULT interval '1 microsecond'
) RETURNS TABLE(ts_out timestamptz[], vals_out double precision[])
LANGUAGE plpgsql
IMMUTABLE
PARALLEL SAFE
RETURNS NULL ON NULL INPUT
AS $$
DECLARE
    bucket_us double precision;
BEGIN
    IF array_length(ts_in, 1) IS NULL OR array_length(vals_in, 1) IS NULL THEN
        RAISE EXCEPTION 'pg_dtw: input arrays must be non-empty';
    END IF;
    IF array_length(ts_in, 1) <> array_length(vals_in, 1) THEN
        RAISE EXCEPTION 'pg_dtw: timestamp/value array lengths must match';
    END IF;
    IF duplicate_bucket <= interval '0' THEN
        RAISE EXCEPTION 'pg_dtw: duplicate_bucket must be > 0';
    END IF;

    bucket_us := EXTRACT(EPOCH FROM duplicate_bucket) * 1000000.0;

    RETURN QUERY
    WITH expanded AS (
        SELECT t.ts, v.val
        FROM unnest(ts_in) WITH ORDINALITY AS t(ts, ord)
        JOIN unnest(vals_in) WITH ORDINALITY AS v(val, ord) USING (ord)
    ),
    grouped AS (
        SELECT
            floor((EXTRACT(EPOCH FROM ts) * 1000000.0) / bucket_us)::bigint AS bucket_key,
            avg(val)::double precision AS agg_val
        FROM expanded
        GROUP BY 1
    )
    SELECT
        array_agg(
            to_timestamp((bucket_key * bucket_us) / 1000000.0)::timestamptz
            ORDER BY bucket_key
        ) AS ts_out,
        array_agg(agg_val ORDER BY bucket_key) AS vals_out
    FROM grouped;
END;
$$;
