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
#ifndef _TYPES_OPENTRACING_H
#define _TYPES_OPENTRACING_H

#define OT_STR               "OT: "
#define OT_MAXTAGS           8
#define OT_DBG(a, ...)       W_DBG(OT, NULL, "  " OT_STR a, ##__VA_ARGS__)
#define OT_VARGS(t,v)        otc_value_##t, (v)
#define OT_VSET(p,t,v)       do { (p)->type = otc_value_##t; (p)->value.t##_value = (v); } while (0)
#define OT_FREE(a)           do { if (_nNULL(a)) OTC_DBG_FREE((void *)a); } while (0)
#define OT_FREE_CLEAR(a)     do { if (_nNULL(a)) { OTC_DBG_FREE((void *)a); (a) = NULL; } } while (0)
#define OT_LIST_ARGS(a)      (a)->n, (a)->p
#define OT_LIST_ISVALID(a)   (_nNULL(a) && _nNULL((a)->n) && _nNULL((a)->p))
#define OT_LIST_DEL(a)       do { if (OT_LIST_ISVALID(a)) LIST_DEL(a); } while (0)

#define OT_DBG_TEXT_MAP(p,a)                   \
	OT_DBG("%s%p:{ %p %p %zu/%zu %hhu }",  \
	       (p), (a), (a)->key, (a)->value, \
	       (a)->count, (a)->size, (a)->is_dynamic)

#define OT_DBG_TEXT_CARRIER(p,a,f)                               \
	OT_DBG("%s%p:{ { %p %p %zu/%zu %hhu } %p }",             \
	       (p), (a), (a)->text_map.key, (a)->text_map.value, \
	       (a)->text_map.count, (a)->text_map.size,          \
	       (a)->text_map.is_dynamic, (a)->f)

#define OT_DBG_CUSTOM_CARRIER(p,a,f)            \
	OT_DBG("%s%p:{ { %p %zu %hhu } %p }",   \
	       (p), (a), (a)->binary_data.data, \
	       (a)->binary_data.size, (a)->binary_data.is_dynamic, (a)->f)

#define OT_DBG_SPAN_CONTEXT(p,a)             \
	OT_DBG("%s%p:{ %" PRId64 " %p %p }", \
	       (p), (a), (a)->idx, (a)->span, (a)->destroy)

#endif /* _TYPES_OPENTRACING_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
