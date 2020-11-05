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
#ifndef _PROTO_OPENTRACING_SCOPE_H
#define _PROTO_OPENTRACING_SCOPE_H

int ot_scope_span_finish_mark(struct ot_runtime_context *ctx, struct ot_scope_span *span, const char *id, size_t id_len);
struct ot_scope_span *ot_scope_span_init(struct ot_runtime_context *ctx, const struct ot_scope_data *data, otc_span_reference_type_t ref_type, const char *ref_id, size_t ref_id_len, const struct client *client);
void ot_scope_data_free(struct ot_scope_data *data);
int ot_scope_data_add_tag(struct ot_scope_data *span, bool_t flag_new, const union spoe_data *data_str, const union spoe_data *data, enum spoe_data_type type);
int ot_scope_data_add_baggage(struct ot_scope_data *span, bool_t flag_new, const union spoe_data *data_str);
int ot_scope_data_add_log(struct ot_scope_data *span, bool_t flag_new, const union spoe_data *data_str);
void ot_scope_span_finish(struct client *client, const char *msg);
int ot_scope_action(struct ot_runtime_context *ctx, struct ot_scope_span *span, struct ot_scope_data *data);
struct ot_runtime_context *ot_runtime_context_init(const char *haproxy_id, size_t haproxy_id_len);
void ot_runtime_context_free(struct ot_runtime_context *ctx);

#endif /* _PROTO_OPENTRACING_SCOPE_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
