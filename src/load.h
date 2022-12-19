/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#ifndef LOAD_H
#define LOAD_H

#include "puzzle.h"

struct puzzle *load_file(
		const char *path);

#endif /* LOAD_H */
