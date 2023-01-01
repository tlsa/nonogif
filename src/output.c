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

	uint8_t palette[3 * 256];
	uint16_t palette_count;
	size_t border_index;
	size_t set_index;
} output_g;

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
		.pGlobalPalette = output_g.palette,
		.numGlobalPaletteEntries = output_g.palette_count,
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

static uint8_t output__get_level(size_t x, size_t y)
{
	const struct puzzle *p = output_g.puzzle;
	const struct options *opt = output_g.options;
	struct puzzle_slot *slot_col = &p->col[x].slot[y];
	struct puzzle_slot *slot_row = &p->row[y].slot[x];
	uint8_t level_set = (uint8_t)output_g.set_index;
	uint8_t level;

	assert(slot_col->done == slot_row->done);

	if (slot_col->done) {
		if (slot_col->value == 0) {
			level = 0;
		} else {
			level = level_set;
		}
	} else if (opt->style == OUTPUT_STYLE_SIMPLE) {
		level = level_set / 2;
	} else {
		size_t slot_val = slot_col->value * p->row[y].slot_max +
		                  slot_row->value * p->col[x].slot_max;
		size_t slot_max = p->col[x].slot_max *
		                  p->row[y].slot_max * 2ll;
		size_t max_idx = level_set;

		assert(slot_val <= slot_max);

		slot_val = slot_max - slot_val;
		level = (uint8_t)(max_idx -
				(slot_val * max_idx /
				slot_max));
	}

	return level;
}

static void output__grid_update(void)
{
	const struct options *opt = output_g.options;
	const struct puzzle *p = output_g.puzzle;
	const struct grid *g = output_g.grid;
	size_t w = p->col_count;
	size_t h = p->row_count;
	uint8_t level_border;

	level_border = (uint8_t)output_g.border_index;

	for (size_t y = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++) {
			uint8_t level = output__get_level(x, y);

			for (size_t i = 0; i < opt->grid_size; i++) {
				size_t yy = y * opt->grid_size + i;
				bool border_y = i < opt->border_width;
				for (size_t j = 0; j < opt->grid_size; j++) {
					size_t xx = x * opt->grid_size + j;
					bool border_x = j < opt->border_width;
					size_t pos = yy * g->width + xx;

					if (border_x || border_y) {
						g->data[pos] = level_border;
					} else {
						g->data[pos] = level;
					}
				}
			}
		}
	}

	for (size_t y = 0; y < g->height; y++) {
		for (size_t x = w * opt->grid_size; x < g->width; x++) {
			size_t pos = y * g->width + x;
			g->data[pos] = level_border;
		}
	}

	for (size_t y = h * opt->grid_size; y < g->height; y++) {
		for (size_t x = 0; x < g->width; x++) {
			size_t pos = y * g->width + x;
			g->data[pos] = level_border;
		}
	}

	output_g.cells_complete = p->cells_complete;
}

static void output__generate_palette_spectrum(int count)
{
	enum { RED, GREEN, BLUE, COUNT };
	int set[COUNT] = {
		[RED  ] = (output_g.options->colour.set >> 16) & 0xFF,
		[GREEN] = (output_g.options->colour.set >>  8) & 0xFF,
		[BLUE ] = (output_g.options->colour.set >>  0) & 0xFF,
	};
	int clear[COUNT] = {
		[RED  ] = (output_g.options->colour.clear >> 16) & 0xFF,
		[GREEN] = (output_g.options->colour.clear >>  8) & 0xFF,
		[BLUE ] = (output_g.options->colour.clear >>  0) & 0xFF,
	};
	uint8_t *p = output_g.palette;

	for (int i = 0; i < count; i++) {
		for (size_t c = 0; c < COUNT; c++) {
			*p++ = (uint8_t)((i * (set[c] - clear[c])) /
					(count - 1) + clear[c]);
		}
	}

	output_g.set_index = (size_t)(count - 1);
}

static void output__generate_palette(void)
{
	enum { RED, GREEN, BLUE, COUNT };
	int border[COUNT] = {
		[RED  ] = (output_g.options->colour.border >> 16) & 0xFF,
		[GREEN] = (output_g.options->colour.border >>  8) & 0xFF,
		[BLUE ] = (output_g.options->colour.border >>  0) & 0xFF,
	};

	output_g.palette_count = 3;

	if (output_g.options->style == OUTPUT_STYLE_DETAILS) {
		output_g.palette_count = 256;
	}

	output__generate_palette_spectrum((int)output_g.palette_count);

	if (output_g.options->border_width != 0) {
		bool have_border_colour = false;

		for (int i = 0; i < output_g.palette_count; i++) {
			uint8_t *p = &output_g.palette[i * 3];

			if (border[RED  ] == p[RED  ] &&
			    border[GREEN] == p[GREEN] &&
			    border[BLUE ] == p[BLUE ]) {
				have_border_colour = true;
				output_g.border_index = (size_t)i;
			}
		}

		if (have_border_colour == false) {
			uint8_t *p;

			if (output_g.palette_count == 256) {
				output_g.palette_count--;
				output__generate_palette_spectrum(
						(int)output_g.palette_count);
			}

			output_g.border_index = output_g.palette_count;
			p = &output_g.palette[output_g.border_index * 3];

			p[RED  ] = (uint8_t)border[RED  ];
			p[GREEN] = (uint8_t)border[GREEN];
			p[BLUE ] = (uint8_t)border[BLUE ];

			output_g.palette_count++;
		}
	}
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
	output__generate_palette();

	output_g.grid = grid_create(width, height);
	if (output_g.grid == NULL) {
		return false;
	}

	output__grid_update();

	if (opt->output == NULL) {
		return true;
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
