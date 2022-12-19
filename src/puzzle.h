/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#ifndef PUZZLE_H
#define PUZZLE_H

/** A line of the puzzle (used for both rows and columns). */
struct puzzle_line {
	size_t *clue;      /**< Array of loaded clue values for this line. */
	size_t clue_count; /**< Number of entries in array. */
};

/** Puzzle data representation. */
struct puzzle {
	char *name;

	struct puzzle_line *col;
	struct puzzle_line *row;

	size_t col_count;
	size_t row_count;
};

void puzzle_free(struct puzzle *p);

struct puzzle *puzzle_create(const char *path);

#endif /* PUZZLE_H */
