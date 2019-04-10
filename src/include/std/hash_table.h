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

#ifndef NG5_FIX_MAP_H
#define NG5_FIX_MAP_H

#include "shared/common.h"
#include "std/vec.h"
#include "core/async/spin.h"

NG5_BEGIN_DECL

typedef struct
{
    bool     in_use_flag;  /* flag indicating if bucket is in use */
    i32  displacement; /* difference between intended position during insert, and actual position in table */
    u32 num_probs;    /* number of probe calls to this bucket */
    u64 data_idx;      /* position of key element in owning carbon_hashtable_t structure */
} carbon_hashtable_bucket_t;

/**
 * Hash table implementation specialized for key and value types of fixed-length size, and where comparision
 * for equals is byte-compare. With this, calling a (type-dependent) compare function becomes obsolete.
 *
 * Example: mapping of u64 to u32.
 *
 * This hash table is optimized to reduce access time to elements. Internally, a robin-hood hashing technique is used.
 *
 * Note: this implementation does not support string or pointer types. The structure is thread-safe by a spinlock
 * lock implementation.
 */
typedef struct
{
    struct vector key_data;
    struct vector value_data;
    struct vector ofType(carbon_hashtable_bucket_t) table;
    struct spinlock lock;
    u32 size;
    struct err err;
} carbon_hashtable_t;

NG5_DEFINE_GET_ERROR_FUNCTION(hashtable, carbon_hashtable_t, table);

NG5_EXPORT(bool)
carbon_hashtable_create(carbon_hashtable_t *map, struct err *err, size_t key_size, size_t value_size, size_t capacity);

NG5_EXPORT(carbon_hashtable_t *)
carbon_hashtable_cpy(carbon_hashtable_t *src);

NG5_EXPORT(bool)
carbon_hashtable_drop(carbon_hashtable_t *map);

NG5_EXPORT(bool)
carbon_hashtable_clear(carbon_hashtable_t *map);

NG5_EXPORT(bool)
carbon_hashtable_avg_displace(float *displace, const carbon_hashtable_t *map);

NG5_EXPORT(bool)
carbon_hashtable_lock(carbon_hashtable_t *map);

NG5_EXPORT(bool)
carbon_hashtable_unlock(carbon_hashtable_t *map);

NG5_EXPORT(bool)
carbon_hashtable_insert_or_update(carbon_hashtable_t *map, const void *keys, const void *values, uint_fast32_t num_pairs);

NG5_EXPORT(bool)
carbon_hashtable_serialize(FILE *file, carbon_hashtable_t *table);

NG5_EXPORT(bool)
carbon_hashtable_deserialize(carbon_hashtable_t *table, struct err *err, FILE *file);

NG5_EXPORT(bool)
carbon_hashtable_remove_if_contained(carbon_hashtable_t *map, const void *keys, size_t num_pairs);

NG5_EXPORT(const void *)
carbon_hashtable_get_value(carbon_hashtable_t *map, const void *key);

NG5_EXPORT(bool)
carbon_hashtable_get_fload_factor(float *factor, carbon_hashtable_t *map);

NG5_EXPORT(bool)
carbon_hashtable_rehash(carbon_hashtable_t *map);

NG5_END_DECL

#endif
