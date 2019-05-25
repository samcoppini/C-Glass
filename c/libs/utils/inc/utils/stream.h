#ifndef UTILS_STREAM_H
#define UTILS_STREAM_H

#include <stdbool.h>
#include <stdio.h>

typedef struct Stream Stream;
struct String;

Stream *stream_from_string(const struct String *str);

Stream *stream_from_file(FILE *file);

void free_stream(Stream *stream);

void stream_set_name(Stream *stream, const struct String *name);

bool stream_ended(const Stream *stream);

char stream_get_char(Stream *stream);

const struct String *stream_get_name(const Stream *stream);

unsigned stream_get_line(const Stream *stream);

unsigned stream_get_col(const Stream *stream);

void stream_unget(Stream *stream);

#endif
