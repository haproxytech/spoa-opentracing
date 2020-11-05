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


/***
 * NAME
 *   spoa_msg_arg_dup -
 *
 * ARGUMENTS
 *   frame  -
 *   i      -
 *   arg    -
 *   arglen -
 *   data   -
 *   ptr    -
 *   len    -
 *   desc   -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int spoa_msg_arg_dup(const struct spoe_frame *frame, int i, const char *arg, size_t arglen, const union spoe_data *data, char **ptr, size_t *len, const char *desc)
{
	int retval = FUNC_RET_ERROR;

	DBG_FUNC(FW_PTR, "%p, %d, \"%.*s\", %zu, %p, %p:%p, %p, \"%s\"", frame, i, (int)arglen, arg, arglen, data, DPTR_ARGS(ptr), len, desc);

	if (_nNULL(*ptr)) {
		f_log(frame, _E("arg[%d] '%.*s': Duplicated %s"), i, (int)arglen, arg, desc);
	}
	else if (_NULL(*ptr = MEM_DUP(data->chk.ptr, data->chk.len))) {
		f_log(frame, _E("Failed to set %s"), desc);
	}
	else {
		retval = FUNC_RET_OK;

		if (_nNULL(len))
			*len = data->chk.len;
	}

	return retval;
}


/***
 * NAME
 *   spoa_msg_iprep -
 *
 * ARGUMENTS
 *   frame    -
 *   buf      -
 *   end      -
 *   ip_score -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int spoa_msg_iprep(struct spoe_frame *frame, const char **buf, const char *end, int *ip_score)
{
	union spoe_data      data;
	enum spoe_data_type  type;
	char                 addr[MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
	const char          *ptr = *buf, *str;
	uint64_t             len;
	uint8_t              nbargs;
	int                  retval = FUNC_RET_OK;

	DBG_FUNC(FW_PTR, "%p, %p:%p, %p, %p", frame, DPTR_ARGS(buf), end, ip_score);

	if (_ERROR(spoe_decode(frame, &ptr, end,
	                       SPOE_DEC_UINT8, &nbargs,
	                       SPOE_DEC_STR0, &str, &len,   /* arg name */
	                       SPOE_DEC_DATA, &data, &type, /* arg value */
	                       SPOE_DEC_END))) {
		retval = FUNC_RET_ERROR;
	}
	else if (nbargs != 1) {
		F_DBG(SPOA, frame, "1 arg expected, got %hhu", nbargs);

		return 0;
	}
	else if (type == SPOE_DATA_T_IPV4) {
		if (_nNULL(inet_ntop(AF_INET, &(data.ipv4), addr, INET_ADDRSTRLEN)))
			retval = random() % 101;

		F_DBG(SPOA, frame, "IPv4 score for %.*s is %d", INET_ADDRSTRLEN, addr, retval);
	}
	else if (type == SPOE_DATA_T_IPV6) {
		if (_nNULL(inet_ntop(AF_INET6, &(data.ipv6), addr, INET6_ADDRSTRLEN)))
			retval = random() % 101;

		F_DBG(SPOA, frame, "IPv6 score for %.*s is %d", INET6_ADDRSTRLEN, addr, retval);
	}
	else {
		return 0;
	}

	*ip_score = retval;

	SPOE_BUFFER_ADVANCE(retval);

	return retval;
}


/***
 * NAME
 *   spoa_msg_iprep_action -
 *
 * ARGUMENTS
 *   frame    -
 *   buf      -
 *   ip_score -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void spoa_msg_iprep_action(struct spoe_frame *frame, char **buf, int ip_score)
{
	DBG_FUNC(FW_PTR, "%p, %p:%p, %d", frame, DPTR_ARGS(buf), ip_score);

	F_DBG(SPOA, frame, "Add action: set variable ip_score=%d", ip_score);

	(void)spoe_encode(frame, buf,
	                  SPOE_ENC_UINT8, SPOE_ACT_T_SET_VAR, /* Action type */
	                  SPOE_ENC_UINT8, 3,                  /* Number of args */
	                  SPOE_ENC_UINT8, SPOE_SCOPE_SESS,    /* Arg 1: the scope */
	                  SPOE_ENC_STR, "ip_score", 8,        /* Arg 2: variable name */
	                  SPOE_ENC_UINT8, SPOE_DATA_T_UINT32, /* Arg 3: variable type */
	                  SPOE_ENC_VARINT, ip_score,          /*        variable value */
	                  SPOE_ENC_END);
}


/***
 * NAME
 *   spoa_msg_test -
 *
 * ARGUMENTS
 *   frame -
 *   buf   -
 *   end   -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int spoa_msg_test(struct spoe_frame *frame, const char **buf, const char *end)
{
	union spoe_data      data;
	enum spoe_data_type  type;
	char                 addr[MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
	const char          *ptr = *buf, *str;
	uint64_t             len;
	uint8_t              nbargs;
	int                  i, retval = FUNC_RET_OK;

	DBG_FUNC(FW_PTR, "%p, %p:%p, %p", frame, DPTR_ARGS(buf), end);

	retval = spoe_decode(frame, &ptr, end, SPOE_DEC_UINT8, &nbargs, SPOE_DEC_END);
	if (_nERROR(retval))
		F_DBG(SPOA, frame, "%hhu arg(s) expected", nbargs);

	for (i = 0; _nERROR(retval) && (i < nbargs); i++) {
		retval = spoe_decode(frame, &ptr, end,
		                     SPOE_DEC_STR0, &str, &len,   /* arg name */
		                     SPOE_DEC_DATA, &data, &type, /* arg value */
		                     SPOE_DEC_END);
		if (_ERROR(retval)) {
			break;
		}
		else if (type == SPOE_DATA_T_NULL) {
			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d:", i, (int)len, str, type);
		}
		else if (type == SPOE_DATA_T_BOOL) {
			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d: %02hhx", i, (int)len, str, type, data.boolean);
		}
		else if (type == SPOE_DATA_T_INT32) {
			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d: %d", i, (int)len, str, type, data.int32);
		}
		else if (type == SPOE_DATA_T_UINT32) {
			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d: %08x", i, (int)len, str, type, data.uint32);
		}
		else if (type == SPOE_DATA_T_INT64) {
			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d: %"PRId64, i, (int)len, str, type, data.int64);
		}
		else if (type == SPOE_DATA_T_UINT64) {
			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d: %016"PRIx64, i, (int)len, str, type, data.uint64);
		}
		else if (type == SPOE_DATA_T_IPV4) {
			if (_NULL(inet_ntop(AF_INET, &(data.ipv4), addr, INET_ADDRSTRLEN)))
				retval = FUNC_RET_ERROR;

			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d: \"%s\"", i, (int)len, str, type, _ERROR(retval) ? "invalid" : addr);
		}
		else if (type == SPOE_DATA_T_IPV6) {
			if (_NULL(inet_ntop(AF_INET6, &(data.ipv6), addr, INET6_ADDRSTRLEN)))
				retval = FUNC_RET_ERROR;

			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d: \"%s\"", i, (int)len, str, type, _ERROR(retval) ? "invalid" : addr);
		}
		else if (type == SPOE_DATA_T_STR) {
			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d: \"%.*s\"", i, (int)len, str, type, (int)data.chk.len, data.chk.ptr);
		}
		else if (type == SPOE_DATA_T_BIN) {
			F_DBG(SPOA, frame, "test[%d] name='%.*s' type=%d: <%s> <%s>", i, (int)len, str, type, str_hex(data.chk.ptr, data.chk.len), str_ctrl(data.chk.ptr, data.chk.len));
		}
		else {
			f_log(frame, _E("test[%d] name='%.*s': Invalid argument data type: %d"), i, (int)len, str, type);

			retval = FUNC_RET_ERROR;
		}
	}

	SPOE_BUFFER_ADVANCE(retval);

	return retval;
}


#ifdef HAVE_LIBOPENTRACING

/***
 * NAME
 *   spoa_msg_opentracing -
 *
 * ARGUMENTS
 *   frame -
 *   buf   -
 *   end   -
 *   name  -
 *   nlen  -
 *
 * DESCRIPTION
 *   args id=unique-id
 *     [ span=str("span id") [ child-of=str("parent span id") | follows-from=str("parent span id") ] ]
 *       [ tag=str("tag id") data ... ]
 *       [ baggage=str("baggage id") data ... ]
 *       [ log=str("log id") data ... ]
 *     [ finish=str("span id") ]
 *   [ event event-name [ { if | unless } condition ] ]
 *
 * RETURN VALUE
 *   -
 */
int spoa_msg_opentracing(struct spoe_frame *frame, const char **buf, const char *end, const char *name __maybe_unused, size_t nlen __maybe_unused)
{
	struct ot_runtime_context *ot_ctx = NULL;
	struct ot_scope_span      *ot_span = NULL;
	struct ot_scope_data       ot_data;
	union spoe_data            data, data_str;
	enum spoe_data_type        type;
	const char                *ptr = *buf, *str, *arg_str = NULL;
	uint64_t                   len, arg_len = 0;
	uint8_t                    nbargs;
	int                        i, retval = FUNC_RET_ERROR;

	DBG_FUNC(FW_PTR, "%p, %p:%p, %p, \"%.*s\", %zu", frame, DPTR_ARGS(buf), end, (int)nlen, name, nlen);

	(void)memset(&ot_data, 0, sizeof(ot_data));

	retval = spoe_decode(frame, &ptr, end, SPOE_DEC_UINT8, &nbargs, SPOE_DEC_END);
	if (_nERROR(retval))
		F_DBG(OT, frame, "%hhu arg(s) expected", nbargs);

	for (i = 0; (i < nbargs) && _nERROR(retval); i++) {
		retval = spoe_decode(frame, &ptr, end,
		                     SPOE_DEC_STR0, &str, &len,   /* arg name */
		                     SPOE_DEC_DATA, &data, &type, /* arg value */
		                     SPOE_DEC_END);
		if (_ERROR(retval))
			break;

		/*
		 * If the argument does not have a name set,
		 * assign it the name of the previous argument.
		 */
		if (len != 0) {
			arg_len = len;
			arg_str = str;
		}

		F_DBG(OT, frame, "arg[%d]: name '%.*s' -> '%.*s' %zu/%zu", i, (int)len, str, (int)arg_len, arg_str, len, arg_len);

		if (_NULL(arg_str)) {
			f_log(frame, _E(OT_STR "arg[%d]: Argument has no name"), i);

			break;
		}
		else if (_ERROR(retval = spoe_data_to_str(&data, type, &data_str))) {
			f_log(frame, _E(OT_STR "arg[%d] '%.*s': Invalid argument data type: %d"), i, (int)len, str, type);

			break;
		}

		F_DBG(OT, frame, "arg[%d]: data '%.*s' %zu, type=%d", i, (int)data_str.chk.len, data_str.chk.ptr, data_str.chk.len, type);

		if (STR_CMP_EQ(OT_SCOPE_HAPROXY_ID, arg_str, arg_len)) {
			if (_NULL(ot_ctx = ot_runtime_context_init(data_str.chk.ptr, data_str.chk.len)))
				retval = FUNC_RET_ERROR;
		}
		else if (_NULL(ot_ctx)) {
			f_log(frame, _E(OT_STR "HAProxy unique ID not set (use: args id=unique-id)"));

			break;
		}
		else if (STR_CMP_EQ(OT_SCOPE_FINISH, arg_str, arg_len)) {
			retval = ot_scope_span_finish_mark(ot_ctx, ot_span, data_str.chk.ptr, data_str.chk.len);
		}
		else if (STR_CMP_EQ(OT_SCOPE_SPAN, arg_str, arg_len)) {
			if (_nNULL(ot_span)) {
				retval  = ot_scope_action(ot_ctx, ot_span, &ot_data);
				ot_span = NULL;
			}

			if (_nERROR(retval))
				retval = spoa_msg_arg_dup(frame, i, arg_str, arg_len, &data_str, &(ot_data.id), &(ot_data.id_len), "span ID");
		}
		else if (_NULL(ot_data.id)) {
			f_log(frame, _E(OT_STR "span ID not set (use: args span=str(\"span id\"))"));

			break;
		}
		else if (STR_CMP_EQ(OT_SCOPE_SPAN_REF_CHILD, arg_str, arg_len)) {
			if (_NULL(ot_span = ot_scope_span_init(ot_ctx, &ot_data, otc_span_reference_child_of, data_str.chk.ptr, data_str.chk.len, FC_PTR)))
				break;
		}
		else if (STR_CMP_EQ(OT_SCOPE_SPAN_REF_FOLLOWS, arg_str, arg_len)) {
			if (_NULL(ot_span = ot_scope_span_init(ot_ctx, &ot_data, otc_span_reference_follows_from, data_str.chk.ptr, data_str.chk.len, FC_PTR)))
				break;
		}
		else {
			if (_nNULL(ot_data.id) && _NULL(ot_span))
				if (_NULL(ot_span = ot_scope_span_init(ot_ctx, &ot_data, -1, NULL, 0, FC_PTR)))
					break;

			if (STR_CMP_EQ(OT_SCOPE_TAG, arg_str, arg_len)) {
				if (_ERROR(retval = ot_scope_data_add_tag(&ot_data, len > 0, &data_str, &data, type)))
					f_log(frame, _E(OT_STR "Failed to set span tag data"));
			}
			else if (STR_CMP_EQ(OT_SCOPE_BAGGAGE, arg_str, arg_len)) {
				if (_ERROR(retval = ot_scope_data_add_baggage(&ot_data, len > 0, &data_str)))
					f_log(frame, _E(OT_STR "Failed to set span baggage data"));
			}
			else if (STR_CMP_EQ(OT_SCOPE_LOG, arg_str, arg_len)) {
				if (_ERROR(retval = ot_scope_data_add_log(&ot_data, len > 0, &data_str)))
					f_log(frame, _E(OT_STR "Failed to set span log data"));
			}
			else {
				f_log(frame, _E(OT_STR "arg[%d] '%.*s': Unknown argument"), i, (int)arg_len, arg_str);

				break;
			}
		}
	}

	if (i < nbargs)
		retval = FUNC_RET_ERROR;

	if (_nERROR(retval))
		retval = ot_scope_action(ot_ctx, ot_span, &ot_data);

	ot_scope_data_free(&ot_data);
	ot_runtime_context_free(ot_ctx);

	SPOE_BUFFER_ADVANCE(retval);

	return retval;
}

#endif /* HAVE_LIBOPENTRACING */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
