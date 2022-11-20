/* It at least one source file do this:

    #define W_JSON_IMPLEMENTATION
    #include "wjson.h"

If putc_unlocked is giving you trouble (non POSIX systems), then also add:

    #define W_JSON_NO_STDIO_UNLOCK

Created By: Justin Meiners (2022). MIT License (see end).
*/

#ifndef W_JSON_INCLUDE
#define W_JSON_INCLUDE

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>

#ifndef W_JSON_DEPTH_MAX
#define W_JSON_DEPTH_MAX 128
#endif

typedef struct {
    FILE* file;
    int depth;
    int stack[W_JSON_DEPTH_MAX];
    int count[W_JSON_DEPTH_MAX];
} wJson;

void wJson_init(wJson* writer, FILE* file);
void wJson_shutdown(wJson* writer);

void wJson_key(wJson* writer, const char* key);
// Write key without escaping. 
void wJson_key_raw(wJson* w, const char* key);

void wJson_string(wJson* writer, const char* str);
// Write string without escaping. 
void wJson_string_raw(wJson* writer, const char* str);

void wJson_int(wJson* writer, int x);
void wJson_double(wJson* writer, double x);
void wJson_null(wJson* writer);

void wJson_object(wJson* writer);
void wJson_array(wJson* writer);
// end the current object or array.
void wJson_end(wJson* writer);

// Helpers for writing arrays and other common structures.
void wJson_string_pair(wJson* writer, const char* key, const char* val);

void wJson_doublev(wJson* writer, int n, const double* v);
void wJson_intv(wJson* writer, int n, const int* v);
void wJson_stringv(wJson* writer, int n, const char** strings);
void wJson_string_pairv(wJson* writer, int n, const char** keys, const char** vals);

#ifdef __cplusplus
}
#endif

#endif

#ifdef W_JSON_IMPLEMENTATION

#ifdef W_JSON_NO_STDIO_UNLOCK
#define putc_unlocked putc
#endif

#include <assert.h>
#include <ctype.h>

static
void write_string_escaped_(const char* s, FILE* file) {
    while (1) {
        switch (*s) {
            case '\0':
                return;
            case '\"':
                fputs("\\\"", file);
                break;
            case '\'':
                fputs("\\\'", file);
                break;
            case '\\':
                fputs("\\\\", file);
                break;
            case '\a':
                fputs("\\a", file);
                break;
            case '\b':
                fputs("\\b", file);
                break;
            case '\n':
                fputs("\\n", file);
                break;
            case '\t':
                fputs("\\t", file);
                break;
            default:
                if (iscntrl(*s)) fprintf(file, "\\%03o", *s);
                else putc_unlocked(*s, file);
        }
        ++s;
    }
}

enum { W_JSON_BLOCK_OBJ, W_JSON_BLOCK_ARRAY };

static
void write_separator(wJson* w) {
    int entries = w->count[w->depth];
    if (entries != 0) {
        putc_unlocked(',', w->file);
    }
}

static
void begin_value_(wJson* w) {
    if (w->stack[w->depth] == W_JSON_BLOCK_ARRAY) {
        write_separator(w);
    } else {
        assert(w->count[w->depth] % 1 == 0);
    }
    ++w->count[w->depth];
}

static
void begin_key_(wJson* w) {
    assert(w->stack[w->depth] == W_JSON_BLOCK_OBJ);
    assert(w->count[w->depth] % 2 == 0);
    write_separator(w);
    ++w->count[w->depth];
}

void wJson_init(wJson* w, FILE* file) {
    w->file = file;
    w->depth = 0;
    w->stack[0] = W_JSON_BLOCK_OBJ;
}

void wJson_shutdown(wJson* w) { assert(w->depth == 0); }

void wJson_key_raw(wJson* w, const char* key) {
    begin_key_(w);
    fprintf(w->file, "\"%s\":", key);
}

void wJson_key(wJson* w, const char* str) {
    begin_key_(w);
    putc_unlocked('\"', w->file);
    write_string_escaped_(str, w->file);
    putc_unlocked('\"', w->file);
    putc_unlocked(':', w->file);
}

void wJson_string_raw(wJson* w, const char* str) {
    begin_value_(w);
    fprintf(w->file, "\"%s\"", str);
}

void wJson_string(wJson* w, const char* str) {
    begin_value_(w);
    putc_unlocked('\"', w->file);
    write_string_escaped_(str, w->file);
    putc_unlocked('\"', w->file);
}

void wJson_int(wJson* w, int x) {
    begin_value_(w);
    fprintf(w->file, "%d", x);
}
void wJson_double(wJson* w, double x) {
    begin_value_(w);
    fprintf(w->file, "%f", x);
}

void wJson_null(wJson* w) {
    begin_value_(w);
    fprintf(w->file, "null");
}

void wJson_object(wJson* w) {
    begin_value_(w);
    putc_unlocked('{', w->file);
    ++w->depth;
    assert(w->depth < W_JSON_DEPTH_MAX);
    w->stack[w->depth] = W_JSON_BLOCK_OBJ;
    w->count[w->depth] = 0;
}

void wJson_array(wJson* w) {
    begin_value_(w);
    putc_unlocked('[', w->file);
    ++w->depth;
    assert(w->depth < W_JSON_DEPTH_MAX);
    w->stack[w->depth] = W_JSON_BLOCK_ARRAY;
    w->count[w->depth] = 0;
}

void wJson_end(wJson* w) {
    if (w->stack[w->depth] == W_JSON_BLOCK_OBJ) {
        assert(w->count[w->depth] % 2 == 0);
        putc_unlocked('}', w->file);
    } else {
        putc_unlocked(']', w->file);
    }
    --w->depth;
    assert(w->depth >= 0);
}

void wJson_string_pair(wJson* w, const char* key, const char* val) {
    wJson_key(w, key);
    wJson_string(w, val);
}

void wJson_doublev(wJson* w, int n, const double* v) {
    wJson_array(w);
    for (int i = 0; i < n; ++i) wJson_double(w, v[i]);
    wJson_end(w);
}

void wJson_intv(wJson* w, int n, const int* v) {
    wJson_array(w);
    for (int i = 0; i < n; ++i) wJson_int(w, v[i]);
    wJson_end(w);
}

void wJson_stringv(wJson* w, int n, const char** strings) {
    wJson_array(w);
    for (int i = 0; i < n; ++i) wJson_string(w, strings[i]);
    wJson_end(w);
}

void wJson_string_pairv(wJson* w, int n, const char** keys, const char** vals) {
    wJson_object(w);
    for (int i = 0; i < n; ++i) wJson_string_pair(w, keys[i], vals[i]);
    wJson_object(w);
}

#ifdef W_JSON_NO_STDIO_UNLOCK
#undef putc_unlocked putc
#endif

#endif

/*
   Copyright 2022 Justin Meiners
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

