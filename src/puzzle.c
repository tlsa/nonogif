
/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "puzzle.h"
#include "load.h"

static void puzzle__line_free(struct puzzle_line *pl)
{
	if (pl != NULL) {
		free(pl);
	}
}

void puzzle_free(struct puzzle *p)
{
	if (p != NULL) {
		free(p->name);
		puzzle__line_free(p->col);
		puzzle__line_free(p->row);
		free(p);
	}
}

struct puzzle *puzzle_create(const char *path)
{
	struct puzzle *p;

	p = load_file(path);
	if (p == NULL) {
		return NULL;
	}

	return p;
}
