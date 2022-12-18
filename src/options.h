/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#ifndef OPTIONS_H
#define OPTIONS_H

struct options {
	bool help;
	bool version;

	const char *input;
};

const struct options *options_parse(int argc, const char *argv[]);

void options_print_usage(const char *argv[]);

#endif /* OPTIONS_H */
