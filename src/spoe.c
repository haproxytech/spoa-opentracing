/***
 * Copyright 2018,2019 HAProxy Technologies
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
 *   spoe_frm_err_reasons -
 *
 * ARGUMENTS
 *   status_code -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
const char *spoe_frm_err_reasons(int status_code)
{
#define SPOE_FRM_ERR_DEF(v,e,s)   { s, SPOE_FRM_ERR_##e },
	static const struct {
		const char *str;
		int code;
	} reasons[] = { SPOE_FRM_ERR_DEFINES };
#undef SPOE_FRM_ERR_DEF
	int i;

	for (i = 0; i < TABLESIZE(reasons); i++)
		if (reasons[i].code == status_code)
			break;

	/* If status_code is invalid, return message for SPOE_FRM_ERR_UNKNOWN code. */
	return (i < TABLESIZE(reasons)) ? reasons[i].str : reasons[TABLESIZE_1(reasons)].str;
}


/***
 * NAME
 *   spoe_data_to_str -
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
int spoe_data_to_str(const union spoe_data *data, enum spoe_data_type type, union spoe_data *value)
{
	static __THR char retbuf[MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
	int               retval = FUNC_RET_OK;

	DBG_FUNC(NULL, "%p, %d, %p", data, type, value);

	value->chk.ptr = retbuf;

	if (type == SPOE_DATA_T_NULL) {
		retbuf[0] = '\0';
	}
	else if (type == SPOE_DATA_T_BOOL) {
		retbuf[0] = data->boolean ? '1' : '0';
		retbuf[1] = '\0';
	}
	else if (type == SPOE_DATA_T_INT32) {
		(void)snprintf(retbuf, sizeof(retbuf), "%d", data->int32);
	}
	else if (type == SPOE_DATA_T_UINT32) {
		(void)snprintf(retbuf, sizeof(retbuf), "%u", data->uint32);
	}
	else if (type == SPOE_DATA_T_INT64) {
		(void)snprintf(retbuf, sizeof(retbuf), "%"PRId64, data->int64);
	}
	else if (type == SPOE_DATA_T_UINT64) {
		(void)snprintf(retbuf, sizeof(retbuf), "%"PRIu64, data->uint64);
	}
	else if (type == SPOE_DATA_T_IPV4) {
		if (_NULL(inet_ntop(AF_INET, &(data->ipv4), retbuf, INET_ADDRSTRLEN)))
			retval = FUNC_RET_ERROR;
	}
	else if (type == SPOE_DATA_T_IPV6) {
		if (_NULL(inet_ntop(AF_INET6, &(data->ipv6), retbuf, INET6_ADDRSTRLEN)))
			retval = FUNC_RET_ERROR;
	}
	else if (type == SPOE_DATA_T_STR) {
		value->chk.ptr = data->chk.ptr;
		value->chk.len = data->chk.len;
	}
	else if (type == SPOE_DATA_T_BIN) {
		W_DBG(SPOA, NULL, "Unsupported argument data type: %d", type);

		retval = FUNC_RET_ERROR;
	}
	else {
		W_DBG(SPOA, NULL, "Invalid argument data type: %d", type);

		retval = FUNC_RET_ERROR;
	}

	if (_nERROR(retval) && (type != SPOE_DATA_T_STR))
		retval = value->chk.len = strlen(value->chk.ptr);

	return retval;
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
