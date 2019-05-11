#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include <stdbool.h>
#include <stddef.h>

typedef struct String String;
struct CopyInterface;
struct HashInterface;

const struct CopyInterface *STRING_COPY_OPS;
const struct HashInterface *STRING_HASH_OPS;

// Returns a pointer to a newly-allocated, empty string
String *new_string(void);

// Returns a pointer to a newly-allocated string with the given content
String *string_from_chars(const char *chars);

// Returns a copy of the given string
String *copy_string(const String *str);

// Frees the memory associated with a string
void free_string(String *str);

// Adds a character to the end of the string
void string_add_char(String *str, char c);

// Adds some characters to the end of the string
void string_add_chars(String *str, const char *chars);

// Adds the content of the string to the end of another string
void string_add_str(String *str1, const String *str2);

// Returns a hash for the string
size_t hash_string(const String *str);

// Returns whether two strings are equal to each other
bool strings_equal(const String *str1, const String *str2);

// Returns the length of the string
size_t string_len(const String *str);

// Returns a pointer to the string's content
const char *string_data(const String *str);

// Returns a pointer to a null-terminated string from the string's content
const char *string_get_c_str(String *str);

// Returns the character at a given index in the string
char string_get(const String *str, size_t index);

#endif
