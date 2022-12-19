/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdlib.h>
#include <stdint.h>

#include "grid.h"

struct grid *grid_create(size_t width, size_t height)
{
	struct grid *g;

	g = calloc(1, sizeof(*g));
	if (g == NULL) {
		return NULL;
	}

	g->data = calloc(width * height, sizeof(*g->data));
	if (g->data == NULL) {
		grid_free(g);
		return NULL;
	}

	g->width = width;
	g->height = height;

	return g;
}

void grid_free(
		struct grid *g)
{
	if (g != NULL) {
		free(g->data);
		free(g);
	}
}
