/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef CARBON_ARCHIVE_H
#define CARBON_ARCHIVE_H

#include "carbon-common.h"
#include "carbon-memblock.h"
#include "carbon-memfile.h"
#include "carbon-compressor.h"
#include "carbon-columndoc.h"
#include "carbon-io-context.h"

CARBON_BEGIN_DECL

typedef struct carbon_strid_iter carbon_strid_iter_t; /* fordwarded from 'carbon-strid-iter.h' */

union carbon_archive_dic_flags;

typedef struct carbon_archive_record_flags
{
    struct {
        uint8_t is_sorted                 : 1;

        uint8_t RESERVED_2                : 1;
        uint8_t RESERVED_3                : 1;
        uint8_t RESERVED_4                : 1;
        uint8_t RESERVED_5                : 1;
        uint8_t RESERVED_6                : 1;
        uint8_t RESERVED_7                : 1;
        uint8_t RESERVED_8                : 1;
    } bits;
    uint8_t value;
} carbon_archive_record_flags_t;

typedef struct carbon_archive_prop_offs {
    carbon_off_t nulls;
    carbon_off_t bools;
    carbon_off_t int8s;
    carbon_off_t int16s;
    carbon_off_t int32s;
    carbon_off_t int64s;
    carbon_off_t uint8s;
    carbon_off_t uint16s;
    carbon_off_t uint32s;
    carbon_off_t uint64s;
    carbon_off_t floats;
    carbon_off_t strings;
    carbon_off_t objects;
    carbon_off_t null_arrays;
    carbon_off_t bool_arrays;
    carbon_off_t int8_arrays;
    carbon_off_t int16_arrays;
    carbon_off_t int32_arrays;
    carbon_off_t int64_arrays;
    carbon_off_t uint8_arrays;
    carbon_off_t uint16_arrays;
    carbon_off_t uint32_arrays;
    carbon_off_t uint64_arrays;
    carbon_off_t float_arrays;
    carbon_off_t string_arrays;
    carbon_off_t object_arrays;
} carbon_archive_prop_offs_t;

typedef union
{
    struct {
        uint32_t has_null_props            : 1;
        uint32_t has_bool_props            : 1;
        uint32_t has_int8_props            : 1;
        uint32_t has_int16_props           : 1;
        uint32_t has_int32_props           : 1;
        uint32_t has_int64_props           : 1;
        uint32_t has_uint8_props           : 1;
        uint32_t has_uint16_props          : 1;
        uint32_t has_uint32_props          : 1;
        uint32_t has_uint64_props          : 1;
        uint32_t has_float_props           : 1;
        uint32_t has_string_props          : 1;
        uint32_t has_object_props          : 1;
        uint32_t has_null_array_props      : 1;
        uint32_t has_bool_array_props      : 1;
        uint32_t has_int8_array_props      : 1;
        uint32_t has_int16_array_props     : 1;
        uint32_t has_int32_array_props     : 1;
        uint32_t has_int64_array_props     : 1;
        uint32_t has_uint8_array_props     : 1;
        uint32_t has_uint16_array_props    : 1;
        uint32_t has_uint32_array_props    : 1;
        uint32_t has_uint64_array_props    : 1;
        uint32_t has_float_array_props     : 1;
        uint32_t has_string_array_props    : 1;
        uint32_t has_object_array_props    : 1;

        uint32_t RESERVED_27               : 1;
        uint32_t RESERVED_28               : 1;
        uint32_t RESERVED_29               : 1;
        uint32_t RESERVED_30               : 1;
        uint32_t RESERVED_31               : 1;
        uint32_t RESERVED_32               : 1;
    } bits;
    uint32_t value;
} carbon_archive_object_flags_t;

typedef struct carbon_archive_string_table
{
    carbon_compressor_t     compressor;
    carbon_off_t            first_entry_off;

} carbon_archive_string_table_t;

typedef struct carbon_archive_record_table
{
    carbon_archive_record_flags_t   flags;
    carbon_memblock_t              *recordDataBase;
} carbon_archive_record_table_t;

typedef struct carbon_archive_info
{
    size_t string_table_size;
    size_t record_table_size;
} carbon_archive_info_t;

typedef struct carbon_archive_object
{
    carbon_memfile_t               file;
    carbon_archive_record_table_t *context;
    carbon_archive_object_flags_t  flags;
    carbon_archive_prop_offs_t     props;
    carbon_off_t                   self;
    carbon_err_t                   err;
} carbon_archive_object_t;

typedef struct carbon_archive_table
{
    size_t                         ngroups;
    const carbon_string_id_t      *keys;
    const carbon_off_t            *groups_offsets;
    carbon_archive_object_t       *context;
    carbon_err_t                   err;
} carbon_archive_table_t;

typedef struct carbon_column_group
{
    size_t                         ncolumns;
    const carbon_off_t            *column_offsets;
    carbon_archive_object_t       *context;
    carbon_err_t                   err;
} carbon_column_group_t;

typedef struct carbon_column
{
    size_t                         nelems;
    carbon_field_type_e            type;
    const carbon_off_t            *entry_offsets;
    const uint32_t                *position_list;
    carbon_archive_object_t       *context;
    carbon_err_t                   err;
} carbon_column_t;

typedef struct carbon_field
{
    carbon_off_t                   data_offset;
    uint32_t                       nentries;
    carbon_field_type_e            type;
    carbon_archive_object_t       *context;
    const void                    *data;
} carbon_field_t;

typedef struct carbon_object_cursor
{
    carbon_field_t *field;
    carbon_memblock_t *mem_block;
    uint32_t current_idx;
    uint32_t max_idx;
    carbon_archive_object_t obj;
} carbon_object_cursor_t;

typedef struct carbon_archive
{
    carbon_archive_info_t         info;
    char                         *diskFilePath;
    carbon_archive_string_table_t string_table;
    carbon_archive_record_table_t record_table;
    carbon_err_t                  err;
} carbon_archive_t;


typedef struct __attribute__((packed)) carbon_string_entry_header
{
    char                          marker;
    carbon_off_t                  next_entry_off;
    carbon_string_id_t            string_id;
    uint32_t                      string_len;
} carbon_string_entry_header_t;

CARBON_EXPORT(bool)
carbon_archive_from_json(carbon_archive_t *out,
                         const char *file,
                         carbon_err_t *err,
                         const char *json_string,
                         carbon_compressor_type_e compressor,
                         bool read_optimized);

CARBON_EXPORT(bool)
carbon_archive_stream_from_json(carbon_memblock_t **stream,
                                carbon_err_t *err,
                                const char *json_string,
                                carbon_compressor_type_e compressor,
                                bool read_optimized);

CARBON_EXPORT(bool)
carbon_archive_from_model(carbon_memblock_t **stream,
                          carbon_err_t *err,
                          carbon_columndoc_t *model,
                          carbon_compressor_type_e compressor);


CARBON_EXPORT(bool)
carbon_archive_write(FILE *file, const carbon_memblock_t *stream);

CARBON_EXPORT(bool)
carbon_archive_load(carbon_memblock_t **stream, FILE *file);

CARBON_EXPORT(bool)
carbon_archive_print(FILE *file, carbon_err_t *err, carbon_memblock_t *stream);

CARBON_EXPORT(bool)
carbon_archive_open(carbon_archive_t *out, const char *file_path);

CARBON_EXPORT(bool)
carbon_archive_get_info(carbon_archive_info_t *info, const struct carbon_archive *archive);

CARBON_DEFINE_GET_ERROR_FUNCTION(archive, carbon_archive_t, archive);

CARBON_EXPORT(bool)
carbon_archive_close(carbon_archive_t *archive);



/**
 * Creates a new <code>carbon_io_context_t</code> to access the archives underlying file for unsafe operations.
 *
 * An unsafe operation directly seeks randomly in the underlying file. To avoid creation of multiple file
 * descriptors while at the same time allow to access unsafe operations in a multi-threading environment, an
 * <code>carbon_io_context_t</code> is used. Roughly, such a context is a regular FILE that is protected by a lock.
 *
 * @param archive The archive
 * @return a heap-allocated instance of <code>carbon_io_context_t</code>, or NULL if not successful
 */
CARBON_EXPORT(carbon_io_context_t *)
carbon_archive_io_context_create(carbon_archive_t *archive);



CARBON_EXPORT(bool)
carbon_archive_query_fullscan_strids(carbon_strid_iter_t *it, carbon_archive_t *archive);

CARBON_EXPORT(char *)
carbon_archive_query_fullscan_string_by_id(carbon_archive_t *archive, carbon_string_id_t id);

CARBON_EXPORT(char *)
carbon_archive_query_unsafe_string_by_offset(carbon_io_context_t *context, carbon_archive_t *archive, carbon_off_t off, size_t strlen);


CARBON_EXPORT(bool)
carbon_archive_record(carbon_archive_object_t *root, carbon_archive_t *archive);

CARBON_EXPORT(CARBON_NULLABLE const carbon_string_id_t *)
carbon_archive_object_keys_to_type(CARBON_NULLABLE size_t *npairs, carbon_type_e type, carbon_archive_object_t *obj);

CARBON_EXPORT(CARBON_NULLABLE const carbon_string_id_t *)
carbon_archive_object_keys_to_array(CARBON_NULLABLE size_t *npairs, carbon_type_e type, carbon_archive_object_t *obj);

CARBON_EXPORT(bool)
carbon_archive_object_values_object(carbon_archive_object_t *out, size_t idx, carbon_archive_object_t *props);

CARBON_EXPORT(const carbon_int8_t *)
carbon_archive_object_values_int8(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_int16_t *)
carbon_archive_object_values_int16s(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_int32_t *)
carbon_archive_object_values_int32(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_int64_t *)
carbon_archive_object_values_int64s(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_uint8_t *)
carbon_archive_object_values_uint8s(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_uint16_t *)
carbon_archive_object_values_uin16(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_uin32_t *)
carbon_archive_object_values_uint32(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_uin64_t *)
carbon_archive_object_values_uint64(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_bool_t *)
carbon_archive_object_values_bool(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_float_t *)
carbon_archive_object_values_float(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_string_id_t *)
carbon_archive_object_values_strings(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_int8_t *)
carbon_archive_object_values_int8_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_int16_t *)
carbon_archive_object_values_int16_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_int32_t *)
carbon_archive_object_values_int32_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_int64_t *)
carbon_archive_object_values_int64_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_uint8_t *)
carbon_archive_object_values_uint8_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_uint16_t *)
carbon_archive_object_values_uint16_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_uin32_t *)
carbon_archive_object_values_uint32_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_uin64_t *)
carbon_archive_object_values_uint64_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_bool_t *)
carbon_archive_object_values_bool_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_float_t *)
carbon_archive_object_values_float_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(const carbon_string_id_t *)
carbon_archive_object_values_string_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(bool)
carbon_archive_object_values_null_array_lengths(uint32_t *length, size_t idx, carbon_archive_object_t *obj);

CARBON_EXPORT(bool)
carbon_archive_table_open(carbon_archive_table_t *out, carbon_archive_object_t *obj);

CARBON_DEFINE_GET_ERROR_FUNCTION(carbon_archive_table, carbon_archive_table_t, table);

CARBON_EXPORT(bool)
carbon_archive_table_column_group(carbon_column_group_t *group, size_t idx, carbon_archive_table_t *table);

CARBON_DEFINE_GET_ERROR_FUNCTION(archive_column_group, carbon_column_group_t, group);

CARBON_EXPORT(bool)
carbon_archive_table_column(carbon_column_t *column, size_t idx, carbon_column_group_t *group);

CARBON_DEFINE_GET_ERROR_FUNCTION(archive_column, carbon_column_t, column);

CARBON_EXPORT(bool)
carbon_archive_table_field_get(carbon_field_t *field, size_t idx, carbon_column_t *column);

CARBON_EXPORT(bool)
carbon_archive_table_field_type(carbon_field_type_e *type, const carbon_field_t *field);

CARBON_EXPORT(const carbon_int8_t *)
carbon_archive_table_field_get_int8_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_int16_t *)
carbon_archive_table_field_get_int16_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_int32_t *)
carbon_archive_table_field_get_int32_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_int64_t *)
carbon_archive_table_field_get_int64_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_uint8_t *)
carbon_archive_table_field_get_uint8_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_uint16_t *)
carbon_archive_table_field_get_uint16_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_uin32_t *)
carbon_archive_table_field_get_uint32_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_uin64_t *)
carbon_archive_table_field_get_uint64_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_bool_t *)
carbon_archive_table_field_get_bool_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_float_t *)
carbon_archive_table_field_get_float_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(const carbon_string_id_t *)
carbon_archive_table_field_get_string_array(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(bool)
carbon_archive_table_field_get_null_array_lengths(uint32_t *length, const carbon_field_t *field);

CARBON_EXPORT(bool)
carbon_archive_table_field_object_cursor_open(carbon_object_cursor_t *cursor, carbon_field_t *field);

CARBON_EXPORT(bool)
carbon_archive_table_field_object_cursor_next(carbon_archive_object_t **obj, carbon_object_cursor_t *cursor);

CARBON_END_DECL

#endif