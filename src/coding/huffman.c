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

#include <limits.h>
#include <assert.h>
#include <inttypes.h>

#include "coding/pack_huffman.h"
#include "std/bitmap.h"

struct huff_node
{
    struct huff_node *prev, *next, *left, *right;
    u64 freq;
    unsigned char letter;
};

static void huff_tree_create(struct vector ofType(carbon_huffman_entry_t) *table, const struct vector ofType(u32) *frequencies);

bool carbon_huffman_create(carbon_huffman_t *dic)
{
    NG5_NON_NULL_OR_ERROR(dic);

    carbon_vec_create(&dic->table, NULL, sizeof(carbon_huffman_entry_t), UCHAR_MAX / 4);
    carbon_error_init(&dic->err);

    return true;
}

NG5_EXPORT(bool)
carbon_huffman_cpy(carbon_huffman_t *dst, carbon_huffman_t *src)
{
    NG5_NON_NULL_OR_ERROR(dst);
    NG5_NON_NULL_OR_ERROR(src);
    if (!carbon_vec_cpy(&dst->table, &src->table)) {
        error(&src->err, NG5_ERR_HARDCOPYFAILED);
        return false;
    } else {
        return carbon_error_cpy(&dst->err, &src->err);
    }
}

NG5_EXPORT(bool)
carbon_huffman_build(carbon_huffman_t *encoder, const string_vector_t *strings)
{
    NG5_NON_NULL_OR_ERROR(encoder);
    NG5_NON_NULL_OR_ERROR(strings);

    struct vector ofType(u32) frequencies;
    carbon_vec_create(&frequencies, NULL, sizeof(u32), UCHAR_MAX);
    carbon_vec_enlarge_size_to_capacity(&frequencies);

    u32 *freq_data = vec_all(&frequencies, u32);
    NG5_ZERO_MEMORY(freq_data, UCHAR_MAX * sizeof(u32));

    for (size_t i = 0; i < strings->num_elems; i++) {
        const char *string = *vec_get(strings, i, const char *);
        size_t string_length = strlen(string);
        for (size_t k = 0; k < string_length; k++) {
            size_t c = (unsigned char) string[k];
            freq_data[c]++;
        }
    }

    huff_tree_create(&encoder->table, &frequencies);
    carbon_vec_drop(&frequencies);

    return true;
}

NG5_EXPORT(bool)
carbon_huffman_get_error(struct err *err, const carbon_huffman_t *dic)
{
    NG5_NON_NULL_OR_ERROR(err)
    NG5_NON_NULL_OR_ERROR(dic)
    carbon_error_cpy(err, &dic->err);
    return true;
}

bool carbon_huffman_drop(carbon_huffman_t *dic)
{
    NG5_NON_NULL_OR_ERROR(dic);

    for (size_t i = 0; i < dic->table.num_elems; i++) {
        carbon_huffman_entry_t *entry = vec_get(&dic->table, i, carbon_huffman_entry_t);
        free(entry->blocks);
    }

    carbon_vec_drop(&dic->table);

    free(dic);

    return true;
}

bool carbon_huffman_serialize_dic(struct memfile *file, const carbon_huffman_t *dic, char marker_symbol)
{
    NG5_NON_NULL_OR_ERROR(file)
    NG5_NON_NULL_OR_ERROR(dic)

    for (size_t i = 0; i < dic->table.num_elems; i++) {
        carbon_huffman_entry_t *entry = vec_get(&dic->table, i, carbon_huffman_entry_t);
            memfile_write(file, &marker_symbol, sizeof(char));
            memfile_write(file, &entry->letter, sizeof(unsigned char));

        /** block one is the block that holds the significant part of the prefix code */
        offset_t offset_meta, offset_continue;
        carbon_memfile_tell(&offset_meta, file);
        /** this will be the number of bytes used to encode the significant part of the prefix code */
        carbon_memfile_skip(file, sizeof(u8));

        carbon_memfile_begin_bit_mode(file);
        bool first_bit_found = false;
        for (int i = 31; entry->blocks && i >= 0; i--) {
            u32 mask = 1 << i;
            u32 k = entry->blocks[0] & mask;
            bool bit_state = k != 0;
            first_bit_found |= bit_state;

            if (first_bit_found) {
                memfile_write_bit(file, bit_state);
            }
        }
        size_t num_bytes_written;
        carbon_memfile_end_bit_mode(&num_bytes_written, file);
        carbon_memfile_tell(&offset_continue, file);
        carbon_memfile_seek(file, offset_meta);
        u8 num_bytes_written_uint8 = (u8) num_bytes_written;
            memfile_write(file, &num_bytes_written_uint8, sizeof(u8));

        carbon_memfile_seek(file, offset_continue);
    }

    return true;
}

static carbon_huffman_entry_t *find_dic_entry(carbon_huffman_t *dic, unsigned char c)
{
    for (size_t i = 0; i < dic->table.num_elems; i++) {
        carbon_huffman_entry_t *entry = vec_get(&dic->table, i, carbon_huffman_entry_t);
        if (entry->letter == c) {
            return entry;
        }
    }
    error(&dic->err, NG5_ERR_HUFFERR)
    return NULL;
}

static size_t encodeString(struct memfile *file, carbon_huffman_t *dic, const char *string)
{
    carbon_memfile_begin_bit_mode(file);

    for (const char *c = string; *c != '\0'; c++) {
        carbon_huffman_entry_t *entry = find_dic_entry(dic, (unsigned char) *c);
        if (!entry) {
            return 0;
        }

        if (!entry->blocks) {
            memfile_write_bit(file, false);
        } else {
            for (size_t j = 0; j < entry->nblocks; j++) {
                u32 block = entry->blocks[j];

                bool first_bit_found = false;
                for (int i = 31; i >= 0; i--) {
                    u32 mask = 1 << i;
                    u32 k = block & mask;
                    bool bit_state = k != 0;
                    first_bit_found |= bit_state;

                    if (first_bit_found) {
                        memfile_write_bit(file, bit_state);
                    }
                }
            }
        }
    }

    size_t num_written_bytes;
    carbon_memfile_end_bit_mode(&num_written_bytes, file);
    return num_written_bytes;
}

NG5_EXPORT(bool)
carbon_huffman_encode_one(struct memfile *file,
                          carbon_huffman_t *dic,
                          const char *string)
{
    NG5_NON_NULL_OR_ERROR(file)
    NG5_NON_NULL_OR_ERROR(dic)
    NG5_NON_NULL_OR_ERROR(string)

    u32 num_bytes_encoded = 0;

    offset_t num_bytes_encoded_off = memfile_tell(file);
    carbon_memfile_skip(file, sizeof(u32));

    if ((num_bytes_encoded = (u32) encodeString(file, dic, string)) == 0) {
        return false;
    }

    offset_t continue_off = memfile_tell(file);
    carbon_memfile_seek(file, num_bytes_encoded_off);
        memfile_write(file, &num_bytes_encoded, sizeof(u32));
    carbon_memfile_seek(file, continue_off);

    return true;
}

bool carbon_huffman_read_string(carbon_huffman_encoded_str_info_t *info, struct memfile *src)
{
    info->nbytes_encoded = *NG5_MEMFILE_READ_TYPE(src, u32);
    info->encoded_bytes = NG5_MEMFILE_READ(src, info->nbytes_encoded);
    return true;
}

bool carbon_huffman_read_dic_entry(carbon_huffman_entry_info_t *info, struct memfile *file, char marker_symbol)
{
    char marker = *NG5_MEMFILE_PEEK(file, char);
    if (marker == marker_symbol) {
        carbon_memfile_skip(file, sizeof(char));
        info->letter = *NG5_MEMFILE_READ_TYPE(file, unsigned char);
        info->nbytes_prefix = *NG5_MEMFILE_READ_TYPE(file, u8);
        info->prefix_code = NG5_MEMFILE_PEEK(file, char);

        carbon_memfile_skip(file, info->nbytes_prefix);

        return true;
    } else {
        return false;
    }
}

static const u32 *get_num_used_blocks(u16 *numUsedBlocks, carbon_huffman_entry_t *entry, u16 num_blocks,
                                        const u32 *blocks)
{
    for (entry->nblocks = 0; entry->nblocks < num_blocks; entry->nblocks++) {
        const u32 *block = blocks + entry->nblocks;
        if (*block != 0) {
            *numUsedBlocks = (num_blocks - entry->nblocks);
            return block;
        }
    }
    return NULL;
}

static void import_into_entry(carbon_huffman_entry_t *entry, const struct huff_node *node, const carbon_bitmap_t *carbon_bitmap_t)
{
    entry->letter = node->letter;
    u32 *blocks, num_blocks;
    const u32 *used_blocks;
    carbon_bitmap_blocks(&blocks, &num_blocks, carbon_bitmap_t);
    used_blocks = get_num_used_blocks(&entry->nblocks, entry, num_blocks, blocks);
    entry->blocks = malloc(entry->nblocks * sizeof(u32));
    if (num_blocks > 0) {
        memcpy(entry->blocks, used_blocks, entry->nblocks * sizeof(u32));
    } else {
        entry->blocks = NULL;
    }
    free(blocks);
}

static struct huff_node *seek_to_begin(struct huff_node *handle) {
    for (; handle->prev != NULL; handle = handle->prev)
        ;
    return handle;
}

static struct huff_node *seek_to_end(struct huff_node *handle) {
    for (; handle->next != NULL; handle = handle->next)
        ;
    return handle;
}

NG5_FUNC_UNUSED
static void __diag_print_insight(struct huff_node *n)
{
    printf("(");
    if (!n->left && !n->right) {
        printf("%c", n->letter);
    } else {
        if (n->left) {
            __diag_print_insight(n->left);
        }
        printf(",");
        if (n->right) {
            __diag_print_insight(n->right);
        }
    }
    printf(")");
    printf(": %"PRIu64"",n->freq);
}

NG5_FUNC_UNUSED
static void __diag_dump_remaining_candidates(struct huff_node *n)
{
    struct huff_node *it = seek_to_begin(n);
    while (it->next != NULL) {
        __diag_print_insight(it);
        printf(" | ");
        it = it->next;
    }
}

static struct huff_node *find_smallest(struct huff_node *begin, u64 lowerBound, struct huff_node *skip)
{
    u64 smallest = UINT64_MAX;
    struct huff_node *result = NULL;
    for (struct huff_node *it = begin; it != NULL; it = it->next) {
        if (it != skip && it->freq >= lowerBound && it->freq <= smallest) {
            smallest = it->freq;
            result = it;
        }
    }
    return result;
}


static void assign_code(struct huff_node *node, const carbon_bitmap_t *path, struct vector ofType(carbon_huffman_entry_t) *table)
{
    if (!node->left && !node->right) {
            carbon_huffman_entry_t *entry = VECTOR_NEW_AND_GET(table, carbon_huffman_entry_t);
            import_into_entry(entry, node, path);
    } else {
        if (node->left) {
            carbon_bitmap_t left;
            carbon_bitmap_cpy(&left, path);
            carbon_bitmap_lshift(&left);
            carbon_bitmap_set(&left, 0, false);
            assign_code(node->left, &left, table);
            carbon_bitmap_drop(&left);
        }
        if (node->right) {
            carbon_bitmap_t right;
            carbon_bitmap_cpy(&right, path);
            carbon_bitmap_lshift(&right);
            carbon_bitmap_set(&right, 0, true);
            assign_code(node->right, &right, table);
            carbon_bitmap_drop(&right);
        }
    }
}

static struct huff_node *trim_and_begin(struct vector ofType(HuffNode) *candidates)
{
    struct huff_node *begin = NULL;
    for (struct huff_node *it = vec_get(candidates, 0, struct huff_node); ; it++) {
        if (it->freq == 0) {
            if (it->prev) {
                it->prev->next = it->next;
            }
            if (it->next) {
                it->next->prev = it->prev;
            }
        } else {
            if (!begin) {
                begin = it;
            }
        }
        if (!it->next) {
            break;
        }
    }
    return begin;
}

static void huff_tree_create(struct vector ofType(carbon_huffman_entry_t) *table, const struct vector ofType(u32) *frequencies)
{
    assert(UCHAR_MAX == frequencies->num_elems);

    struct vector ofType(HuffNode) candidates;
    carbon_vec_create(&candidates, NULL, sizeof(struct huff_node), UCHAR_MAX * UCHAR_MAX);
    size_t appender_idx = UCHAR_MAX;

    for (unsigned char i = 0; i < UCHAR_MAX; i++) {
        struct huff_node *node = VECTOR_NEW_AND_GET(&candidates, struct huff_node);
        node->letter = i;
        node->freq = *vec_get(frequencies, i, u32);
    }

    for (unsigned char i = 0; i < UCHAR_MAX; i++) {
        struct huff_node *node = vec_get(&candidates, i, struct huff_node);
        struct huff_node *prev = i > 0 ? vec_get(&candidates, i - 1, struct huff_node) : NULL;
        struct huff_node *next = i + 1 < UCHAR_MAX ? vec_get(&candidates, i + 1, struct huff_node) : NULL;
        node->next = next;
        node->prev = prev;
        node->left = node->right = NULL;
    }


    struct huff_node *smallest, *small;
    struct huff_node *handle = trim_and_begin(&candidates);
    struct huff_node *new_node = NULL;


    while (handle->next != NULL) {
        smallest = find_smallest(handle, 0, NULL);
        small = find_smallest(handle, smallest->freq, smallest);

        appender_idx++;
        new_node = VECTOR_NEW_AND_GET(&candidates, struct huff_node);
        new_node->freq = small->freq + smallest->freq;
        new_node->letter = '\0';
        new_node->left = small;
        new_node->right = smallest;

        if((small->prev == NULL && smallest->next == NULL) && small->next == smallest) {
            break;
        }

        if((smallest->prev == NULL && small->next == NULL) && smallest->next == small) {
            break;
        }
        if (smallest->prev) {
            smallest->prev->next = smallest->next;
        }
        if (smallest->next) {
            smallest->next->prev = smallest->prev;
        }
        if (small->prev) {
            small->prev->next = small->next;
        }
        if (small->next) {
            small->next->prev = small->prev;
        }

        if (small->prev) {
            handle = seek_to_begin(small->prev);
        } else if (smallest->prev) {
            handle = seek_to_begin(smallest->prev);
        } else if (small->next) {
            handle = seek_to_begin(small->next);
        } else if (smallest->next) {
            handle = seek_to_begin(smallest->next);
        } else {
            carbon_print_error_and_die(NG5_ERR_INTERNALERR);
        }

        assert (!handle->prev);
        struct huff_node *end = seek_to_end(handle);
        assert(!end->next);
        end->next = new_node;
        new_node->prev = end;
        new_node->next = NULL;

#ifdef DIAG_HUFFMAN_ENABLE_DEBUG
        printf("in-memory huff-tree: ");
        __diag_print_insight(new_node);
        printf("\n");
        printf("remaining candidates: ");
        __diag_dump_remaining_candidates(handle);
        printf("\n");
#endif
    }

    seek_to_begin(handle);
    if(handle->next) {
        struct huff_node *finalNode = VECTOR_NEW_AND_GET(&candidates, struct huff_node);
        finalNode->freq = small->freq + smallest->freq;
        finalNode->letter = '\0';
        if (handle->freq > handle->next->freq) {
            finalNode->left = handle;
            finalNode->right = handle->next;
        } else {
            finalNode->left = handle->next;
            finalNode->right = handle;
        }
        new_node = finalNode;
    }

#ifdef DIAG_HUFFMAN_ENABLE_DEBUG
    printf("final in-memory huff-tree: ");
    __diag_print_insight(new_node);
    printf("\n");
#endif

    carbon_bitmap_t root_path;
    carbon_bitmap_create(&root_path, UCHAR_MAX);
    carbon_bitmap_set(&root_path, 0, true);
    assign_code(new_node, &root_path, table);
    carbon_bitmap_drop(&root_path);

    carbon_vec_drop(&candidates);
}