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

#ifndef NG5_DOC_H
#define NG5_DOC_H

#include "shared/common.h"
#include "std/vec.h"
#include "stdx/strdic.h"
#include "json.h"

NG5_BEGIN_DECL

typedef struct carbon_doc_obj carbon_doc_obj_t;
typedef struct carbon_columndoc carbon_columndoc_t;

typedef struct carbon_doc_entries
{
    carbon_doc_obj_t                         *context;
    const char                               *key;
    field_e                       type;
    struct vector ofType(<T>)                  values;
} carbon_doc_entries_t;

typedef struct carbon_doc_bulk
{
    struct strdic                          *dic;
    struct vector ofType(char *)               keys,
                                              values;
    struct vector ofType(carbon_doc_t)         models;

} carbon_doc_bulk_t;

typedef struct carbon_doc
{
    carbon_doc_bulk_t                        *context;
    struct vector ofType(carbon_doc_obj_t)     obj_model;
    field_e                       type;
} carbon_doc_t;

typedef struct carbon_doc_obj
{
    struct vector ofType(carbon_doc_entries_t) entries;
    carbon_doc_t                             *doc;
} carbon_doc_obj_t;


NG5_EXPORT(bool)
carbon_doc_bulk_create(carbon_doc_bulk_t *bulk, struct strdic *dic);

NG5_EXPORT(bool)
carbon_doc_bulk_Drop(carbon_doc_bulk_t *bulk);

NG5_EXPORT(bool)
carbon_doc_bulk_shrink(carbon_doc_bulk_t *bulk);

NG5_EXPORT(bool)
carbon_doc_bulk_print(FILE *file, carbon_doc_bulk_t *bulk);

NG5_EXPORT(carbon_doc_t *)
carbon_doc_bulk_new_doc(carbon_doc_bulk_t *context, field_e type);

NG5_EXPORT(carbon_doc_obj_t *)
carbon_doc_bulk_new_obj(carbon_doc_t *model);

NG5_EXPORT(bool)
carbon_doc_bulk_get_dic_contents(struct vector ofType (const char *) **strings,
                                 struct vector ofType(carbon_string_id_t) **string_ids,
                                 const carbon_doc_bulk_t *context);

NG5_EXPORT(bool)
carbon_doc_print(FILE *file, const carbon_doc_t *doc);

NG5_EXPORT(const struct vector ofType(carbon_doc_entries_t) *)
carbon_doc_get_entries(const carbon_doc_obj_t *model);

NG5_EXPORT(void)
carbon_doc_print_entries(FILE *file, const carbon_doc_entries_t *entries);

NG5_EXPORT(void)
carbon_doc_drop(carbon_doc_obj_t *model);

NG5_EXPORT(bool)
carbon_doc_obj_add_key(carbon_doc_entries_t **out, carbon_doc_obj_t *obj, const char *key, field_e type);

NG5_EXPORT(bool)
carbon_doc_obj_push_primtive(carbon_doc_entries_t *entry, const void *value);

NG5_EXPORT(bool)
carbon_doc_obj_push_object(carbon_doc_obj_t **out, carbon_doc_entries_t *entry);

NG5_EXPORT(carbon_doc_entries_t *)
carbon_doc_bulk_new_entries(carbon_doc_bulk_t *dst);

NG5_EXPORT(carbon_doc_obj_t *)
carbon_doc_bulk_add_json(carbon_doc_entries_t *partition, carbon_json_t *json);

NG5_EXPORT(carbon_doc_obj_t *)
carbon_doc_entries_get_root(const carbon_doc_entries_t *partition);

NG5_EXPORT(carbon_columndoc_t *)
carbon_doc_entries_to_columndoc(const carbon_doc_bulk_t *bulk,
                                const carbon_doc_entries_t *partition, bool read_optimized);

NG5_EXPORT(bool)
carbon_doc_entries_drop(carbon_doc_entries_t *partition);

NG5_END_DECL

#endif