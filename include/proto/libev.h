/***
 * Copyright 2020 HAProxy Technologies
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
#ifndef _PROTO_LIBEV_H
#define _PROTO_LIBEV_H

const char *ev_backend_name(uint type);
const char *ev_backend_type(struct ev_loop *loop);
const char *ev_backends_supported(void);

#endif /* _PROTO_LIBEV_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
