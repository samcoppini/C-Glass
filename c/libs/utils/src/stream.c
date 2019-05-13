#include "utils/stream.h"
#include "utils/string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct Stream {
    char *chars;

    size_t cur_index;

    size_t len;
};

Stream *stream_from_string(const String *str) {
    Stream *stream = malloc(sizeof(Stream));
    stream->chars = malloc(sizeof(char) * string_len(str));
    stream->cur_index = 0;
    stream->len = string_len(str);
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
    free(stream->chars);
    free(stream);
}

bool stream_ended(const Stream *stream) {
    return stream->cur_index == stream->len;
}

char stream_get_char(Stream *stream) {
    if (stream_ended(stream)) {
        return 0;
    }
    else {
        return stream->chars[stream->cur_index++];
    }
}

void stream_unget(Stream *stream) {
    if (stream->cur_index > 0) {
        stream->cur_index--;
    } 
}
