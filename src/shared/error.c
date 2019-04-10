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

#include "shared/error.h"

NG5_EXPORT(bool)
carbon_error_init(struct err *err)
{
    if (err) {
        err->code = NG5_ERR_NOERR;
        err->details = NULL;
        err->file = NULL;
        err->line = 0;
    }
    return (err != NULL);
}

NG5_EXPORT(bool)
carbon_error_cpy(struct err *dst, const struct err *src)
{
    NG5_NON_NULL_OR_ERROR(dst);
    NG5_NON_NULL_OR_ERROR(src);
    *dst = *src;
    return true;
}

NG5_EXPORT(bool)
carbon_error_drop(struct err *err)
{
    NG5_NON_NULL_OR_ERROR(err);
    if (err->details) {
        free(err->details);
        err->details = NULL;
    }
    return true;
}

NG5_EXPORT(bool)
carbon_error_set(struct err *err, int code, const char *file, u32 line)
{
    return carbon_error_set_wdetails(err, code, file, line, NULL);
}

NG5_EXPORT(bool)
carbon_error_set_wdetails(struct err *err, int code, const char *file, u32 line, const char *details)
{
    if (err) {
        err->code = code;
        err->file = file;
        err->line = line;
        err->details = details ? strdup(details) : NULL;
#ifndef NDEBUG
        carbon_error_print_and_abort(err);
#endif
    }
    return (err != NULL);
}

NG5_EXPORT(bool)
carbon_error_str(const char **errstr, const char **file, u32 *line, bool *details, const char **detailsstr,
                 const struct err *err)
{
    if (err) {
        if (err->code >= _carbon_nerr_str) {
            NG5_OPTIONAL_SET(errstr, NG5_ERRSTR_ILLEGAL_CODE)
        } else {
            NG5_OPTIONAL_SET(errstr, _carbon_err_str[err->code])
        }
        NG5_OPTIONAL_SET(file, err->file)
        NG5_OPTIONAL_SET(line, err->line)
        NG5_OPTIONAL_SET(details, err->details != NULL);
        NG5_OPTIONAL_SET(detailsstr, err->details)
        return true;
    }
    return false;
}

NG5_EXPORT(bool)
carbon_error_print(const struct err *err)
{
    if (err) {
        const char *errstr;
        const char *file;
        u32 line;
        bool has_details;
        const char *details;
        if (carbon_error_str(&errstr, &file, &line, &has_details, &details, err)) {
            fprintf(stderr, "*** ERROR ***   %s\n", errstr);
            fprintf(stderr, "                details: %s\n", has_details ? details : "no details");
            fprintf(stderr, "                source.: %s(%d)\n", file, line);
        } else {
            fprintf(stderr, "*** ERROR ***   internal error during error information fetch");
        }
        fflush(stderr);
    }
    return (err != NULL);
}

NG5_EXPORT(bool)
carbon_error_print_and_abort(const struct err *err)
{
    carbon_error_print(err);
    abort();
}