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
#ifndef _TYPES_OPENTRACING_SCOPE_H
#define _TYPES_OPENTRACING_SCOPE_H

#define OT_SCOPE_HAPROXY_ID             "id"
#define OT_SCOPE_SPAN                   "span"
#define OT_SCOPE_SPAN_REF_CHILD         "child-of"
#define OT_SCOPE_SPAN_REF_FOLLOWS       "follows-from"
#define OT_SCOPE_TAG                    "tag"
#define OT_SCOPE_BAGGAGE                "baggage"
#define OT_SCOPE_LOG                    "log"
#define OT_SCOPE_FINISH                 "finish"
#define OT_SCOPE_FINISH_ALL             "*"

#define OT_SCOPE_SPAN_CHECK_ID(p,i,l)   (((p)->id_len == (l)) && (memcmp((p)->id, (i), (l)) == 0))
#define OT_SCOPE_SPAN_IS_USED(p)        (_nNULL((p)->span) || !(p)->flag_unused)

#define OT_MUTEX_LOCK_INIT()            bool_t _fml = 0
#define OT_MUTEX_OPER(o,m,e)            ((cfg.num_workers > 1) && _nOK((e) = pthread_mutex_##o(m)))
#define OT_MUTEX_ERR(o,v,e)                                        \
	IFDEF_DBG(                                                 \
		do {                                               \
			char _buf[BUFSIZ];                         \
			(void)strerror_r((e), _buf, sizeof(_buf)); \
			OT_DBG("Cannot " #o " mutex: %s", _buf);   \
			return v;                                  \
		} while (0);                                       \
	, )
#define OT_MUTEX_LOCK(m,v,f)                                             \
	do {                                                             \
		int _err;                                                \
		TIMEVAL_DIFF_START(_t);                                  \
		if (!_fml && OT_MUTEX_OPER(lock, (m), _err)) {           \
			{ f; }                                           \
			OT_MUTEX_ERR(lock, v, _err);                     \
		} else {                                                 \
			_fml = 1;                                        \
		}                                                        \
		TIMEVAL_DIFF_END(_t, OT, OT_STR "ctx mutex lock took "); \
	} while (0)
#define OT_MUTEX_UNLOCK(m,v,f)                                  \
	do {                                                    \
		int _err;                                       \
		if (_fml && OT_MUTEX_OPER(unlock, (m), _err)) { \
			{ f; }                                  \
			OT_MUTEX_ERR(unlock, v, _err);          \
		} else {                                        \
			_fml = 0;                               \
		}                                               \
		OT_DBG("ctx mutex unlocked");                   \
	} while (0)

#define OT_SCOPE_SPAN_FREE(c,p)                                                                   \
	do {                                                                                      \
		OT_LIST_DEL(&((p)->list));                                                        \
		(c)->nbspans--;                                                                   \
                                                                                                  \
		OT_DBG("SPAN '%.*s' '%.*s' deleted, %u left",                                     \
		       OT_STR_HDR_ARGS((c), haproxy_id), OT_STR_HDR_ARGS((p), id), (c)->nbspans); \
		OT_FREE((p)->id);                                                                 \
		OT_FREE(p);                                                                       \
	} while (0)

#define OT_RUNTIME_CONTEXT_FREE(p)                                                                             \
	do {                                                                                                   \
		OT_LIST_DEL(&((p)->list));                                                                     \
		prg.ot_nbcontexts--;                                                                           \
                                                                                                               \
		OT_DBG("rt_CTX '%.*s' deleted, %u left", OT_STR_HDR_ARGS((p), haproxy_id), prg.ot_nbcontexts); \
		OT_FREE((p)->haproxy_id);                                                                      \
		OT_FREE(p);                                                                                    \
	} while (0)

#define OT_STR_HDR(n)            \
	struct {                 \
		char   *n;       \
		size_t  n##_len; \
	}

#define OT_STRN_ARGS(n)                 (int)n##_len, n
#define OT_STR_ARGS(n)                  (int)n##_len, n, n##_len
#define OT_STR_HDR_ARGS(p,n)            (int)(p)->n##_len, (p)->n

#define OT_DBG_SCOPE_SPAN(p,a)                                                     \
	OT_DBG("%s%p:{ '%s' %zu %hhu %hhu %p %d %p %p { %p %p } }",                \
	       (p), (a), (a)->id, (a)->id_len, (a)->flag_unused, (a)->flag_finish, \
	       (a)->span, (a)->ref_type, (a)->ref_span, (a)->ref_ctx,              \
	       OT_LIST_ARGS(&((a)->list)))

#define OT_DBG_SCOPE_DATA(p,a)                                                    \
	OT_DBG("%s%p:{ '%s' %zu %p %d %p %d %p %p %p %p %d }",                    \
	       (p), (a), (a)->id, (a)->id_len, &((a)->tag_buffer), (a)->tag_args, \
	       (a)->tags, (a)->num_tags, &((a)->baggage_buffer), (a)->baggage,    \
	       &((a)->log_buffer), (a)->log_fields, (a)->num_log_fields)

#define OT_DBG_SCOPE_CONTEXT(p,a)                                \
	OT_DBG("%s%p:{ '%s' %zu %hhu %p { %p %p } }",            \
	       (p), (a), (a)->id, (a)->id_len, (a)->flag_finish, \
	       (a)->context, OT_LIST_ARGS(&((a)->list)))

#define OT_DBG_RUNTIME_CONTEXT(p,a)                                     \
	OT_DBG("%s%p:{ '%s' %zu { %p %p } %u { %p %p } %u { %p %p } }", \
	       (p), (a), (a)->haproxy_id, (a)->haproxy_id_len,          \
	       OT_LIST_ARGS(&((a)->spans)), (a)->nbspans,               \
	       OT_LIST_ARGS(&((a)->contexts)), (a)->nbcontexts,         \
	       OT_LIST_ARGS(&((a)->list)))


struct ot_scope_span {
	OT_STR_HDR(id);                             /* The span operation name/len. */
	bool                           flag_unused; /* Whether the span is marked as unused. */
	bool                           flag_finish; /* Whether the span is marked for completion. */
	struct otc_span               *span;        /* The current span. */
	otc_span_reference_type_t      ref_type;    /* Span reference type. */
	const struct otc_span         *ref_span;    /* Span to which the current span refers. */
	const struct otc_span_context *ref_ctx;     /* Span context to which the current span refers. */
	const struct client           *client;      /* Client owner of this span. */
	struct list                    list;        /* Used to chain this data in struct worker. */
};

struct ot_scope_data {
	OT_STR_HDR(id);                                     /* The span operation name/len. */
	struct buffer         tag_buffer;                   /* */
	int                   tag_args;                     /* */
	struct otc_tag        tags[OT_MAXTAGS];             /* */
	int                   num_tags;                     /* */
	struct buffer         baggage_buffer;               /* */
	struct otc_text_map  *baggage;                      /* */
	struct buffer         log_buffer;                   /* */
	struct otc_log_field  log_fields[OTC_MAXLOGFIELDS]; /* */
	int                   num_log_fields;               /* */
};

struct ot_scope_context {
	OT_STR_HDR(id);                       /* The span context name/len. */
	bool                     flag_finish; /* Whether the span context is marked for completion. */
	struct otc_span_context *context;     /* The current span context. */
	struct list              list;        /* Used to chain this data in struct worker. */
};

struct ot_runtime_context {
	OT_STR_HDR(haproxy_id);    /* */
	struct list spans;         /* The scope spans. */
	uint        nbspans;       /* */
	struct list contexts;      /* The scope contexts. */
	uint        nbcontexts;    /* */
	struct list list;          /* Used to chain this data in struct program_data. */
};

#endif /* _TYPES_OPENTRACING_SCOPE_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
