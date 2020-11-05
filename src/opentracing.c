/***
 * Copyright 2019,2020 HAProxy Technologies
 *
 * This file is part of spoa-opentracing.
 *
 * spoa-opentracing is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * spoa-opentracing is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "include.h"


#ifdef DEBUG

static void ot_text_map_show(const struct otc_text_map *text_map)
{
	DBG_FUNC(NULL, "%p", text_map);

	if (_NULL(text_map))
		return;

	OT_DBG_TEXT_MAP("", text_map);

	if (_nNULL(text_map->key) && _nNULL(text_map->value) && (text_map->count > 0)) {
		size_t i;

		for (i = 0; i < text_map->count; i++)
			W_DBG(NOTICE, NULL, "    \"%s\" -> \"%s\"", text_map->key[i], text_map->value[i]);
	}
}

#endif /* DEBUG */


/***
 * NAME
 *   ot_init -
 *
 * ARGUMENTS
 *   This function takes no arguments.
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_init(void)
{
	char errbuf[BUFSIZ];
	int  retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "");

	if (_NULL(cfg.ot_plugin))
		(void)fprintf(stderr, "ERROR: the OpenTracing library not specified\n");
	else if (_NULL(cfg.ot_config))
		(void)fprintf(stderr, "ERROR: the OpenTracing configuration file not specified\n");
	else if (_NULL(cfg.ot_tracer = otc_tracer_init(cfg.ot_plugin, cfg.ot_config, NULL, errbuf, sizeof(errbuf))))
		(void)fprintf(stderr, "ERROR: %s\n", (*errbuf == '\0') ? "Unable to initialize tracing library" : errbuf);
	else
		retval = FUNC_RET_OK;

	return retval;
}


/***
 * NAME
 *   ot_close -
 *
 * ARGUMENTS
 *   This function takes no arguments.
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void ot_close(void)
{
	DBG_FUNC(NULL, "");

	if (_NULL(cfg.ot_tracer))
		return;

	cfg.ot_tracer->close(cfg.ot_tracer);

	cfg.ot_tracer = NULL;
}


/***
 * NAME
 *   ot_statistics -
 *
 * ARGUMENTS
 *   This function takes no arguments.
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void ot_statistics(void)
{
	char buffer[BUFSIZ];

	if (_NULL(cfg.ot_tracer))
		return;

	otc_statistics(buffer, sizeof(buffer));
	w_log(NULL, "%s", buffer);
}


/***
 * NAME
 *   ot_span_init -
 *
 * ARGUMENTS
 *   operation_name -
 *   ts_steady      -
 *   ts_system      -
 *   ref_type       -
 *   ref_ctx_idx    -
 *   ref_span       -
 *   tags           -
 *   num_tags       -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span *ot_span_init(const char *operation_name, const struct timespec *ts_steady, const struct timespec *ts_system, int ref_type, int ref_ctx_idx, const struct otc_span *ref_span, const struct otc_tag *tags, int num_tags)
{
	struct otc_start_span_options  options;
	struct otc_span_context        context = { .idx = ref_ctx_idx, .span = ref_span };
	struct otc_span_reference      references = { ref_type, &context };
	struct otc_span               *retptr = NULL;

	DBG_FUNC(NULL, "\"%s\", %p, %p, %d, %d, %p, %p, %d", operation_name, ts_steady, ts_system, ref_type, ref_ctx_idx, ref_span, tags, num_tags);

	if (_NULL(operation_name))
		return retptr;
	else if (_NULL(cfg.ot_tracer))
		return retptr;

	(void)memset(&options, 0, sizeof(options));

	if (_nNULL(ts_steady))
		(void)memcpy(&(options.start_time_steady.value), ts_steady, sizeof(options.start_time_steady.value));

	if (_nNULL(ts_system))
		(void)memcpy(&(options.start_time_system.value), ts_system, sizeof(options.start_time_system.value));

	if (IN_RANGE(ref_type, otc_span_reference_child_of, otc_span_reference_follows_from)) {
		options.references     = &references;
		options.num_references = 1;
	}

	options.tags     = tags;
	options.num_tags = num_tags;

	retptr = cfg.ot_tracer->start_span_with_options(cfg.ot_tracer, operation_name, &options);
	if (_NULL(retptr))
		OT_DBG("cannot init new span");
	else
		OT_DBG("span %p:%zd initialized", retptr, retptr->idx);

	return retptr;
}


/***
 * NAME
 *   ot_span_init_va -
 *
 * ARGUMENTS
 *   operation_name -
 *   ts_steady      -
 *   ts_system      -
 *   ref_type       -
 *   ref_ctx_idx    -
 *   ref_span       -
 *   tag_key        -
 *   tag_value      -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span *ot_span_init_va(const char *operation_name, const struct timespec *ts_steady, const struct timespec *ts_system, int ref_type, int ref_ctx_idx, const struct otc_span *ref_span, const char *tag_key, const char *tag_value, ...)
{
	struct otc_tag   tags[OT_MAXTAGS];
	int              num_tags = 0;
	struct otc_span *retptr = NULL;

	DBG_FUNC(NULL, "\"%s\", %p, %p, %d, %d, %p, \"%s\", \"%s\", ...", operation_name, ts_steady, ts_system, ref_type, ref_ctx_idx, ref_span, tag_key, tag_value);

	if (_nNULL(tag_key)) {
		va_list ap;

		va_start(ap, tag_value);
		for (num_tags = 0; (num_tags < TABLESIZE(tags)) && _nNULL(tag_key) && _nNULL(tag_value); num_tags++) {
			tags[num_tags].key = (char *)tag_key;
			OT_VSET(&(tags[num_tags].value), string, tag_value);

			if (_nNULL(tag_key = va_arg(ap, typeof(tag_key))))
				tag_value = va_arg(ap, typeof(tag_value));
		}
		va_end(ap);
	}

	retptr = ot_span_init(operation_name, ts_steady, ts_system, ref_type, ref_ctx_idx, ref_span, tags, num_tags);

	return retptr;
}


/***
 * NAME
 *   ot_span_tag -
 *
 * ARGUMENTS
 *   span     -
 *   tags     -
 *   num_tags -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_span_tag(struct otc_span *span, const struct otc_tag *tags, int num_tags)
{
	int retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "%p, %p, %d", span, tags, num_tags);

	if (_NULL(span) || _NULL(tags))
		return retval;
	else if (_NULL(cfg.ot_tracer))
		return retval;

	for (retval = 0; retval < num_tags; retval++)
		span->set_tag(span, tags[retval].key, &(tags[retval].value));

	return retval;
}


/***
 * NAME
 *   ot_span_tag_va -
 *
 * ARGUMENTS
 *   span  -
 *   key   -
 *   type  -
 *   value -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_span_tag_va(struct otc_span *span, const char *key, int type, ...)
{
	va_list          ap;
	struct otc_value ot_value;
	int              retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "%p, \"%s\", %d, ...", span, key, type);

	if (_NULL(span) || _NULL(key))
		return retval;
	else if (_NULL(cfg.ot_tracer))
		return retval;

	va_start(ap, type);
	for (retval = 0; _nNULL(key) && IN_RANGE(type, otc_value_bool, otc_value_null); retval++) {
		ot_value.type = type;
		if (type == otc_value_bool)
			ot_value.value.bool_value = va_arg(ap, typeof(ot_value.value.bool_value));
		else if (type == otc_value_double)
			ot_value.value.double_value = va_arg(ap, typeof(ot_value.value.double_value));
		else if (type == otc_value_int64)
			ot_value.value.int64_value = va_arg(ap, typeof(ot_value.value.int64_value));
		else if (type == otc_value_uint64)
			ot_value.value.uint64_value = va_arg(ap, typeof(ot_value.value.uint64_value));
		else if (type == otc_value_string)
			ot_value.value.string_value = va_arg(ap, typeof(ot_value.value.string_value));
		else if (type == otc_value_null)
			ot_value.value.string_value = va_arg(ap, typeof(ot_value.value.string_value));
		span->set_tag(span, key, &ot_value);

		if (_nNULL(key = va_arg(ap, typeof(key))))
			type = va_arg(ap, typeof(type));
	}
	va_end(ap);

	return retval;
}


/***
 * NAME
 *   ot_span_log -
 *
 * ARGUMENTS
 *   span       -
 *   log_fields -
 *   num_fields -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_span_log(struct otc_span *span, const struct otc_log_field *log_fields, int num_fields)
{
	int retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "%p, %p, %d", span, log_fields, num_fields);

	if (_NULL(span) || _NULL(log_fields))
		return retval;
	else if (_NULL(cfg.ot_tracer))
		return retval;

	retval = MIN(OTC_MAXLOGFIELDS, num_fields);

	span->log_fields(span, log_fields, retval);

	return retval;
}


/***
 * NAME
 *   ot_span_log_va -
 *
 * ARGUMENTS
 *   span  -
 *   key   -
 *   value -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_span_log_va(struct otc_span *span, const char *key, const char *value, ...)
{
	va_list              ap;
	struct otc_log_field log_field[OTC_MAXLOGFIELDS];
	int                  retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "%p, \"%s\", \"%s\", ...", span, key, value);

	if (_NULL(span) || _NULL(key) || _NULL(value))
		return retval;
	else if (_NULL(cfg.ot_tracer))
		return retval;

	va_start(ap, value);
	for (retval = 0; (retval < TABLESIZE(log_field)) && _nNULL(key); retval++) {
		log_field[retval].key                      = key;
		log_field[retval].value.type               = otc_value_string;
		log_field[retval].value.value.string_value = value;

		if (_nNULL(key = va_arg(ap, typeof(key))))
			value = va_arg(ap, typeof(value));
	}
	va_end(ap);

	span->log_fields(span, log_field, retval);

	return retval;
}


/***
 * NAME
 *   ot_span_log_fmt -
 *
 * ARGUMENTS
 *   span   -
 *   key    -
 *   format -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_span_log_fmt(struct otc_span *span, const char *key, const char *format, ...)
{
	va_list ap;
	char    value[BUFSIZ];
	int     n;

	DBG_FUNC(NULL, "%p, \"%s\", \"%s\", ...", span, key, format);

	if (_NULL(span) || _NULL(key) || _NULL(format))
		return FUNC_RET_ERROR;
	else if (_NULL(cfg.ot_tracer))
		return FUNC_RET_ERROR;

	va_start(ap, format);
	n = vsnprintf(value, sizeof(value), format, ap);
	if (!IN_RANGE(n, 0, (int)SIZEOF_N(value, 1))) {
		OT_DBG("log buffer too small (%d > %zu)", n, sizeof(value));

		STR_ELLIPSIS(value, sizeof(value));
	}
	va_end(ap);

	return ot_span_log_va(span, key, value, NULL);
}


/***
 * NAME
 *   ot_span_set_baggage -
 *
 * ARGUMENTS
 *   span    -
 *   baggage -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_span_set_baggage(struct otc_span *span, const struct otc_text_map *baggage)
{
	size_t i;
	int    retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "%p, %p", span, baggage);

	if (_NULL(span) || _NULL(baggage))
		return retval;
	else if (_NULL(cfg.ot_tracer))
		return retval;

	if (_NULL(baggage->key) || _NULL(baggage->value))
		return retval;

	for (retval = i = 0; i < baggage->count; i++) {
		OT_DBG("set baggage: \"%s\" -> \"%s\"", baggage->key[i], baggage->value[i]);

		if (_nNULL(baggage->key[i]) && _nNULL(baggage->value[i])) {
			span->set_baggage_item(span, baggage->key[i], baggage->value[i]);

			retval++;
		}
	}

	return retval;
}


/***
 * NAME
 *   ot_span_set_baggage_va -
 *
 * ARGUMENTS
 *   span  -
 *   key   -
 *   value -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_span_set_baggage_va(struct otc_span *span, const char *key, const char *value, ...)
{
	va_list ap;
	int     retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "%p, \"%s\", \"%s\", ...", span, key, value);

	if (_NULL(span) || _NULL(key) || _NULL(value))
		return retval;
	else if (_NULL(cfg.ot_tracer))
		return retval;

	va_start(ap, value);
	for (retval = 0; _nNULL(key); retval++) {
		OT_DBG("set baggage: \"%s\" -> \"%s\"", key, value);

		span->set_baggage_item(span, key, value);

		if (_nNULL(key = va_arg(ap, typeof(key))))
			value = va_arg(ap, typeof(value));
	}
	va_end(ap);

	return retval;
}


/***
 * NAME
 *   ot_span_baggage_va -
 *
 * ARGUMENTS
 *   span -
 *   key  -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_text_map *ot_span_baggage_va(const struct otc_span *span, const char *key, ...)
{
	va_list              ap;
	struct otc_text_map *retptr = NULL;
	int                  i, n;

	DBG_FUNC(NULL, "%p, \"%s\", ...", span, key);

	if (_NULL(span) || _NULL(key))
		return retptr;
	else if (_NULL(cfg.ot_tracer))
		return retptr;

	va_start(ap, key);
	for (n = 1; _nNULL(va_arg(ap, typeof(key))); n++);
	va_end(ap);

	if (_NULL(retptr = otc_text_map_new(NULL, n)))
		return retptr;

	va_start(ap, key);
	for (i = 0; (i < n) && _nNULL(key); i++) {
		char *value;

		if (_nNULL(value = (char *)span->baggage_item(span, key))) {
			(void)otc_text_map_add(retptr, key, 0, value, 0, OTC_TEXT_MAP_DUP_KEY | OTC_TEXT_MAP_DUP_VALUE);

			OT_DBG("get baggage[%d]: \"%s\" -> \"%s\"", i, retptr->key[i], retptr->value[i]);
		} else {
			OT_DBG("get baggage[%d]: \"%s\" -> invalid key", i, key);
		}

		key = va_arg(ap, typeof(key));
	}
	va_end(ap);

	return retptr;
}


/***
 * NAME
 *   ot_inject_text_map -
 *
 * ARGUMENTS
 *   span    -
 *   carrier -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_inject_text_map(const struct otc_span *span, struct otc_text_map_writer *carrier)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	DBG_FUNC(NULL, "%p, %p", span, carrier);

	if (_NULL(span) || _NULL(carrier))
		return retptr;
	else if (_NULL(cfg.ot_tracer))
		return retptr;

	if (_NULL(retptr = span->span_context((struct otc_span *)span)))
		return retptr;

	(void)memset(carrier, 0, sizeof(*carrier));

	rc = cfg.ot_tracer->inject_text_map(cfg.ot_tracer, carrier, retptr);
	if (rc != otc_propagation_error_code_success) {
		OT_FREE_CLEAR(retptr);
	} else {
#ifdef DEBUG
		OT_DBG_TEXT_CARRIER("", carrier, set);
		ot_text_map_show(&(carrier->text_map));
		OT_DBG_SPAN_CONTEXT("", retptr);
#endif
	}

	return retptr;
}


/***
 * NAME
 *   ot_inject_http_headers -
 *
 * ARGUMENTS
 *   span    -
 *   carrier -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_inject_http_headers(const struct otc_span *span, struct otc_http_headers_writer *carrier)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	DBG_FUNC(NULL, "%p, %p", span, carrier);

	if (_NULL(span) || _NULL(carrier))
		return retptr;
	else if (_NULL(cfg.ot_tracer))
		return retptr;

	if (_NULL(retptr = span->span_context((struct otc_span *)span)))
		return retptr;

	(void)memset(carrier, 0, sizeof(*carrier));

	rc = cfg.ot_tracer->inject_http_headers(cfg.ot_tracer, carrier, retptr);
	if (rc != otc_propagation_error_code_success) {
		OT_FREE_CLEAR(retptr);
	} else {
#ifdef DEBUG
		OT_DBG_TEXT_CARRIER("", carrier, set);
		ot_text_map_show(&(carrier->text_map));
		OT_DBG_SPAN_CONTEXT("", retptr);
#endif
	}

	return retptr;
}


/***
 * NAME
 *   ot_inject_binary -
 *
 * ARGUMENTS
 *   span    -
 *   carrier -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_inject_binary(const struct otc_span *span, struct otc_custom_carrier_writer *carrier)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	DBG_FUNC(NULL, "%p, %p", span, carrier);

	if (_NULL(span) || _NULL(carrier))
		return retptr;
	else if (_NULL(cfg.ot_tracer))
		return retptr;

	if (_NULL(retptr = span->span_context((struct otc_span *)span)))
		return retptr;

	(void)memset(carrier, 0, sizeof(*carrier));

	rc = cfg.ot_tracer->inject_binary(cfg.ot_tracer, carrier, retptr);
	if (rc != otc_propagation_error_code_success) {
		OT_FREE_CLEAR(retptr);
	} else {
#ifdef DEBUG
		struct otc_jaeger_trace_context *ctx = carrier->binary_data.data;

		OT_DBG_CUSTOM_CARRIER("", carrier, inject);
		OT_DBG("trace context: %016" PRIx64 "%016" PRIx64 ":%016" PRIx64 ":%016" PRIx64 ":%02hhx <%s> <%s>",
		       ctx->trace_id[0], ctx->trace_id[1], ctx->span_id, ctx->parent_span_id, ctx->flags,
		       str_hex(ctx->baggage, carrier->binary_data.size - sizeof(*ctx)),
		       str_ctrl(ctx->baggage, carrier->binary_data.size - sizeof(*ctx)));
		OT_DBG_SPAN_CONTEXT("", retptr);
#endif
	}

	return retptr;
}


/***
 * NAME
 *   ot_extract_text_map -
 *
 * ARGUMENTS
 *   carrier  -
 *   text_map -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_extract_text_map(struct otc_text_map_reader *carrier, const struct otc_text_map *text_map)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	DBG_FUNC(NULL, "%p, %p", carrier, text_map);

	if (_NULL(carrier))
		return retptr;
	else if (_NULL(cfg.ot_tracer))
		return retptr;

	if (_nNULL(text_map)) {
		(void)memset(carrier, 0, sizeof(*carrier));
		(void)memcpy(&(carrier->text_map), text_map, sizeof(carrier->text_map));

		OT_DBG_TEXT_CARRIER("", carrier, foreach_key);
	}

	rc = cfg.ot_tracer->extract_text_map(cfg.ot_tracer, carrier, &retptr);
	if (rc != otc_propagation_error_code_success)
		OT_FREE_CLEAR(retptr);
	else if (_nNULL(retptr))
		OT_DBG_SPAN_CONTEXT("", retptr);

	return retptr;
}


/***
 * NAME
 *   ot_extract_http_headers -
 *
 * ARGUMENTS
 *   carrier  -
 *   text_map -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_extract_http_headers(struct otc_http_headers_reader *carrier, const struct otc_text_map *text_map)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	DBG_FUNC(NULL, "%p, %p", carrier, text_map);

	if (_NULL(carrier))
		return retptr;
	else if (_NULL(cfg.ot_tracer))
		return retptr;

	if (_nNULL(text_map)) {
		(void)memset(carrier, 0, sizeof(*carrier));
		(void)memcpy(&(carrier->text_map), text_map, sizeof(carrier->text_map));

		OT_DBG_TEXT_CARRIER("", carrier, foreach_key);
	}

	rc = cfg.ot_tracer->extract_http_headers(cfg.ot_tracer, carrier, &retptr);
	if (rc != otc_propagation_error_code_success)
		OT_FREE_CLEAR(retptr);
	else if (_nNULL(retptr))
		OT_DBG_SPAN_CONTEXT("", retptr);

	return retptr;
}


/***
 * NAME
 *   ot_extract_binary -
 *
 * ARGUMENTS
 *   carrier -
 *   data    -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_extract_binary(struct otc_custom_carrier_reader *carrier, const struct otc_binary_data *binary_data)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	DBG_FUNC(NULL, "%p, %p", carrier, binary_data);

	if (_NULL(carrier))
		return retptr;
	else if (_NULL(cfg.ot_tracer))
		return retptr;

	if (_nNULL(binary_data) && _nNULL(binary_data->data) && (binary_data->size > 0)) {
		(void)memset(carrier, 0, sizeof(*carrier));
		(void)memcpy(&(carrier->binary_data), binary_data, sizeof(carrier->binary_data));

		OT_DBG_CUSTOM_CARRIER("", carrier, extract);
	}

	rc = cfg.ot_tracer->extract_binary(cfg.ot_tracer, carrier, &retptr);
	if (rc != otc_propagation_error_code_success)
		OT_FREE_CLEAR(retptr);
	else if (_nNULL(retptr))
		OT_DBG_SPAN_CONTEXT("", retptr);

	return retptr;
}


/***
 * NAME
 *   ot_span_finish -
 *
 * ARGUMENTS
 *   span      -
 *   ts_finish -
 *   log_ts    -
 *   log_key   -
 *   log_value -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void ot_span_finish(struct otc_span **span, const struct timespec *ts_finish, const struct timespec *log_ts, const char *log_key, const char *log_value, ...)
{
	struct otc_finish_span_options options;
	struct otc_log_field           log_field[OTC_MAXLOGFIELDS];
	struct otc_log_record          log_records = { .fields = log_field, .num_fields = 0 };
#ifdef DEBUG
	typeof((*span)->idx)                   idx = (_NULL(span) || _NULL(*span)) ? 0 : (*span)->idx;
#endif

	DBG_FUNC(NULL, "%p:%p, %p, %p, \"%s\", \"%s\", ...", DPTR_ARGS(span), ts_finish, log_ts, STR_SAFE(log_key), STR_SAFE(log_value));

	if (_NULL(span) || _NULL(*span))
		return;
	else if (_NULL(cfg.ot_tracer))
		return;

	(void)memset(&options, 0, sizeof(options));

	if (_nNULL(ts_finish))
		(void)memcpy(&(options.finish_time.value), ts_finish, sizeof(options.finish_time.value));

	if (_nNULL(log_key)) {
		va_list ap;
		int     i;

		if (_nNULL(log_ts))
			(void)memcpy(&(log_records.timestamp.value), log_ts, sizeof(log_records.timestamp.value));

		va_start(ap, log_value);
		for (i = 0; (i < TABLESIZE(log_field)) && _nNULL(log_key); i++) {
			log_field[i].key                      = log_key;
			log_field[i].value.type               = otc_value_string;
			log_field[i].value.value.string_value = log_value;

			if (_nNULL(log_key = va_arg(ap, typeof(log_key))))
				log_value = va_arg(ap, typeof(log_value));
		}
		va_end(ap);

		log_records.num_fields  = i;
		options.log_records     = &log_records;
		options.num_log_records = 1;
	}

	/*
	 * Caution: memory allocated for the span is released
	 *          in the function finish_with_options().
	 */
	(*span)->finish_with_options(*span, &options);

	OT_DBG("span %p:%zu finished", *span, idx);

	*span = NULL;
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
