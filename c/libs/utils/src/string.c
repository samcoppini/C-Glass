#include "utils/string.h"
#include "utils/copy-interface.h"

#include <stdlib.h>
#include <string.h>

struct String {
    char *buf;

    size_t len;

    size_t alloc;
};

#define STR_INIT_ALLOC 16

String *new_string(void) {
    String *str = malloc(sizeof(String));
    str->buf = malloc(sizeof(char) * STR_INIT_ALLOC);
    str->len = 0;
    str->alloc = STR_INIT_ALLOC;
    return str;
}

String *string_from_chars(const char *chars) {
    String *str = malloc(sizeof(String));
    str->len = strlen(chars);
    str->alloc = STR_INIT_ALLOC;
    while (str->len > str->alloc) {
        str->alloc *= 2;
    }
    str->buf = malloc(sizeof(char) * str->alloc);
    memcpy(str->buf, chars, str->len);
    return str;
}

String *copy_string(const String *str) {
    String *copy = malloc(sizeof(String));
    copy->buf = malloc(sizeof(char) * str->alloc);
    copy->len = str->len;
    copy->alloc = str->alloc;
    memcpy(copy->buf, str->buf, copy->alloc);
    return copy;
}

void free_string(String *str) {
    free(str->buf);
    free(str);
}

static void string_reserve_space(String *str, size_t len) {
    if (str->alloc < len) {
        do {
            str->alloc *= 2;
        } while (str->alloc < len);

        str->buf = realloc(str->buf, sizeof(char) * str->alloc);
    }
}

void string_add_char(String *str, char c) {
    string_reserve_space(str, str->len + 1);
    str->buf[str->len] = c;
    str->len++;
}

void string_add_chars(String *str, const char *chars) {
    size_t chars_len = strlen(chars);
    string_reserve_space(str, str->len + chars_len);
    memcpy(str->buf + str->len, chars, chars_len);
    str->len += chars_len;
}

void string_add_str(String *str1, const String *str2) {
    string_reserve_space(str1, str1->len + str2->len);
    memcpy(str1->buf + str1->len, str2->buf, str2->len);
    str1->len += str2->len;
}

size_t hash_string(const String *str) {
    size_t hash = 5381;
    for (size_t i = 0; i < str->len; i++) {
        hash = (hash * 33) + str->buf[i];
    }
    return hash;
}

bool strings_equal(const String *str1, const String *str2) {
    if (str1->len != str2->len) {
        return false;
    }
    return memcmp(str1->buf, str2->buf, str1->len) == 0;
}

size_t string_len(const String *str) {
    return str->len;
}

const char *string_data(const String *str) {
    return str->buf;
}

const char *string_get_c_str(String *str) {
    string_reserve_space(str, str->len + 1);
    str->buf[str->len] = '\0';
    return str->buf;
}

char string_get(const String *str, size_t index) {
    return str->buf[index];
}

// "Generic" versions of some functions that are used to satisfy interfaces

static void *copy_string_generic(const void *str) {
    return copy_string(str);
}

static void free_string_generic(void *str) {
    free_string(str);
}

static size_t hash_string_generic(const void *str) {

}

static bool strings_equal_generic(const void *str1, const void *str2) {

}

const CopyInterface *STRING_COPY_OPS = & (CopyInterface) {
    copy_string_generic,
    free_string_generic,
};
