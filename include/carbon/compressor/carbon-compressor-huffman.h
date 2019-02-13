/**
 * Copyright 2019 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CARBON_COMPRESSOR_HUFFMAN_H
#define CARBON_COMPRESSOR_HUFFMAN_H

#include "carbon/carbon-common.h"
#include "carbon/carbon-vector.h"
#include "carbon/carbon-memfile.h"

CARBON_BEGIN_DECL

typedef struct carbon_compressor carbon_compressor_t; /* forwarded from 'carbon-compressor.h' */

void compressor_huffman_write_dictionary(carbon_compressor_t *self, carbon_memfile_t *memfile, const carbon_vec_t ofType (const char *) *strings,
                                         const carbon_vec_t ofType(carbon_string_id_t) *string_ids);

void compressor_huffman_dump_dictionary(carbon_compressor_t *self, FILE *file, carbon_memfile_t *memfile);

bool compressor_huffman_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err, carbon_string_id_t string_id,
                                      const char *string);

char *compressor_huffman_decode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err, carbon_string_id_t string_id);


CARBON_END_DECL

#endif
