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

#include <inttypes.h>
#include <assert.h>
#include <carbon/carbon-compressor.h>
#include "carbon/compressor/carbon-compressor-none.h"

CARBON_EXPORT(bool)
carbon_compressor_none_init(carbon_compressor_t *self)
{
    CARBON_UNUSED(self);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_none_cpy(const carbon_compressor_t *self, carbon_compressor_t *dst)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_NONE);

    /* nothing to hard copy but the function pointers */
    *dst = *self;
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_none_drop(carbon_compressor_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_NONE);

    CARBON_UNUSED(self);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_none_write_extra(carbon_compressor_t *self, carbon_memfile_t *dst,
                                        const carbon_vec_t ofType (const char *) *strings)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_NONE);

    CARBON_UNUSED(self);
    CARBON_UNUSED(dst);
    CARBON_UNUSED(strings);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

bool carbon_compressor_none_print_extra(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_NONE);

    CARBON_UNUSED(self);
    CARBON_UNUSED(file);
    CARBON_UNUSED(src);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_none_print_encoded_string(carbon_compressor_t *self,
                                                 FILE *file,
                                                 carbon_memfile_t *src,
                                                 uint32_t decompressed_strlen)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_NONE);

    CARBON_UNUSED(self);

    const char         *string        =  CARBON_MEMFILE_READ(src, decompressed_strlen);

    char *printableString = malloc(decompressed_strlen + 1);
    memcpy(printableString, string, decompressed_strlen);
    printableString[decompressed_strlen] = '\0';

    fprintf(file, "[string: %s]", printableString);

    free(printableString);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_none_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                                          const char *string)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_NONE);

    CARBON_UNUSED(self);

    uint32_t string_length = strlen(string);

    CARBON_SUCCESS_OR_JUMP(carbon_memfile_write(dst, string, string_length), error_handling)

    return true;

error_handling:
    CARBON_ERROR(err, CARBON_ERR_IO)
    return false;
}

CARBON_EXPORT(bool)
carbon_compressor_none_decode_string(carbon_compressor_t *self, char *dst, size_t strlen, FILE *src)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_NONE);

    CARBON_UNUSED(self);

    size_t num_read = fread(dst, sizeof(char), strlen, src);
    return (num_read == strlen);
}
