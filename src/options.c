/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdlib.h>
#include <stdint.h>

#include "cli.h"
#include "options.h"

/** Default options, overwritten by CLI arguments. */
static struct options options = {
	0
};

static const struct cli_table_entry cli_entries[] = {
	{
		.p = true,
		.l = "FILE",
		.t = CLI_STRING,
		.v.s = &options.input,
		.d = "Path to YAML input file.",
	},
	{
		.s = 'h',
		.l = "help",
		.t = CLI_SBOOL,
		.v.b = &options.help,
		.d = "Print this text.",
	},
	{
		.s = 'v',
		.l = "version",
		.t = CLI_SBOOL,
		.v.b = &options.version,
		.d = "Print version information.",
	},
};

const struct cli_table cli = {
	.entries = cli_entries,
	.count = (sizeof(cli_entries))/(sizeof(*cli_entries)),
	.min_positional = 1,
};

const struct options *options_parse(int argc, const char *argv[])
{

	if (!cli_parse(&cli, argc, (void *)argv)) {
		cli_help(&cli, argv[0]);
		return NULL;
	}

	return &options;
}

void options_print_usage(const char *argv[])
{
	cli_help(&cli, argv[0]);
}
