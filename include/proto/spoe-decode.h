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
#ifndef _PROTO_SPOE_DECODE_H
#define _PROTO_SPOE_DECODE_H

int spoe_decode(struct spoe_frame *frame, const char **buf, const char *end, int type, ...);
int spoe_decode_kv(struct spoe_frame *frame, const char **buf, const char *end, ...);
int spoe_decode_skip_msg(struct spoe_frame *frame, const char **buf, const char *end);
int spoe_decode_frame(const char *msg, struct spoe_frame *frame, uint8_t spoe_type, int spoe_retval, int type, ...);

#endif /* _PROTO_SPOE_DECODE_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
