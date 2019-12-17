#include "utils/stream.h"
#include "utils/string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct Stream {
    String *name;

    char *chars;

    size_t cur_index;

    size_t len;

    unsigned line;

    unsigned col;
};

Stream *stream_from_string(const String *str) {
    Stream *stream = malloc(sizeof(Stream));
    stream->chars = malloc(sizeof(char) * string_len(str));
    stream->cur_index = 0;
    stream->len = string_len(str);
    stream->line = 1;
    stream->col = 0;
    stream->name = NULL;
    memcpy(stream->chars, string_data(str), stream->len);
    return stream;
}

Stream *stream_from_file(FILE *file) {
    String *str = new_string();

    int c;

    while ((c = fgetc(file)) != EOF) {
        string_add_char(str, c);
    }

    Stream *stream = stream_from_string(str);
    free_string(str);
    return stream;
}

void free_stream(Stream *stream) {
    if (stream->name != NULL) {
        free_string(stream->name);
    }
    free(stream->chars);
    free(stream);
}

void stream_set_name(Stream *stream, const struct String *name) {
    if (stream->name != NULL) {
        free_string(stream->name);
    }
    stream->name = copy_string(name);
}

bool stream_ended(const Stream *stream) {
    return stream->cur_index == stream->len;
}

char stream_get_char(Stream *stream) {
    if (stream_ended(stream)) {
        return 0;
    }
    else {
        if (stream->cur_index > 0 && stream->chars[stream->cur_index - 1] == '\n') {
            stream->line++;
            stream->col = 1;
        }
        else {
            stream->col++;
        }
        return stream->chars[stream->cur_index++];
    }
}

const String *stream_get_name(const Stream *stream) {
    return stream->name;
}

unsigned stream_get_line(const Stream *stream) {
    return stream->line;
}

unsigned stream_get_col(const Stream *stream) {
    return stream->col;
}

void stream_unget(Stream *stream) {
    if (stream->cur_index > 0) {
        stream->cur_index--;
        stream->col--;
    } 
}
