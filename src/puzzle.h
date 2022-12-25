/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#ifndef PUZZLE_H
#define PUZZLE_H

/** A slot on a puzzle line. */
struct puzzle_slot {
	bool done;    /**< Whether this slot is solved. */
	size_t value; /**< Value of this slot. */
};

/** A line of the puzzle (used for both rows and columns). */
struct puzzle_line {
	size_t *clue;      /**< Array of loaded clue values for this line. */
	size_t clue_count; /**< Number of entries in array. */
	size_t clue_total; /**< Sum of loaded clue values for this line */

	size_t total; /**< Solver: Number of "done" slots on line. */

	struct puzzle_slot *slot; /**< Solver: Array of slots on line. */
	size_t slot_count;        /**< Solver: Number of entries in array. */
	size_t slot_max;          /**< Solver: Maximum slot value. */
};

/** Puzzle data representation. */
struct puzzle {
	char *name;

	struct puzzle_line *col;
	struct puzzle_line *row;

	size_t col_count;
	size_t row_count;

	size_t cells_complete;

	size_t clue_total;

	size_t *clue_start;
	size_t clue_start_count;
};

void puzzle_free(struct puzzle *p);

struct puzzle *puzzle_create(const char *path);

bool puzzle_is_complete(const struct puzzle *p);
bool puzzle_solve(struct puzzle *p);

#endif /* PUZZLE_H */
