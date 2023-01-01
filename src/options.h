/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#ifndef OPTIONS_H
#define OPTIONS_H

struct options_colour {
	uint64_t set;
	uint64_t clear;
	uint64_t border;
};

struct options {
	bool help;
	bool quiet;
	bool version;
	bool progress;
	bool keep_frames;

	const char *input;
	const char *output;

	int64_t event;
	int64_t style;

	uint64_t delay;
	uint64_t final_delay;

	uint64_t grid_size;
	uint64_t border_width;

	struct options_colour colour;
};

const struct options *options_parse(int argc, const char *argv[]);

void options_print_usage(const char *argv[]);

#endif /* OPTIONS_H */
