/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#include <cgif.h>

#include "options.h"
#include "output.h"
#include "puzzle.h"
#include "grid.h"

static struct {
	const struct options *options;
	const struct puzzle *puzzle;
	struct grid *grid;

	size_t cells_complete;

	CGIF *gif;

	uint8_t palette[OUTPUT_STYLE__COUNT][3 * 256];
} output_g = {
	.palette = {
		[OUTPUT_STYLE_SIMPLE] = {
			  0,   0,   0, // Black
			127, 127, 127, // Grey
			255, 255, 255, // White
		},
	}
};

static inline uint16_t output__palette_colours(enum output_style style)
{
	uint16_t count[] = {
		[OUTPUT_STYLE_SIMPLE ] =   3,
		[OUTPUT_STYLE_DETAILS] = 256,
	};

	if (style >= sizeof(count) / sizeof(*count)) {
		return 0;
	}

	return count[style];
}

static bool output__add_frame(void)
{
	uint64_t delay = (puzzle_is_complete(output_g.puzzle)) ?
			output_g.options->final_delay : output_g.options->delay;
	CGIF_FrameConfig config = {
		.delay = (uint16_t)delay,
		.pImageData = output_g.grid->data,
		.genFlags = CGIF_FRAME_GEN_USE_DIFF_WINDOW,
	};

	if (cgif_addframe(output_g.gif, &config) != CGIF_OK) {
		fprintf(stderr, "Error adding GIF frame\n");
		return false;
	}

	return true;
}

static bool output__create(
		const struct options *opt,
		uint16_t width,
		uint16_t height)
{
	uint32_t attr_flags = 0;

	if (opt->event != OUTPUT_EVENT_FINAL) {
		attr_flags |= CGIF_ATTR_IS_ANIMATED;
	}

	output_g.gif = cgif_newgif(&(CGIF_Config) {
		.numLoops = 1,
		.width = width,
		.height = height,
		.path = opt->output,
		.attrFlags = attr_flags,
		.pGlobalPalette = output_g.palette[opt->style],
		.numGlobalPaletteEntries = output__palette_colours(
				(enum output_style) opt->style),
	});
	if (output_g.gif == NULL) {
		fprintf(stderr, "Failed to create output GIF file.\n");
		return false;
	}

	if (opt->event != OUTPUT_EVENT_FINAL) {
		if (!output__add_frame()) {
			return false;
		}
	}

	return true;
}

static void output__grid_update(void)
{
	const struct options *opt = output_g.options;
	const struct puzzle *p = output_g.puzzle;
	const struct grid *g = output_g.grid;
	size_t w = p->col_count;
	size_t h = p->row_count;
	uint8_t level;

	for (size_t y = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++) {
			struct puzzle_slot *slot_col = &p->col[x].slot[y];
			struct puzzle_slot *slot_row = &p->row[y].slot[x];

			assert(slot_col->done == slot_row->done);

			if (opt->style == OUTPUT_STYLE_SIMPLE) {
				if (slot_col->done) {
					if (slot_col->value == 0) {
						level = 2;
					} else {
						level = 0;
					}
				} else {
					level = 1;
				}
			} else {
				size_t slot_val = slot_col->value * p->row[y].slot_max +
				                  slot_row->value * p->col[x].slot_max;
				size_t slot_max = p->col[x].slot_max *
				                  p->row[y].slot_max *
				                  2ll;

				if (slot_col->done) {
					if (slot_col->value == 0) {
						level = 255;
					} else {
						level = 0;
					}
				} else {
					assert(slot_val <= slot_max);

					slot_val = slot_max - slot_val;
					level = (uint8_t)(slot_val * 255 /
							slot_max);
				}
			}

			for (size_t i = 0; i < opt->grid_size; i++) {
				size_t yy = y * opt->grid_size + i;
				bool border_y = i < opt->border_width;
				for (size_t j = 0; j < opt->grid_size; j++) {
					size_t xx = x * opt->grid_size + j;
					bool border_x = j < opt->border_width;
					size_t pos = yy * g->width + xx;

					if (border_x || border_y) {
						g->data[pos] = 0;
					} else {
						g->data[pos] = level;
					}
				}
			}
		}
	}

	output_g.cells_complete = p->cells_complete;
}

bool output_init(const struct options *opt,
		const struct puzzle *puzzle)
{
	uint16_t height;
	uint16_t width;
	size_t size;

	size = puzzle->col_count * opt->grid_size + opt->border_width;
	if (size > UINT16_MAX) {
		fprintf(stderr, "Output width too large for GIF format\n");
		return NULL;
	}
	width = (uint16_t) size;

	size = puzzle->row_count * opt->grid_size + opt->border_width;
	if (size > UINT16_MAX) {
		fprintf(stderr, "Output height too large for GIF format\n");
		return NULL;
	}
	height = (uint16_t) size;

	output_g.options = opt;
	output_g.puzzle = puzzle;

	output_g.grid = grid_create(width, height);
	if (output_g.grid == NULL) {
		return false;
	}
	output__grid_update();

	if (opt->output == NULL) {
		return true;
	}

	for (size_t i = 0; i < 256; i++) {
		size_t offset = i * 3;
		output_g.palette[OUTPUT_STYLE_DETAILS][offset + 0] = (uint8_t)i;
		output_g.palette[OUTPUT_STYLE_DETAILS][offset + 1] = (uint8_t)i;
		output_g.palette[OUTPUT_STYLE_DETAILS][offset + 2] = (uint8_t)i;
	}

	if (!output__create(opt, width, height)) {
		grid_free(output_g.grid);
		return false;
	}

	return true;
}

bool output_event_notify(enum output_event event)
{
	const struct puzzle *p = output_g.puzzle;

	if (event == OUTPUT_EVENT_PASS && output_g.options->progress) {
		fprintf(stderr, "Solved %zu of %zu cells\n", p->cells_complete,
				p->col_count * p->row_count);
	}

	if (output_g.options->keep_frames == false &&
	    output_g.options->style == OUTPUT_STYLE_SIMPLE &&
	    event != OUTPUT_EVENT_FINAL) {
		if (output_g.cells_complete == p->cells_complete) {
			return true;
		}
	}

	if (output_g.options->quiet == false &&
	    event == OUTPUT_EVENT_FINAL) {
		for (size_t r = 0; r < p->row_count; r++) {
			for (size_t s = 0; s < p->row[r].slot_count; s++) {
				if (p->row[r].slot[s].done) {
					if (p->row[r].slot[s].value == 0) {
						printf("  ");
					} else {
						printf("##");
					}
				} else {
					printf("><");
				}
			}
			printf("\n");
		}
	}

	if (event != output_g.options->event) {
		return true;
	}

	if (output_g.grid != NULL && output_g.gif != NULL) {
		output__grid_update();
		return output__add_frame();
	}

	return true;
}

void output_fini(void)
{
	grid_free(output_g.grid);

	if (output_g.gif != NULL) {
		cgif_close(output_g.gif);
	}
}
