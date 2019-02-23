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

#include <carbon/carbon-int-archive.h>
#include <carbon/carbon-string-pred.h>
#include "carbon/carbon-query.h"


#define OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(num_pairs, obj, bit_flag_name, offset_name)                                \
{                                                                                                                      \
    if (!obj) {                                                                                                        \
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_NULLPTR)                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    if (obj->flags.bits.bit_flag_name) {                                                                               \
        assert(obj->props.offset_name != 0);                                                                           \
        carbon_memfile_seek(&obj->file, obj->props.offset_name);                                                       \
        carbon_fixed_prop_t prop;                                                                                      \
        carbon_int_embedded_fixed_props_read(&prop, &obj->file);                                                       \
        carbon_int_reset_cabin_object_mem_file(obj);                                                                   \
        CARBON_OPTIONAL_SET(num_pairs, prop.header->num_entries);                                                      \
        return prop.keys;                                                                                              \
    } else {                                                                                                           \
        CARBON_OPTIONAL_SET(num_pairs, 0);                                                                             \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}


CARBON_EXPORT(bool)
carbon_query_create(carbon_query_t *query, carbon_archive_t *archive)
{
    CARBON_NON_NULL_OR_ERROR(query)
    CARBON_NON_NULL_OR_ERROR(archive)
    query->archive = archive;
    query->context = carbon_archive_io_context_create(archive);
    carbon_error_init(&query->err);
    return query->context != NULL;
}

CARBON_EXPORT(bool)
carbon_query_drop(carbon_query_t *query)
{
    CARBON_NON_NULL_OR_ERROR(query)
    return carbon_io_context_drop(query->context);
}

CARBON_EXPORT(bool)
carbon_query_scan_strids(carbon_strid_iter_t *it, carbon_query_t *query)
{
    CARBON_NON_NULL_OR_ERROR(it)
    CARBON_NON_NULL_OR_ERROR(query)
    return carbon_strid_iter_open(it, &query->err, query->archive);
}


CARBON_EXPORT(char *)
carbon_query_fetch_string_by_id(carbon_query_t *query, carbon_string_id_t id)
{
    assert(query);

    carbon_strid_iter_t  strid_iter;
    carbon_strid_info_t *info;
    size_t               vector_len;
    bool                 status;
    bool                 success;

    status = carbon_query_scan_strids(&strid_iter, query);

    if (status) {
        while (carbon_strid_iter_next(&success, &info, &query->err, &vector_len, &strid_iter)) {
            for (size_t i = 0; i < vector_len; i++) {
                if (info[i].id == id) {
                    char *result = malloc(info[i].strlen + 1);
                    memset(result, 0, info[i].strlen + 1);

                    fseek(strid_iter.disk_file, info[i].offset, SEEK_SET);

                    bool decode_result = carbon_compressor_decode(&query->err, &query->archive->string_table.compressor,
                                                                  result, info[i].strlen, strid_iter.disk_file);

                    bool close_iter_result = carbon_strid_iter_close(&strid_iter);

                    if (!decode_result || !close_iter_result) {
                        free (result);
                        CARBON_ERROR(&query->err, !decode_result ? CARBON_ERR_DECOMPRESSFAILED :
                                                    CARBON_ERR_ITERATORNOTCLOSED);
                        return NULL;
                    } else {
                        return result;
                    }
                }
            }
        }
        CARBON_ERROR(&query->err, CARBON_ERR_NOTFOUND);
        return NULL;
    } else {
        CARBON_ERROR(&query->err, CARBON_ERR_SCAN_FAILED);
        return NULL;
    }
}

CARBON_EXPORT(char **)
carbon_query_fetch_strings_by_offset(carbon_query_t *query, carbon_off_t *offs, uint32_t *strlens, size_t num_offs)
{
    assert(query);
    assert(offs);
    assert(strlens);

    FILE *file;

    if (num_offs == 0)
    {
        return NULL;
    }

    char **result = malloc(num_offs * sizeof(char*));
    if (!result) {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        return NULL;
    }
    for (size_t i = 0; i < num_offs; i++)
    {
        if ((result[i] = malloc((strlens[i] + 1) * sizeof(char))) == NULL) {
            for (size_t k = 0; k < i; k++) {
                free(result[k]);
            }
            free(result);
            return NULL;
        }
        memset(result[i], 0, (strlens[i] + 1) * sizeof(char));
    }

    if (!result)
    {
        CARBON_ERROR(carbon_io_context_get_error(query->context), CARBON_ERR_MALLOCERR);
        return NULL;
    } else {
        if (!(file = carbon_io_context_lock_and_access(query->context)))
        {
            carbon_error_cpy(&query->err, carbon_io_context_get_error(query->context));
            goto cleanup_and_error;
        }

        for (size_t i = 0; i < num_offs; i++)
        {
            fseek(file, offs[i], SEEK_SET);
            if (!carbon_compressor_decode(&query->err, &query->archive->string_table.compressor,
                                          result[i], strlens[i], file))
            {
                carbon_io_context_unlock(query->context);
                goto cleanup_and_error;
            }
        }
        carbon_io_context_unlock(query->context);
        return result;
    }

cleanup_and_error:
    for (size_t i = 0; i < num_offs; i++) {
        free(result[i]);
    }
    free(result);
    return NULL;
}

CARBON_EXPORT(carbon_string_id_t *)
carbon_query_find_ids(size_t *num_found, carbon_query_t *query, const carbon_string_pred_t *pred,
                      void *capture, int64_t limit)
{
    if (CARBON_UNLIKELY(carbon_string_pred_validate(&query->err, pred) == false)) {
        return NULL;
    }
    int64_t              pred_limit;
    carbon_string_pred_get_limit(&pred_limit, pred);
    pred_limit = pred_limit < 0 ? limit : CARBON_MIN(pred_limit, limit);

    carbon_strid_iter_t  it;
    carbon_strid_info_t *info              = NULL;
    size_t               info_len          = 0;
    size_t               step_len          = 0;
    carbon_off_t        *str_offs          = NULL;
    uint32_t            *str_lens          = NULL;
    size_t              *idxs_matching     = NULL;
    size_t               num_matching      = 0;
    void                *tmp               = NULL;
    size_t               str_cap           = 1024;
    carbon_string_id_t  *step_ids          = NULL;
    carbon_string_id_t  *result_ids        = NULL;
    size_t               result_len        = 0;
    size_t               result_cap        = pred_limit < 0 ? str_cap : (size_t) pred_limit;
    bool                 success           = false;

    if (CARBON_UNLIKELY(pred_limit == 0))
    {
        *num_found = 0;
        return NULL;
    }

    if (CARBON_UNLIKELY(!num_found || !query || !pred))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_NULLPTR);
        return NULL;
    }

    if (CARBON_UNLIKELY((step_ids = malloc(str_cap * sizeof(carbon_string_id_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        return NULL;
    }

    if (CARBON_UNLIKELY((str_offs = malloc(str_cap * sizeof(carbon_off_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        goto cleanup_result_and_error;
        return NULL;
    }

    if (CARBON_UNLIKELY((str_lens = malloc(str_cap * sizeof(uint32_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        free(str_offs);
        goto cleanup_result_and_error;
        return NULL;
    }

    if (CARBON_UNLIKELY((idxs_matching = malloc(str_cap * sizeof(size_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        free(str_offs);
        free(str_lens);
        goto cleanup_result_and_error;
        return NULL;
    }

    if (CARBON_UNLIKELY(carbon_query_scan_strids(&it, query) == false))
    {
        free(str_offs);
        free(str_lens);
        free(idxs_matching);
        goto cleanup_result_and_error;
    }

    if (CARBON_UNLIKELY((result_ids = malloc(result_cap * sizeof(carbon_string_id_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        free(str_offs);
        free(str_lens);
        free(idxs_matching);
        carbon_strid_iter_close(&it);
        goto cleanup_result_and_error;
        return NULL;
    }

    while (carbon_strid_iter_next(&success, &info, &query->err, &info_len, &it))
    {
        if (CARBON_UNLIKELY(info_len > str_cap))
        {
            str_cap = (info_len + 1) * 1.7f;
            if (CARBON_UNLIKELY((tmp = realloc(str_offs, str_cap * sizeof(carbon_off_t))) == NULL))
            {
                goto realloc_error;
            } else {
                str_offs = tmp;
            }
            if (CARBON_UNLIKELY((tmp = realloc(str_lens, str_cap * sizeof(uint32_t))) == NULL))
            {
                goto realloc_error;
            } else {
                str_lens = tmp;
            }
            if (CARBON_UNLIKELY((tmp = realloc(idxs_matching, str_cap * sizeof(size_t))) == NULL))
            {
                goto realloc_error;
            } else {
                idxs_matching = tmp;
            }
        }
        assert(info_len <= str_cap);
        for (step_len = 0; step_len < info_len; step_len++)
        {
            assert(step_len < str_cap);
            str_offs[step_len] = info[step_len].offset;
            str_lens[step_len] = info[step_len].strlen;
        }

        char **strings = carbon_query_fetch_strings_by_offset(query, str_offs, str_lens, step_len); // TODO: buffer + cleanup buffer

        if (CARBON_UNLIKELY(carbon_string_pred_eval(pred, idxs_matching, &num_matching,
                                                    strings, step_len, capture) == false))
        {
            CARBON_ERROR(&query->err, CARBON_ERR_PREDEVAL_FAILED);
            carbon_strid_iter_close(&it);
            goto cleanup_intermediate;
        }

        for (size_t i = 0; i < step_len; i++) {
            free(strings[i]);
        }
        free(strings);

        for (size_t i = 0; i < num_matching; i++)
        {
            assert (idxs_matching[i] < info_len);
            result_ids[result_len++] =  info[idxs_matching[i]].id;
            if (pred_limit > 0 && result_len == (size_t) pred_limit) {
                goto stop_search_and_return;
            }
            if (CARBON_UNLIKELY(result_len > result_cap))
            {
                result_cap = (result_len + 1) * 1.7f;
                if (CARBON_UNLIKELY((tmp = realloc(result_ids, result_cap * sizeof(carbon_string_id_t))) == NULL))
                {
                    carbon_strid_iter_close(&it);
                    goto cleanup_intermediate;
                } else {
                    result_ids = tmp;
                }
            }
        }
    }

stop_search_and_return:
    if (CARBON_UNLIKELY(success == false)) {
        carbon_strid_iter_close(&it);
        goto cleanup_intermediate;
    }

    *num_found = result_len;
    return result_ids;

realloc_error:
    CARBON_ERROR(&query->err, CARBON_ERR_REALLOCERR);

cleanup_intermediate:
    free(str_offs);
    free(str_lens);
    free(idxs_matching);
    free(result_ids);

cleanup_result_and_error:
    free (step_ids);
    return NULL;
}