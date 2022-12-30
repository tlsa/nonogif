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
#include "options.h"

int main(int argc, const char *argv[])
{
	const struct options *options;
	struct puzzle *puzzle;
	int exit_code;

	options = options_parse(argc, argv);
	if (options == NULL) {
		return EXIT_FAILURE;
	}

	if (options->help) {
		options_print_usage(argv);
		return EXIT_SUCCESS;

	} else if (options->version) {
		int major = VERSION_MAJOR;
		int minor = VERSION_MINOR;
		int patch = VERSION_PATCH;

		printf("%s version %d.%d.%d\n", argv[0], major, minor, patch);
		return EXIT_SUCCESS;

	} else if (options->input == NULL) {
		fprintf(stderr, "No input YAML puzzle file provided!\n");
		options_print_usage(argv);
		return EXIT_FAILURE;
	}

	puzzle = puzzle_create(options->input);
	if (puzzle == NULL) {
		fprintf(stderr, "Failed to create puzzle!\n");
		return EXIT_FAILURE;
	}

	if (!output_init(options, puzzle)) {
		fprintf(stderr, "Failed to initialise output!\n");
		exit_code = EXIT_FAILURE;
		goto exit;
	}

	if (!puzzle_solve(puzzle)) {
		fprintf(stderr, "Failed to solve puzzle!\n");
		exit_code = EXIT_FAILURE;
		goto exit;
	}

	exit_code = EXIT_SUCCESS;

exit:
	output_fini();
	puzzle_free(puzzle);
	return exit_code;
}
