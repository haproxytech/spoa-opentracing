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
#ifndef _TYPES_WORKER_H
#define _TYPES_WORKER_H

struct worker {
	pthread_t        thread;
	int              id;
	int              fd;
	struct ev_async  ev_async;
	struct ev_loop  *ev_base;
	struct ev_timer  ev_monitor;

	struct list      engines;

	struct list      clients;
	uint             nbclients;

	struct list      frames;
	uint             nbframes;
};

struct worker_signal {
	struct ev_signal signal;
	int              signum;
	void             (*func)(struct ev_loop *, struct ev_signal *, int);
};

#endif /* _TYPES_WORKER_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
