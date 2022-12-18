/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "options.h"

int main(int argc, const char *argv[])
{
	const struct options *options;
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

	exit_code = EXIT_SUCCESS;

	return exit_code;
}
