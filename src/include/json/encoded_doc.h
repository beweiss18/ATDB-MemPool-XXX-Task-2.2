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

#ifndef NG5_ENCODED_DOC_H
#define NG5_ENCODED_DOC_H

#include "shared/common.h"
#include "std/hash_table.h"
#include "core/oid/oid.h"
#include "shared/types.h"
#include "core/carbon/archive.h"

NG5_BEGIN_DECL

typedef struct carbon_encoded_doc_collection carbon_encoded_doc_collection_t;

typedef struct carbon_encoded_doc carbon_encoded_doc_t;

typedef union {
    field_i8_t int8;
    field_i16_t int16;
    field_i32_t int32;
    field_i64_t int64;
    field_u8_t uint8;
    field_u16_t uint16;
    field_u32_t uint32;
    field_u64_t uint64;
    field_number_t number;
    field_boolean_t boolean;
    field_sid_t string;
    carbon_object_id_t object;
    u32 null;
} carbon_encoded_doc_value_t;

typedef enum
{
    NG5_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING,
    NG5_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING,
} carbon_encoded_doc_prop_string_type_e;

typedef enum
{
    NG5_ENCODED_DOC_PROP_VALUE_TYPE_BUILTIN,
    NG5_ENCODED_DOC_PROP_VALUE_TYPE_DECODED_STRING,
} carbon_encoded_doc_prop_value_type_e;

typedef struct
{
    carbon_encoded_doc_t                  *context;

    carbon_encoded_doc_prop_string_type_e  key_type;
    union {
        field_sid_t          key_id;
        char                       *key_str;
    } key;

    carbon_encoded_doc_prop_value_type_e   value_type;
    enum field_type                    type;

} carbon_encoded_doc_prop_header_t;

typedef struct
{
    carbon_encoded_doc_prop_header_t header;

    union {
        carbon_encoded_doc_value_t       builtin;
        char                            *string;
    } value;

} carbon_encoded_doc_prop_t;

typedef struct
{
    carbon_encoded_doc_prop_header_t header;
    struct vector ofType(carbon_encoded_doc_value_t) values;
} carbon_encoded_doc_prop_array_t;

typedef struct carbon_encoded_doc
{
    carbon_encoded_doc_collection_t                      *context;
    carbon_object_id_t                                    object_id;
    struct vector ofType(carbon_encoded_doc_prop_t)        props;
    struct vector ofType(carbon_encoded_doc_prop_array_t)  props_arrays;
    carbon_hashtable_t ofMapping(field_sid_t, u32) prop_array_index; /* maps key to index in prop arrays */

    struct err err;

} carbon_encoded_doc_t;



typedef struct carbon_encoded_doc_collection
{
    struct archive *archive;

    struct vector ofType(carbon_encoded_doc_t) flat_object_collection;   /* list of objects; also nested ones */
    carbon_hashtable_t ofMapping(carbon_object_id_t, u32) index;   /* maps oid to index in collection */

    struct err err;

} carbon_encoded_doc_collection_t;

NG5_EXPORT(bool)
carbon_encoded_doc_collection_create(carbon_encoded_doc_collection_t *collection, struct err *err,
                                     struct archive *archive);

NG5_EXPORT(bool)
carbon_encoded_doc_collection_drop(carbon_encoded_doc_collection_t *collection);

NG5_EXPORT(carbon_encoded_doc_t *)
encoded_doc_collection_get_or_append(carbon_encoded_doc_collection_t *collection, carbon_object_id_t id);

NG5_EXPORT(bool)
carbon_encoded_doc_collection_print(FILE *file, carbon_encoded_doc_collection_t *collection);

NG5_EXPORT(bool)
carbon_encoded_doc_drop(carbon_encoded_doc_t *doc);

NG5_EXPORT(bool)
carbon_encoded_doc_get_object_id(carbon_object_id_t *oid, carbon_encoded_doc_t *doc);

#define DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(name, built_in_type)                                                  \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_##name(carbon_encoded_doc_t *doc, field_sid_t key, built_in_type value);

#define DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(name, built_in_type)                                          \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_##name##_decoded(carbon_encoded_doc_t *doc, const char *key, built_in_type value);

DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(int8, field_i8_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(int16, field_i16_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(int32, field_i32_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(int64, field_i64_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(uint8, field_u8_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(uint16, field_u16_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(uint32, field_u32_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(uint64, field_u64_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(number, field_number_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(boolean, field_boolean_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC(string, field_sid_t)

DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int8, field_i8_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int16, field_i16_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int32, field_i32_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int64, field_i64_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint8, field_u8_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint16, field_u16_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint32, field_u32_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint64, field_u64_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(number, field_number_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(boolean, field_boolean_t)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(string, field_sid_t)

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_string_decoded_string_value_decoded(carbon_encoded_doc_t *doc, const char *key, const char *value);

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_null(carbon_encoded_doc_t *doc, field_sid_t key);

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_null_decoded(carbon_encoded_doc_t *doc, const char *key);

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_object(carbon_encoded_doc_t *doc, field_sid_t key, carbon_encoded_doc_t *value);

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_object_decoded(carbon_encoded_doc_t *doc, const char *key, carbon_encoded_doc_t *value);

#define DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name)                                                            \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_array_##name(carbon_encoded_doc_t *doc, field_sid_t key);

#define DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name)                                                    \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_array_##name##_decoded(carbon_encoded_doc_t *doc, const char *key);

DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object)

DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null)
DEFINE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object)

#define DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type)                                                 \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_array_push_##name(carbon_encoded_doc_t *doc, field_sid_t key,                                \
                                     const built_in_type *array, u32 array_length);

#define DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type)                                         \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_array_push_##name##_decoded(carbon_encoded_doc_t *doc, const char *key,                             \
                                     const built_in_type *array, u32 array_length);

DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, field_i8_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, field_i16_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, field_i32_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, field_i64_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, field_u8_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, field_u16_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, field_u32_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, field_u64_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(number, field_number_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, field_boolean_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(string, field_sid_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(null, field_u32_t)

DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, field_i8_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, field_i16_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, field_i32_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, field_i64_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, field_u8_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, field_u16_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, field_u32_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, field_u64_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, field_number_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, field_boolean_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, field_sid_t)
DEFINE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, field_u32_t)

//NG5_EXPORT(bool)
//carbon_encoded_doc_array_push_null(carbon_encoded_doc_t *doc, field_sid_t key, u32 how_many);

NG5_EXPORT(bool)
carbon_encoded_doc_array_push_object(carbon_encoded_doc_t *doc, field_sid_t key, carbon_object_id_t id);

NG5_EXPORT(bool)
carbon_encoded_doc_array_push_object_decoded(carbon_encoded_doc_t *doc, const char *key, carbon_object_id_t id);

NG5_EXPORT(bool)
carbon_encoded_doc_get_nested_object(carbon_encoded_doc_t *nested, carbon_object_id_t oid, carbon_encoded_doc_t *doc);

NG5_EXPORT(bool)
carbon_encoded_doc_print(FILE *file, carbon_encoded_doc_t *doc);


NG5_END_DECL

#endif
