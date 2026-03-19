#ifndef PG_DTW_OPTIONS_H
#define PG_DTW_OPTIONS_H

#include "postgres.h"
#include "utils/jsonb.h"

#include "pg_dtw/types.h"

void dtw_options_init_defaults(DtwOptions *opts);
void dtw_parse_options(Jsonb *jsonb, DtwOptions *opts);

#endif
