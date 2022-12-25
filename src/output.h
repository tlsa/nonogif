/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#ifndef OUTPUT_H
#define OUTPUT_H

struct options;
struct puzzle;

enum output_event {
	OUTPUT_EVENT_LINE,  // One frame for every line.
	OUTPUT_EVENT_PASS,  // One frame for each pass of the puzzle.
	OUTPUT_EVENT_FINAL, // No animation, just output final result.
};

enum output_style {
	OUTPUT_STYLE_SIMPLE,
	OUTPUT_STYLE_DETAILS,
	OUTPUT_STYLE__COUNT,
};

struct output_options {
	const char *path;

	int64_t event;
	int64_t style;

	uint32_t grid_size;
	uint32_t border_width;
};

bool output_init(
		const struct options *opt,
		const struct puzzle *puzzle);

bool output_event_notify(
		enum output_event event);

void output_fini(void);

#endif /* OUTPUT_H */
