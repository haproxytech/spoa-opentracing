/***
 * Copyright 2018-2020 HAProxy Technologies
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

/***
 * NAME
 *   ot_runtime_context_check -
 *
 * ARGUMENTS
 *   ctx -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_runtime_context_check(const struct ot_runtime_context *ctx)
{
	const struct ot_scope_span    *ot_span;
	const struct ot_scope_context *ot_context;
	uint                           nbspans = 0, nbcontexts = 0;

	DBG_FUNC(NULL, "%p", ctx);

	if (_NULL(ctx))
		return;

	list_for_each_entry(ot_span, &(ctx->spans), list)
		nbspans++;
	list_for_each_entry(ot_context, &(ctx->contexts), list)
		nbcontexts++;

	if ((ctx->nbspans == nbspans) && (ctx->nbcontexts == nbcontexts))
		return;

	OT_DBG_RUNTIME_CONTEXT("invalid rt_CTX content ", ctx);

	if (ctx->nbspans != nbspans) {
		OT_DBG("  incorrect number of SPANS, %u instead of %u", nbspans, ctx->nbspans);

		list_for_each_entry(ot_span, &(ctx->spans), list)
			OT_DBG("  SPAN '%.*s'", OT_STR_HDR_ARGS(ot_span, id));
	}

	if (ctx->nbcontexts != nbcontexts) {
		OT_DBG("  incorrect number of SPAN CONTEXTS, %u instead of %u", nbcontexts, ctx->nbcontexts);

		list_for_each_entry(ot_context, &(ctx->contexts), list)
			OT_DBG("  SPAN CONTEXT '%.*s'", OT_STR_HDR_ARGS(ot_context, id));
	}
}

#endif /* DEBUG */


/***
 * NAME
 *   ot_spoe_data_to_value -
 *
 * ARGUMENTS
 *   data  -
 *   type  -
 *   value -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static int ot_spoe_data_to_value(const union spoe_data *data, enum spoe_data_type type, struct otc_value *value)
{
	static __THR char retbuf[MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
	int               retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "%p, %d, %p", data, type, value);

	if (_NULL(data) || _NULL(value))
		return retval;

	if (type == SPOE_DATA_T_NULL) {
		value->type               = otc_value_null;
		value->value.string_value = "";

		retval = 0;
	}
	else if (type == SPOE_DATA_T_BOOL) {
		value->type             = otc_value_bool;
		value->value.bool_value = data->boolean;

		retval = sizeof(value->value.bool_value);
	}
	else if (type == SPOE_DATA_T_INT32) {
		value->type              = otc_value_int64;
		value->value.int64_value = data->int32;

		retval = sizeof(value->value.int64_value);
	}
	else if (type == SPOE_DATA_T_UINT32) {
		value->type               = otc_value_uint64;
		value->value.uint64_value = data->uint32;

		retval = sizeof(value->value.uint64_value);
	}
	else if (type == SPOE_DATA_T_INT64) {
		value->type              = otc_value_int64;
		value->value.int64_value = data->int64;

		retval = sizeof(value->value.int64_value);
	}
	else if (type == SPOE_DATA_T_UINT64) {
		value->type               = otc_value_uint64;
		value->value.uint64_value = data->uint64;

		retval = sizeof(value->value.uint64_value);
	}
	else if (type == SPOE_DATA_T_IPV4) {
		if (_nNULL(inet_ntop(AF_INET, &(data->ipv4), retbuf, INET_ADDRSTRLEN))) {
			value->type               = otc_value_string;
			value->value.string_value = retbuf;

			retval = strlen(value->value.string_value);
		}
	}
	else if (type == SPOE_DATA_T_IPV6) {
		if (_nNULL(inet_ntop(AF_INET6, &(data->ipv6), retbuf, INET6_ADDRSTRLEN))) {
			value->type               = otc_value_string;
			value->value.string_value = retbuf;

			retval = strlen(value->value.string_value);
		}
	}
	else if (type == SPOE_DATA_T_STR) {
		value->type               = otc_value_string;
		value->value.string_value = data->chk.ptr;

		retval = data->chk.len;
	}
	else if (type == SPOE_DATA_T_BIN) {
		OT_DBG("Unsupported data type: %d", type);
	}
	else {
		OT_DBG("Invalid data type: %d", type);
	}

	return retval;
}


/***
 * NAME
 *   ot_value_to_spoe_data_str -
 *
 * ARGUMENTS
 *   value    -
 *   data_str -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static int ot_value_to_spoe_data_str(const struct otc_value *value, union spoe_data *data_str)
{
	static __THR char retbuf[32];
	int               retval = FUNC_RET_OK;

	if (_NULL(value) || _NULL(data_str))
		return FUNC_RET_ERROR;

	data_str->chk.ptr = retbuf;

	if (value->type == otc_value_bool) {
		retbuf[0] = value->value.bool_value ? '1' : '0';
		retbuf[1] = '\0';
	}
	else if (value->type == otc_value_double) {
		(void)snprintf(retbuf, sizeof(retbuf), "%f", value->value.double_value);
	}
	else if (value->type == otc_value_int64) {
		(void)snprintf(retbuf, sizeof(retbuf), "%"PRId64, value->value.int64_value);
	}
	else if (value->type == otc_value_uint64) {
		(void)snprintf(retbuf, sizeof(retbuf), "%"PRIu64, value->value.uint64_value);
	}
	else if (value->type == otc_value_string) {
		data_str->chk.ptr = (typeof(data_str->chk.ptr))value->value.string_value;
	}
	else if (value->type == otc_value_null) {
		retbuf[0] = '\0';
	}
	else {
		OT_DBG("Invalid data type: %d", value->type);

		retval = FUNC_RET_ERROR;
	}

	if (_nERROR(retval))
		retval = data_str->chk.len = strlen(data_str->chk.ptr);

	return retval;
}


/***
 * NAME
 *   ot_scope_span_finish_mark -
 *
 * ARGUMENTS
 *   ctx    -
 *   span   -
 *   id     -
 *   id_len -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_scope_span_finish_mark(struct ot_runtime_context *ctx, struct ot_scope_span *span, const char *id, size_t id_len)
{
	OT_MUTEX_LOCK_INIT();
	struct ot_scope_span *ot_span;
	int                   retval = 0;

	DBG_FUNC(NULL, "%p, %p, \"%.*s\", %zu", ctx, span, OT_STR_ARGS(id));

	if (id_len == 0) {
		if (_nNULL(span))
			span->flag_finish = 1;

		OT_DBG("current SPAN%s marked for completion", _NULL(span) ? " unavailable, not" : "");
	}
	else if (STR_CMP_EQ(OT_SCOPE_FINISH_ALL, id, id_len)) {
		OT_MUTEX_LOCK(&(prg.ot_mutex), FUNC_RET_ERROR, );

		list_for_each_entry(ot_span, &(ctx->spans), list) {
			ot_span->flag_finish = 1;
			retval++;
		}

		OT_DBG("%d SPAN(s) marked for completion", retval);
	}
	else {
		OT_MUTEX_LOCK(&(prg.ot_mutex), FUNC_RET_ERROR, );

		list_for_each_entry(ot_span, &(ctx->spans), list)
			if (OT_SCOPE_SPAN_CHECK_ID(ot_span, id, id_len)) {
				ot_span->flag_finish = 1;
				retval++;

				break;
			}

		OT_DBG("SPAN '%.*s'%s marked for completion", OT_STRN_ARGS(id), (retval > 0) ? "" : " unavailable, not");
	}

	OT_MUTEX_UNLOCK(&(prg.ot_mutex), FUNC_RET_ERROR, );

	return retval;
}


/***
 * NAME
 *   ot_scope_span_finish_marked -
 *
 * ARGUMENTS
 *   ctx       -
 *   ts_finish -
 *
 * DESCRIPTION
 *   Finish marked spans.
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_scope_span_finish_marked(const struct ot_runtime_context *ctx, const struct timespec *ts_finish)
{
	OT_MUTEX_LOCK_INIT();
	struct ot_scope_span *ot_span;

	DBG_FUNC(NULL, "%p, %p", ctx, ts_finish);

	OT_MUTEX_LOCK(&(prg.ot_mutex), , );

	list_for_each_entry(ot_span, &(ctx->spans), list)
		if (ot_span->flag_finish) {
			OT_DBG("finishing marked SPAN '%.*s' '%.*s' %p", OT_STR_HDR_ARGS(ctx, haproxy_id), OT_STR_HDR_ARGS(ot_span, id), ot_span);

			ot_span_finish(&(ot_span->span), ts_finish, NULL, NULL, NULL);

			ot_span->flag_finish = 0;
			ot_span->flag_unused = 1;
		}

	OT_MUTEX_UNLOCK(&(prg.ot_mutex), , );
}


/***
 * NAME
 *   ot_scope_span_init -
 *
 * ARGUMENTS
 *   ctx        -
 *   data       -
 *   ref_type   -
 *   ref_id     -
 *   ref_id_len -
 *   client     -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct ot_scope_span *ot_scope_span_init(struct ot_runtime_context *ctx, const struct ot_scope_data *data, otc_span_reference_type_t ref_type, const char *ref_id, size_t ref_id_len, const struct client *client)
{
	OT_MUTEX_LOCK_INIT();
	const struct otc_span         *ref_span = NULL;
	const struct otc_span_context *ref_ctx = NULL;
	const struct ot_scope_span    *ot_span;
	const struct ot_scope_context *ot_ctx;
	struct ot_scope_span          *retptr = NULL;

	DBG_FUNC(CW_PTR, "%p, %p, %d, \"%.*s\", %zu, %p", ctx, data, ref_type, OT_STR_ARGS(ref_id), client);

	if (_NULL(ctx))
		return retptr;

	OT_MUTEX_LOCK(&(prg.ot_mutex), NULL, );

#ifdef DEBUG
	ot_runtime_context_check(ctx);
#endif

	/* Maybe the span was created before, in which case we'll find it here. */
	list_for_each_entry(ot_span, &(ctx->spans), list)
		if (OT_SCOPE_SPAN_CHECK_ID(ot_span, data->id, data->id_len)) {
			OT_DBG("SPAN found '%.*s' '%.*s' %p", OT_STR_HDR_ARGS(ctx, haproxy_id), OT_STR_HDR_ARGS(ot_span, id), ot_span);

			OT_MUTEX_UNLOCK(&(prg.ot_mutex), NULL, );

			return (typeof(retptr))ot_span;
		}

	if (_nNULL(ref_id)) {
		/* If the referenced span is not found, it is an error. */
		list_for_each_entry(ot_span, &(ctx->spans), list)
			if (OT_SCOPE_SPAN_CHECK_ID(ot_span, ref_id, ref_id_len)) {
				ref_span = ot_span->span;

				break;
			}

		if (_nNULL(ref_span)) {
			OT_DBG("referenced SPAN found '%.*s' '%.*s' %p", OT_STR_HDR_ARGS(ctx, haproxy_id), OT_STR_HDR_ARGS(ot_span, id), ot_span);
		} else {
			/*
			 * Span contexts are not used, only a NULL value can be
			 * returned here.
			 */
			list_for_each_entry(ot_ctx, &(ctx->contexts), list)
				if ((ot_ctx->id_len == ref_id_len) && (memcmp(ot_ctx->id, ref_id, ref_id_len) == 0)) {
					ref_ctx = ot_ctx->context;

					break;
				}

			if (_nNULL(ref_ctx)) {
				OT_DBG("referenced SPAN CONTEXT found '%.*s' '%.*s' %p", OT_STR_HDR_ARGS(ctx, haproxy_id), OT_STR_HDR_ARGS(ot_ctx, id), ot_ctx);
			} else {
				OT_DBG("cannot find referenced SPAN/CONTEXT '%.*s' '%.*s'", OT_STR_HDR_ARGS(ctx, haproxy_id), OT_STRN_ARGS(ref_id));

#ifdef DEBUG
				ot_runtime_context_check(ctx);
#endif

				OT_MUTEX_UNLOCK(&(prg.ot_mutex), NULL, );

				return retptr;
			}
		}
	}

	if (_NULL(retptr = OTC_DBG_CALLOC(1, sizeof(*retptr)))) {
		/* Do nothing. */
	}
	else if (_NULL(retptr->id = OTC_DBG_MEMDUP(data->id, data->id_len))) {
		OT_FREE_CLEAR(retptr);
	}
	else {
		retptr->id_len   = data->id_len;
		retptr->ref_type = ref_type;
		retptr->ref_span = ref_span;
		retptr->ref_ctx  = ref_ctx;
		retptr->client   = client;

		LIST_ADD(&(ctx->spans), &(retptr->list));
		ctx->nbspans++;
	}

	OT_MUTEX_UNLOCK(&(prg.ot_mutex), NULL, OT_SCOPE_SPAN_FREE(ctx, retptr));

	if (_nNULL(retptr)) {
		OT_DBG("new SPAN '%.*s' '%.*s' %p", OT_STR_HDR_ARGS(ctx, haproxy_id), OT_STR_HDR_ARGS(retptr, id), retptr);
		OT_DBG_SCOPE_SPAN("new SPAN ", retptr);
	}

	return retptr;
}


/***
 * NAME
 *   ot_scope_span_free -
 *
 * ARGUMENTS
 *   ctx  -
 *   span -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_scope_span_free(struct ot_runtime_context *ctx, struct ot_scope_span **span)
{
	DBG_FUNC(NULL, "%p, %p:%p", ctx, DPTR_ARGS(span));

	if (_NULL(span) || _NULL(*span))
		return;

	OT_DBG_SCOPE_SPAN("SPAN ", *span);

	/* If the span is still active, do nothing. */
	if (OT_SCOPE_SPAN_IS_USED(*span)) {
		OT_DBG(_W("Cannot finish active SPAN"));

		return;
	}

	OT_SCOPE_SPAN_FREE(ctx, *span);
}


/***
 * NAME
 *   ot_scope_data_free -
 *
 * ARGUMENTS
 *   data -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void ot_scope_data_free(struct ot_scope_data *data)
{
	int i;

	DBG_FUNC(NULL, "%p", data);

	if (_NULL(data))
		return;

	OT_DBG_SCOPE_DATA("data ", data);

	OT_FREE(data->id);
	for (i = 0; i < data->num_tags; i++) {
		OT_FREE(data->tags[i].key);
		if (data->tags[i].value.type == otc_value_string)
			OT_FREE(data->tags[i].value.value.string_value);
	}
	otc_text_map_destroy(&(data->baggage), OTC_TEXT_MAP_FREE_KEY | OTC_TEXT_MAP_FREE_VALUE);
	for (i = 0; i < data->num_log_fields; i++) {
		OT_FREE(data->log_fields[i].key);
		if (data->log_fields[i].value.type == otc_value_string)
			OT_FREE(data->log_fields[i].value.value.string_value);
	}

	(void)memset(data, 0, sizeof(*data));
}


/***
 * NAME
 *   ot_scope_data_add_tag -
 *
 * ARGUMENTS
 *   span     -
 *   flag_new -
 *   data_str -
 *   data     -
 *   type     -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_scope_data_add_tag(struct ot_scope_data *span, bool_t flag_new, const union spoe_data *data_str, const union spoe_data *data, enum spoe_data_type type)
{
	struct otc_tag *tag;
	struct buffer  *buffer;
	int             retval = FUNC_RET_OK;

	DBG_FUNC(NULL, "%p, %hhu, %p, %p, %d", span, flag_new, data_str, data, type);

	if (_NULL(span) || (!flag_new && _NULL(data_str)))
		return FUNC_RET_ERROR;

	tag    = span->tags + span->num_tags;
	buffer = &(span->tag_buffer);

	if (flag_new) {
		/*
		 * Checking if there is a previously created tag.  If any,
		 * it should be completed (ie memory allocated in the buffer
		 * should be converted to tag value).
		 */
		if (_nNULL(tag->key)) {
			if (_NULL(buffer->ptr)) {
				OT_DBG("tag[%d]: <%s> -> %d", span->num_tags, tag->key, tag->value.type);
			}
			else if (_nERROR(retval = buffer_grow(buffer, "\0", 1))) {
				OT_DBG("tag[%d]: <%s> -> <%s>", span->num_tags, tag->key, buffer->ptr);

				tag->value.type               = otc_value_string;
				tag->value.value.string_value = (const char *)buffer->ptr;
			}

			if (_nERROR(retval))
				tag = span->tags + ++span->num_tags;
		}

		/* Creating a new tag key. */
		if (_NULL(data_str) || _ERROR(retval))
			/* Do nothing. */;
		else if (_nNULL(tag->key = OTC_DBG_MEMDUP(data_str->chk.ptr, data_str->chk.len)))
			OT_DBG("tag[%d]: <%s>", span->num_tags, tag->key);
		else
			retval = FUNC_RET_ERROR;

		buffer_init(buffer);
		span->tag_args = 0;
	} else {
		/* Constructing tag value. */
		if (span->tag_args == 0) {
			/*
			 * If the tag contains only one piece of information,
			 * then it may not be necessary to convert it to a
			 * string.
			 *
			 * In case the final data type is still a string,
			 * memory is allocated for it so that the function
			 * ot_scope_data_free() works correctly.  The length
			 * of the string is saved in buffer->len, so that the
			 * strlen() function does not have to be called later.
			 */
			if (_ERROR(retval = ot_spoe_data_to_value(data, type, &(tag->value))))
				/* Do nothing. */;
			else if (tag->value.type != otc_value_string)
				/* Do nothing. */;
			else if (_NULL(tag->value.value.string_value = OTC_DBG_MEMDUP(tag->value.value.string_value, retval)))
				retval = FUNC_RET_ERROR;
			else
				buffer->len = retval;
		}
		else if ((span->tag_args == 1) && _NULL(buffer->ptr)) {
			union spoe_data str;

			/*
			 * This is not the first data for the current tag,
			 * which means that the value of the tag may need
			 * to be converted to a string.
			 */
			if (tag->value.type == otc_value_string) {
				retval = buffer_grow(buffer, tag->value.value.string_value, buffer->len);

				OT_FREE_CLEAR(tag->value.value.string_value);
			}
			else if (_nERROR(retval = ot_value_to_spoe_data_str(&(tag->value), &str)))
				retval = buffer_grow(buffer, str.chk.ptr, str.chk.len);
		}

		if (_nERROR(retval) && (span->tag_args > 0)) {
			retval = buffer_grow(buffer, data_str->chk.ptr, data_str->chk.len);

			OT_DBG("tag[%d]: <%s> -> <%.*s>", span->num_tags, tag->key, (int)buffer->len, buffer->ptr);
		}

		span->tag_args++;
	}

	return retval;
}


/***
 * NAME
 *   ot_scope_data_add_baggage -
 *
 * ARGUMENTS
 *   span     -
 *   flag_new -
 *   data_str -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_scope_data_add_baggage(struct ot_scope_data *span, bool_t flag_new, const union spoe_data *data_str)
{
	int retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "%p, %hhu, %p", span, flag_new, data_str);

	if (_NULL(span) || (!flag_new && _NULL(data_str)))
		return retval;

	if (flag_new) {
		/*
		 * Checking if there is a previously created baggage.  If any,
		 * it should be completed (ie memory allocated in the buffer
		 * should be converted to baggage value).
		 */
		if (_nNULL(span->baggage) && _nNULL(span->baggage->key[span->baggage->count - 1])) {
			if (_nERROR(retval = buffer_grow(&(span->baggage_buffer), "\0", 1))) {
				OT_DBG("baggage[%zu]: <%s> -> <%s>", span->baggage->count - 1, span->baggage->key[span->baggage->count - 1], span->baggage_buffer.ptr);

				span->baggage->value[span->baggage->count - 1] = (char *)span->baggage_buffer.ptr;

				buffer_init(&(span->baggage_buffer));
			}
		} else {
			retval = FUNC_RET_OK;
		}

		/* Creating a new baggage key. */
		if (_nNULL(data_str) && _nERROR(retval)) {
			if (_NULL(span->baggage))
				span->baggage = otc_text_map_new(NULL, 8);

			if (_nNULL(span->baggage)) {
				retval = otc_text_map_add(span->baggage, data_str->chk.ptr, data_str->chk.len, "", 0, OTC_TEXT_MAP_DUP_KEY);

				OT_DBG("baggage[%zu]: <%s>", span->baggage->count - 1, span->baggage->key[span->baggage->count - 1]);
			}
			else
				retval = FUNC_RET_ERROR;
		}
	} else {
		/* Constructing baggage value. */
		retval = buffer_grow(&(span->baggage_buffer), data_str->chk.ptr, data_str->chk.len);

		OT_DBG("baggage[%zu]: <%s> -> <%.*s>", span->baggage->count - 1, span->baggage->key[span->baggage->count - 1], (int)span->baggage_buffer.len, span->baggage_buffer.ptr);
	}

	return retval;
}


/***
 * NAME
 *   ot_scope_data_add_log -
 *
 * ARGUMENTS
 *   span     -
 *   flag_new -
 *   data_str -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_scope_data_add_log(struct ot_scope_data *span, bool_t flag_new, const union spoe_data *data_str)
{
	int retval = FUNC_RET_ERROR;

	DBG_FUNC(NULL, "%p, %hhu, %p", span, flag_new, data_str);

	if (_NULL(span) || (!flag_new && _NULL(data_str)))
		return retval;

	if (flag_new) {
		/*
		 * Checking if there is a previously created log.  If any,
		 * it should be completed (ie memory allocated in the buffer
		 * should be converted to log value).
		 */
		if (_nNULL(span->log_fields[span->num_log_fields].key)) {
			if (_nERROR(retval = buffer_grow(&(span->log_buffer), "\0", 1))) {
				OT_DBG("log[%d]: <%s> -> <%s>", span->num_log_fields, span->log_fields[span->num_log_fields].key, span->log_buffer.ptr);

				span->log_fields[span->num_log_fields].value.value.string_value = (const char *)span->log_buffer.ptr;
				span->num_log_fields++;

				buffer_init(&(span->log_buffer));
			}
		} else {
			retval = FUNC_RET_OK;
		}

		/* Creating a new log key. */
		if (_nNULL(data_str) && _nERROR(retval)) {
			if (_nNULL(span->log_fields[span->num_log_fields].key = OTC_DBG_MEMDUP(data_str->chk.ptr, data_str->chk.len))) {
				OT_DBG("log[%d]: <%s>", span->num_log_fields, span->log_fields[span->num_log_fields].key);

				span->log_fields[span->num_log_fields].value.type = otc_value_string;

				retval = FUNC_RET_OK;
			} else {
				retval = FUNC_RET_ERROR;
			}
		}
	} else {
		/* Constructing log value. */
		retval = buffer_grow_va(&(span->log_buffer), data_str->chk.ptr, data_str->chk.len);

		OT_DBG("log[%d]: <%s> -> <%.*s>", span->num_log_fields, span->log_fields[span->num_log_fields].key, (int)span->log_buffer.len, span->log_buffer.ptr);
	}

	return retval;
}


/***
 * NAME
 *   ot_scope_span_finish -
 *
 * ARGUMENTS
 *   client  -
 *   msg     -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void ot_scope_span_finish(struct client *client, const char *msg)
{
	OT_MUTEX_LOCK_INIT();
	struct ot_runtime_context *ot_ctx, *ot_ctx_back;
	struct ot_scope_span      *ot_span, *ot_span_back;
	struct timespec            ts;

	DBG_FUNC(CW_PTR, "%p, \"%s\"", client, msg);

	(void)clock_gettime(CLOCK_MONOTONIC, &ts);

	OT_MUTEX_LOCK(&(prg.ot_mutex), , );

	list_for_each_entry_safe(ot_ctx, ot_ctx_back, &(prg.ot_contexts), list) {
		bool_t flag_ctx_free = 0;

		list_for_each_entry_safe(ot_span, ot_span_back, &(ot_ctx->spans), list)
			if (ot_span->client == client) {
				OT_DBG("releasing client SPAN '%.*s' '%.*s' %p", OT_STR_HDR_ARGS(ot_ctx, haproxy_id), OT_STR_HDR_ARGS(ot_span, id), ot_span);

				if (_nNULL(ot_span->span)) {
					/*
					 * If the span does not have a reference set, it
					 * is probably a root span.  In that case, the
					 * error tag should be set.
					 */
					if (_NULL(ot_span->ref_span) && _NULL(ot_span->ref_ctx)) {
						(void)ot_span_tag_va(ot_span->span, "error", otc_value_bool, 1, NULL);
						(void)ot_span_log_fmt(ot_span->span, "reason", "%s", (*(prg.stop_msg) == '\0') ? msg : prg.stop_msg);
					}

					ot_span_finish(&(ot_span->span), &ts, NULL, NULL, NULL);
				}

				ot_span->flag_unused = 1;
				flag_ctx_free        = 1;

				ot_scope_span_free(ot_ctx, &ot_span);
			}

		if (flag_ctx_free)
			ot_runtime_context_free(ot_ctx);
	}

	OT_MUTEX_UNLOCK(&(prg.ot_mutex), , );
}


/***
 * NAME
 *   ot_scope_action -
 *
 * ARGUMENTS
 *   ctx  -
 *   span -
 *   data -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_scope_action(struct ot_runtime_context *ctx, struct ot_scope_span *span, struct ot_scope_data *data)
{
	struct timespec ts;
	int             retval = FUNC_RET_OK;

	DBG_FUNC(NULL, "%p, %p, %p", ctx, span, data);

	/* All spans should be created/completed at the same time. */
	(void)clock_gettime(CLOCK_MONOTONIC, &ts);

	(void)ot_scope_data_add_tag(data, 1, NULL, NULL, -1);
	(void)ot_scope_data_add_baggage(data, 1, NULL);
	(void)ot_scope_data_add_log(data, 1, NULL);

	if (_nNULL(span)) {
		if (_NULL(span->span)) {
			if (_NULL(span->span = ot_span_init(span->id, &ts, NULL, span->ref_type, -1, span->ref_span, data->tags, data->num_tags)))
				retval = FUNC_RET_ERROR;
		}
		else if (data->num_tags > 0)
			if (_ERROR(ot_span_tag(span->span, data->tags, data->num_tags)))
				retval = FUNC_RET_ERROR;

		if (_nNULL(span->span) && _nNULL(data->baggage))
			if (_ERROR(ot_span_set_baggage(span->span, data->baggage)))
				retval = FUNC_RET_ERROR;

		if (_nNULL(span->span) && (data->num_log_fields > 0))
			if (_ERROR(ot_span_log(span->span, data->log_fields, data->num_log_fields)))
				retval = FUNC_RET_ERROR;
	}

	ot_scope_span_finish_marked(ctx, &ts);
	ot_scope_data_free(data);

	return retval;
}


/***
 * NAME
 *   ot_runtime_context -
 *
 * ARGUMENTS
 *   haproxy_id     -
 *   haproxy_id_len -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct ot_runtime_context *ot_runtime_context_init(const char *haproxy_id, size_t haproxy_id_len)
{
	OT_MUTEX_LOCK_INIT();
	struct ot_runtime_context *retptr = NULL;

	DBG_FUNC(NULL, "\"%.*s\", %zu", OT_STR_ARGS(haproxy_id));

	OT_MUTEX_LOCK(&(prg.ot_mutex), NULL, );

	list_for_each_entry(retptr, &(prg.ot_contexts), list)
		if ((retptr->haproxy_id_len == haproxy_id_len) && (memcmp(retptr->haproxy_id, haproxy_id, haproxy_id_len) == 0)) {
			OT_DBG("found rt_CTX '%.*s' %p", OT_STR_HDR_ARGS(retptr, haproxy_id), retptr);

			OT_MUTEX_UNLOCK(&(prg.ot_mutex), NULL, );

			return retptr;
		}

	if (_NULL(retptr = OTC_DBG_CALLOC(1, sizeof(*retptr)))) {
		/* Do nothing. */
	}
	else if (_NULL(retptr->haproxy_id = OTC_DBG_MEMDUP(haproxy_id, haproxy_id_len))) {
		OT_FREE_CLEAR(retptr);
	}
	else {
		retptr->haproxy_id_len = haproxy_id_len;
		LIST_INIT(&(retptr->spans));
		LIST_INIT(&(retptr->contexts));

		LIST_ADD(&(prg.ot_contexts), &(retptr->list));
		prg.ot_nbcontexts++;
	}

	OT_MUTEX_UNLOCK(&(prg.ot_mutex), NULL, OT_RUNTIME_CONTEXT_FREE(retptr));

	if (_nNULL(retptr))
		OT_DBG_RUNTIME_CONTEXT("new rt_CTX ", retptr);

	return retptr;
}


/***
 * NAME
 *   ot_runtime_context_free -
 *
 * ARGUMENTS
 *   ctx -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void ot_runtime_context_free(struct ot_runtime_context *ctx)
{
	OT_MUTEX_LOCK_INIT();

	DBG_FUNC(NULL, "%p", ctx);

	if (_NULL(ctx))
		return;

	OT_DBG_RUNTIME_CONTEXT("rt_CTX ", ctx);

	OT_MUTEX_LOCK(&(prg.ot_mutex), , );
	TIMEVAL_DIFF_START(_to);

#ifdef DEBUG
	ot_runtime_context_check(ctx);
#endif

	if (!LIST_ISEMPTY(&(ctx->spans))) {
		struct ot_scope_span *ot_span, *ot_span_back;

		list_for_each_entry_safe(ot_span, ot_span_back, &(ctx->spans), list)
			if (!OT_SCOPE_SPAN_IS_USED(ot_span))
				ot_scope_span_free(ctx, &ot_span);
	}

	/* Span contexts are not used, this block must never be executed. */
	if (!LIST_ISEMPTY(&(ctx->contexts))) {
		struct ot_scope_context *ot_ctx, *ot_ctx_back;

		list_for_each_entry_safe(ot_ctx, ot_ctx_back, &(ctx->contexts), list)
			/* Do nothing. */;
	}

	if ((ctx->nbspans == 0) && (ctx->nbcontexts == 0))
		OT_RUNTIME_CONTEXT_FREE(ctx);
	else
		OT_DBG("rt_CTX '%.*s' %u/%u spared, %u left", OT_STR_HDR_ARGS(ctx, haproxy_id), ctx->nbspans, ctx->nbcontexts, prg.ot_nbcontexts);

	OT_MUTEX_UNLOCK(&(prg.ot_mutex), , );
	TIMEVAL_DIFF_END(_to, OT, OT_STR "rt_CTX operation took ");
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
