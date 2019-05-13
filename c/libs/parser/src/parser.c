#include "parser/parser.h"
#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-class.h"
#include "glasstypes/glass-function.h"
#include "utils/map.h"
#include "utils/stream.h"
#include "utils/string.h"

#include <assert.h>
#include <ctype.h>

static bool skip_whitespace(Stream *stream) {
    while (!stream_ended(stream)) {
        char c = stream_get_char(stream);

        // Skip over comments
        if (c == '\'') {
            do {
                c = stream_get_char(stream);
            } while (c != '\'' && !stream_ended(stream));
        }
        else if (!isspace(c)) {
            stream_unget(stream);
            return true;
        }
    }

    return false;
}

static String *parse_name(Stream *stream) {
    char c = stream_get_char(stream);

    if (isalpha(c)) {
        String *name = new_string();
        string_add_char(name, c);
        return name;
    }
    else if (c == '(') {
        String *name = new_string();

        c = stream_get_char(stream);

        while (!stream_ended(stream) && c != ')') {
            string_add_char(name, c);
            c = stream_get_char(stream);
        }

        return name;
    }
    else {
        stream_unget(stream);
        return NULL;
    }
}

static GlassFunction *parse_function(Stream *stream) {
    char c = stream_get_char(stream);
    assert(c == '[');

    String *name = parse_name(stream);
    if (name == NULL) {
        return NULL;
    }

    GlassFuncBuilder *builder = new_func_builder(name);
    free_string(name);

    skip_whitespace(stream);
    c = stream_get_char(stream);
    while (!stream_ended(stream) && c != ']') {
        skip_whitespace(stream);
        c = stream_get_char(stream);
    }

    if (c != ']') {
        free_func_builder(builder);
        return NULL;
    }

    GlassFunction *func = build_glass_function(builder);
    free_func_builder(builder);
    return func;
}

static GlassClass *parse_class(Stream *stream) {
    char c = stream_get_char(stream);
    assert(c == '{');

    String *name = parse_name(stream);
    if (name == NULL) {
        return NULL;
    }

    GlassClassBuilder *builder = new_class_builder(name);
    free_string(name);

    skip_whitespace(stream);
    c = stream_get_char(stream);
    while (!stream_ended(stream) && c != '}') {
        if (c == '[') {
            stream_unget(stream);
            GlassFunction *func = parse_function(stream);
            if (func == NULL) {
                free_class_builder(builder);
                return NULL;
            }
            if (builder_add_func(builder, func)) {
                free_class_builder(builder);
                return NULL;
            }
        }
        else if (c != '}') {
            free_class_builder(builder);
            return NULL;
        }
        skip_whitespace(stream);
        c = stream_get_char(stream);
    }

    if (c != '}') {
        free_class_builder(builder);
        return NULL;
    }

    GlassClass *gclass = build_glass_class(builder);
    free_class_builder(builder);
    return gclass;
}

Map *parse_classes(Stream *stream) {
    Map *classes = new_map(STRING_HASH_OPS, CLASS_COPY_OPS);

    while (skip_whitespace(stream)) {
        char c = stream_get_char(stream);
        if (c == '{') {
            stream_unget(stream);
            GlassClass *gclass = parse_class(stream);
            if (gclass == NULL) {
                free_map(classes);
                return NULL;
            }
            map_set(classes, class_get_name(gclass), gclass);
            free_glass_class(gclass);
        }
        else {
            free_map(classes);
            return NULL;
        }
    }

    return classes;
}
