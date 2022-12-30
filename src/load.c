/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2022 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include <cyaml/cyaml.h>

#include "load.h"
#include "puzzle.h"

static const cyaml_schema_value_t schema_puzzle_line_entry = {
	CYAML_VALUE_UINT(CYAML_FLAG_DEFAULT, size_t),
};

static const cyaml_schema_field_t schema_puzzle_line_fields[] = {
	CYAML_FIELD_SEQUENCE("clues", CYAML_FLAG_POINTER,
			struct puzzle_line, clue,
			&schema_puzzle_line_entry, 0, CYAML_UNLIMITED),
	CYAML_FIELD_END
};

static const cyaml_schema_value_t schema_puzzle_line = {
	CYAML_VALUE_MAPPING(CYAML_FLAG_DEFAULT,
			struct puzzle_line, schema_puzzle_line_fields),
};

static const cyaml_schema_field_t schema_puzzle_fields[] = {
	CYAML_FIELD_STRING_PTR("name", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL,
			struct puzzle, name, 0, CYAML_UNLIMITED),
	CYAML_FIELD_SEQUENCE("rows", CYAML_FLAG_POINTER,
			struct puzzle, row,
			&schema_puzzle_line, 0, CYAML_UNLIMITED),
	CYAML_FIELD_SEQUENCE("columns", CYAML_FLAG_POINTER,
			struct puzzle, col,
			&schema_puzzle_line, 0, CYAML_UNLIMITED),
	CYAML_FIELD_END
};

static const cyaml_schema_value_t schema = {
	CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER,
			struct puzzle, schema_puzzle_fields),
};

static const cyaml_config_t cfg = {
	.log_fn = cyaml_log,            /* Use the default logging function. */
	.mem_fn = cyaml_mem,            /* Use the default memory allocator. */
	.log_level = CYAML_LOG_WARNING, /* Logging errors and warnings only. */
};

struct puzzle *load_file(
		const char *path)
{
	cyaml_err_t err;
	struct puzzle *p;

	err = cyaml_load_file(path, &cfg, &schema, (cyaml_data_t **)&p, NULL);
	if (err != CYAML_OK) {
		fprintf(stderr, "ERROR: %s\n", cyaml_strerror(err));
		return NULL;
	}

	return p;
}
