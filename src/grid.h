/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#ifndef GRID_H
#define GRID_H

struct grid {
	uint8_t *data;

	size_t width;
	size_t height;
};

struct grid *grid_create(size_t width, size_t height);

void grid_free(struct grid *g);

#endif /* GRID_H */
