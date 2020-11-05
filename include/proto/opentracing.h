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
#ifndef _PROTO_OPENTRACING_H
#define _PROTO_OPENTRACING_H

int ot_init(void);
struct otc_span *ot_span_init(const char *operation_name, const struct timespec *ts_steady, const struct timespec *ts_system, int ref_type, int ref_ctx_idx, const struct otc_span *ref_span, const struct otc_tag *tags, int num_tags);
struct otc_span *ot_span_init_va(const char *operation_name, const struct timespec *ts_steady, const struct timespec *ts_system, int ref_type, int ref_ctx_idx, const struct otc_span *ref_span, const char *tag_key, const char *tag_value, ...);
int ot_span_tag(struct otc_span *span, const struct otc_tag *tags, int num_tags);
int ot_span_tag_va(struct otc_span *span, const char *key, int type, ...);
int ot_span_log(struct otc_span *span, const struct otc_log_field *log_fields, int num_fields);
int ot_span_log_va(struct otc_span *span, const char *key, const char *value, ...);
int ot_span_log_fmt(struct otc_span *span, const char *key, const char *format, ...)
	__fmt(printf, 3, 4);
int ot_span_set_baggage(struct otc_span *span, const struct otc_text_map *baggage);
int ot_span_set_baggage_va(struct otc_span *span, const char *key, const char *value, ...);
struct otc_text_map *ot_span_baggage_va(const struct otc_span *span, const char *key, ...);
struct otc_span_context *ot_inject_text_map(const struct otc_span *span, struct otc_text_map_writer *carrier);
struct otc_span_context *ot_inject_http_headers(const struct otc_span *span, struct otc_http_headers_writer *carrier);
struct otc_span_context *ot_inject_binary(const struct otc_span *span, struct otc_custom_carrier_writer *carrier);
struct otc_span_context *ot_extract_text_map(struct otc_text_map_reader *carrier, const struct otc_text_map *text_map);
struct otc_span_context *ot_extract_http_headers(struct otc_http_headers_reader *carrier, const struct otc_text_map *text_map);
struct otc_span_context *ot_extract_binary(struct otc_custom_carrier_reader *carrier, const struct otc_binary_data *binary_data);
void ot_span_finish(struct otc_span **span, const struct timespec *ts_finish, const struct timespec *log_ts, const char *log_key, const char *log_value, ...);
void ot_close(void);
void ot_statistics(void);

#endif /* _PROTO_OPENTRACING_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
