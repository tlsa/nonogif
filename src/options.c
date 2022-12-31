/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdlib.h>
#include <stdint.h>

#include "cli.h"
#include "output.h"
#include "options.h"

/** Default options, overwritten by CLI arguments. */
static struct options options = {
	.delay = 20,
	.grid_size = 16,
	.border_width = 1,
	.final_delay = 500,
	.event = OUTPUT_EVENT_LINE,
	.style = OUTPUT_STYLE_SIMPLE,
};

static struct cli_str_val cli_img_opt_style[] = {
	{
		.str = "simple",
		.val = OUTPUT_STYLE_SIMPLE,
		.d   = "Cells in the output have three colours. They are "
		       "either Set, Clear or a mid-value for anything "
		       "currently Unknown.",
	},
	{
		.str = "detail",
		.val = OUTPUT_STYLE_DETAILS,
		.d   = "Cells in the output still use the Set and Clear "
		       "values, but the Unknown cells maybe anywhere in the "
		       "spectrum between the Set and Clear colour, indicating "
		       "the likelihood that the cell will end up set or clear.",
	},
	{ .str = NULL },
};

static struct cli_str_val cli_img_opt_event[] = {
	{
		.str = "line",
		.val = OUTPUT_EVENT_LINE,
		.d   = "Output a frame of animation for each line considered.",
	},
	{
		.str = "pass", .val = OUTPUT_EVENT_PASS,
		.d   = "Output a frame of animation for each pass of the whole "
		       "puzzle.",
	},
	{
		.str = "final",
		.val = OUTPUT_EVENT_FINAL,
		.d   = "Only output the final result image. (No animation.)",
	},
	{ .str = NULL },
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
		.s = 'b',
		.l = "border-width",
		.t = CLI_UINT,
		.v.u = &options.border_width,
		.d = "Set grid border thickness in pixels.",
	},
	{
		.s = 'd',
		.l = "delay",
		.t = CLI_UINT,
		.v.u = &options.delay,
		.d = "Delay between frames (cs).\n"
		     "Suggest minimum of 2.",
	},
	{
		.s = 'e',
		.l = "event",
		.t = CLI_ENUM,
		.v.e.e = &options.event,
		.v.e.desc = cli_img_opt_event,
		.d = "Set which event triggers a new GIF frame.",
	},
	{
		.s = 'f',
		.l = "final-delay",
		.t = CLI_UINT,
		.v.u = &options.final_delay,
		.d = "Time to pause showing final frame (cs).",
	},
	{
		.s = 'g',
		.l = "grid-size",
		.t = CLI_UINT,
		.v.u = &options.grid_size,
		.d = "Set the grid size in pixels.",
	},
	{
		.s = 'h',
		.l = "help",
		.t = CLI_BOOL,
		.no_pos = true,
		.v.b = &options.help,
		.d = "Print this text.",
	},
	{
		.s = 'k',
		.l = "keep-frames",
		.t = CLI_BOOL,
		.v.b = &options.keep_frames,
		.d = "Keep GIF frames with no changes.",
	},
	{
		.s = 'o',
		.l = "output",
		.t = CLI_STRING,
		.v.s = &options.output,
		.d = "Output a GIF file at the given path.",
	},
	{
		.s = 'p',
		.l = "progress",
		.t = CLI_BOOL,
		.v.b = &options.progress,
		.d = "Print solver progress.",
	},
	{
		.s = 'q',
		.l = "quiet",
		.t = CLI_BOOL,
		.v.b = &options.quiet,
		.d = "Don't print final result.",
	},
	{
		.s = 's',
		.l = "style",
		.t = CLI_ENUM,
		.v.e.e = &options.style,
		.v.e.desc = cli_img_opt_style,
		.d = "Set the output GIF style to use.",
	},
	{
		.s = 'v',
		.l = "version",
		.t = CLI_BOOL,
		.no_pos = true,
		.v.b = &options.version,
		.d = "Print version information.",
	},
};

const struct cli_table cli = {
	.entries = cli_entries,
	.count = (sizeof(cli_entries))/(sizeof(*cli_entries)),
	.min_positional = 1,
	.d = "NonoGIF is a tool for generating animated GIFs of Nonogram "
	     "solutions.",
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
