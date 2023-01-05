
/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "output.h"
#include "puzzle.h"
#include "load.h"

static void puzzle__line_free(struct puzzle_line *pl, size_t count)
{
	if (pl != NULL) {
		for (size_t i = 0; i < count; i++) {
			free(pl[i].clue);
			free(pl[i].slot);
		}
		free(pl);
	}
}

void puzzle_free(struct puzzle *p)
{
	if (p != NULL) {
		free(p->name);
		free(p->clue_start);
		puzzle__line_free(p->col, p->col_count);
		puzzle__line_free(p->row, p->row_count);
		free(p);
	}
}

static bool puzzle__initialise_lines(
		struct puzzle_line *lines,
		size_t line_count,
		size_t slot_count,
		size_t *clue_total_out,
		size_t *max_clues_out)
{
	size_t clue_total = 0;
	size_t max_clues = 0;

	for (size_t i = 0; i < line_count; i++) {
		struct puzzle_line *line = &lines[i];

		line->update_needed = true;
		line->slot_count = slot_count;
		line->slot = calloc(slot_count, sizeof(*line->slot));
		if (line->slot == NULL) {
			return false;
		}

		for (size_t j = 0; j < line->clue_count; j++) {
			line->clue_total += line->clue[j];
		}
		clue_total += line->clue_total;

		if (line->clue_count > max_clues) {
			max_clues = line->clue_count;
		}
	}

	for (size_t i = 0; i < line_count; i++) {
		struct puzzle_line *line = &lines[i];

		for (size_t j = 0; j < line->slot_count; j++) {
			line->slot[j].value = clue_total * 2;
		}
		line->slot_max = line_count * slot_count;
	}

	*clue_total_out += clue_total;
	if (*max_clues_out < max_clues) {
		*max_clues_out = max_clues;
	}
	return true;
}

struct puzzle *puzzle_create(const char *path)
{
	struct puzzle *p;

	p = load_file(path);
	if (p == NULL) {
		return NULL;
	}

	if (!puzzle__initialise_lines(p->col, p->col_count, p->row_count,
			&p->clue_total, &p->clue_start_count)) {
		fprintf(stderr, "Error: Failed to initialise lines!\n");
		puzzle_free(p);
		return NULL;
	}

	if (!puzzle__initialise_lines(p->row, p->row_count, p->col_count,
			&p->clue_total, &p->clue_start_count)) {
		fprintf(stderr, "Error: Failed to initialise lines!\n");
		puzzle_free(p);
		return NULL;
	}

	p->clue_start = calloc(p->clue_start_count, sizeof(*p->clue_start));
	if (p->clue_start == NULL) {
		fprintf(stderr, "Error: Allocation failed!\n");
		puzzle_free(p);
		return NULL;
	}

	return p;
}

static inline bool puzzle__slot_is_set(struct puzzle_slot *slot)
{
	return slot->done && slot->value > 0;
}

static inline bool puzzle__slot_is_clear(struct puzzle_slot *slot)
{
	return slot->done && slot->value == 0;
}

static inline size_t puzzle__available_gap(
		const struct puzzle_line *line,
		size_t pos)
{
	for (size_t i = pos; i < line->slot_count; i++) {
		if (puzzle__slot_is_clear(&line->slot[i])) {
			return i - pos;
		}
	}

	return line->slot_count - pos;
}

static inline bool puzzle__can_place_single_clue(
		const struct puzzle_line *line,
		size_t clue,
		size_t gap,
		size_t pos)
{
	if (gap < clue) {
		return false;
	}

	if (pos + clue < line->slot_count &&
	    puzzle__slot_is_set(&line->slot[pos + clue])) {
		return false;
	}

	return true;
}

static inline bool puzzle__can_place_clue(
		const struct puzzle *p,
		const struct puzzle_line *line,
		size_t clue_idx,
		size_t pos)
{
	for (size_t c = clue_idx; c < line->clue_count; c++) {
		bool placed = false;

		for (size_t i = pos; i < line->slot_count; i++) {
			size_t gap = puzzle__available_gap(line, i);
			if (puzzle__can_place_single_clue(line, line->clue[c],
					gap, i)) {
				p->clue_start[c] = i;
				placed = true;
				pos = i + line->clue[c] + 1;
				break;
			}
		}

		if (placed == false) {
			return false;
		}
	}

	return true;
}

static inline bool puzzle__missed_set_cell(
		const struct puzzle *p,
		struct puzzle_line *line)
{
	for (size_t s = 0; s < line->slot_count; s++) {
		if (puzzle__slot_is_set(&line->slot[s])) {
			bool inside = false;
			for (size_t c = 0; c < line->clue_count; c++) {
				if (s >= p->clue_start[c] &&
				    s < p->clue_start[c] + line->clue[c]) {
					inside = true;
					break;
				}
			}
			if (!inside) {
				return true;
			}
		}
	}

	return false;
}

static inline bool puzzle__try_place_clues(
		const struct puzzle *p,
		struct puzzle_line *line,
		size_t clue_idx,
		size_t pos)
{
	if (puzzle__can_place_clue(p, line, clue_idx, pos)) {
		if (puzzle__missed_set_cell(p, line)) {
			return true;
		}

		for (size_t c = 0; c < line->clue_count; c++) {
			size_t start = p->clue_start[c];
			for (size_t s = start; s < start + line->clue[c]; s++) {
				if (line->slot[s].done == false) {
					line->slot[s].value++;
				}
			}
		}

		line->slot_max++;
		return true;
	}

	return false;
}

static void puzzle__solve_slot_done(
		struct puzzle *p,
		struct puzzle_line *lines,
		size_t line_idx,
		size_t slot_idx)
{
	struct puzzle_line *line = &lines[line_idx];
	struct puzzle_line *other = (lines == p->col) ?
			&p->row[slot_idx] : &p->col[slot_idx];
	struct puzzle_slot *other_slot = &other->slot[line_idx];

	line->slot[slot_idx].done = true;
	line->total++;

	other->update_needed = true;
	other_slot->value = line->slot[slot_idx].value;
	other_slot->done = true;
	other->total++;

	p->cells_complete++;
}

static bool puzzle__solve_line(
		struct puzzle *p,
		struct puzzle_line *lines,
		size_t line_idx)
{
	bool placed;
	size_t clue;
	struct puzzle_line *line = &lines[line_idx];

	line->slot_max = 0;
	for (size_t s = 0; s < line->slot_count; s++) {
		if (line->slot[s].done == false) {
			line->slot[s].value = 0;
		}
	}

	placed = puzzle__try_place_clues(p, line, 0, 0);
	if (!placed) {
		fprintf(stderr, "ERROR: Couldn't fit clues on line!\n");
		return false;
	}

	clue = line->clue_count - 1;

	while (clue < line->clue_count) {
		size_t pos = p->clue_start[clue];

		if (puzzle__slot_is_set(&line->slot[pos]) ||
		    pos == line->slot_count) {
			clue--;
			continue;
		}
		pos++;

		placed = puzzle__try_place_clues(p, line, clue, pos);
		if (placed) {
			clue = line->clue_count - 1;
		} else {
			clue--;
		}
	}

	for (size_t s = 0; s < line->slot_count; s++) {
		if (line->slot[s].done == false) {
			if (line->slot[s].value == line->slot_max ||
			    line->slot[s].value == 0) {
				puzzle__solve_slot_done(p, lines, line_idx, s);
			}
		}
	}

	line->update_needed = false;
	output_event_notify(OUTPUT_EVENT_LINE);
	return true;
}

static bool puzzle__solve_pass(
		struct puzzle *p,
		struct puzzle_line *lines,
		size_t line_count)
{
	for (size_t i = 0; i < line_count; i++) {
		if (lines[i].total == lines[i].slot_count ||
		    lines[i].update_needed == false) {
			continue;
		}
		if (!puzzle__solve_line(p, lines, i)) {
			return false;
		}
	}

	output_event_notify(OUTPUT_EVENT_PASS);
	return true;
}

bool puzzle_is_complete(const struct puzzle *p)
{
	return p->cells_complete == p->col_count * p->row_count;
}

bool puzzle_solve(struct puzzle *p)
{
	size_t cells_complete = 0;
	bool ok = true;
	int pass = 0;

	while (ok && !puzzle_is_complete(p)) {
		bool vertical = pass & 0x1;
		struct puzzle_line *lines = vertical ? p->col : p->row;
		size_t line_count = vertical ? p->col_count : p->row_count;

		if (!puzzle__solve_pass(p, lines, line_count)) {
			ok = false;
			break;
		}

		if (vertical) {
			if (cells_complete == p->cells_complete) {
				fprintf(stderr, "Couldn't solve puzzle!\n");
				break;
			}

			cells_complete = p->cells_complete;
		}

		pass++;
	}

	output_event_notify(OUTPUT_EVENT_FINAL);
	return ok;
}
