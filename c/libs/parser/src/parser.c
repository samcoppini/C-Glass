#include "parser/parser.h"
#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-class.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/stream.h"
#include "utils/string.h"

#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>

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
    skip_whitespace(stream);
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

static bool parse_parenthesized(Stream *stream, GlassCommand *cmd) {
    char c = stream_get_char(stream);

    assert(c == '(');

    c = stream_get_char(stream);

    if (isalpha(c) || c == '_') {
        cmd->type = CMD_PUSH_NAME;
        cmd->str = string_from_char(c);

        c = stream_get_char(stream);
        while (c != ')' && !stream_ended(stream)) {
            if (!isalnum(c) && c != '_') {
                return true;
            }
            string_add_char(cmd->str, c);
            c = stream_get_char(stream);
        }

        if (c != ')') {
            free_string(cmd->str);
            return true;
        }

        return false;
    }
    else if (isdigit(c)) {
        cmd->type = CMD_DUPLICATE;
        cmd->index = c - '0';
        c = stream_get_char(stream);

        while (c != ')' && !stream_ended(stream)) {
            if (!isdigit(c)) {
                return true;
            }
            cmd->index = (cmd->index * 10) + c - '0';
            c = stream_get_char(stream);
        }

        if (c != ')') {
            return true;
        }

        return false;
    }
    else {
        return true;
    }
}

static bool parse_quoted(Stream *stream, GlassCommand *cmd) {
    char c = stream_get_char(stream);

    assert(c == '"');

    cmd->type = CMD_PUSH_STR;
    cmd->str = new_string();

    c = stream_get_char(stream);
    while (c != '"' && !stream_ended(stream)) {
        if (c == '\\') {
            if (stream_ended(stream)) {
                free_string(cmd->str);
                return true;
            }
            c = stream_get_char(stream);
            switch (c) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                default: break;
            }
        }
        string_add_char(cmd->str, c);
        c = stream_get_char(stream);
    }

    if (c != '"') {
        free_string(cmd->str);
        return true;
    }

    return false;
}

static bool parse_angled(Stream *stream, GlassCommand *cmd) {
    char c = stream_get_char(stream);

    assert(c == '<');

    String *num_str = new_string();
    cmd->type = CMD_PUSH_NUM;
    
    c = stream_get_char(stream);
    while (c != '>' && !stream_ended(stream)) {
        string_add_char(num_str, c);
        c = stream_get_char(stream);
    }

    if (c != '>') {
        free_string(num_str);
        return true;
    }

    cmd->number = atof(string_get_c_str(num_str));
    free_string(num_str);

    return false;
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
        GlassCommand cmd = { .type = CMD_NOP };

        switch (c) {
            case '.':
                cmd.type = CMD_GET_FUNC;
                break;
            case '?':
                cmd.type = CMD_EXECUTE_FUNC;
                break;
            case '^':
                cmd.type = CMD_RETURN;
                break;
            case '!':
                cmd.type = CMD_NEW_INST;
                break;
            case ',':
                cmd.type = CMD_POP_STACK;
                break;
            case '*':
                cmd.type = CMD_GET_VAL;
                break;
            case '$':
                cmd.type = CMD_ASSIGN_SELF;
                break;
            case '=':
                cmd.type = CMD_ASSIGN_VAL;
                break;
            case '(':
                stream_unget(stream);
                if (parse_parenthesized(stream, &cmd)) {
                    free_func_builder(builder);
                    return NULL;
                }
                break;
            case '"':
                stream_unget(stream);
                if (parse_quoted(stream, &cmd)) {
                    free_func_builder(builder);
                    return NULL;
                }
                break;
            case '<':
                stream_unget(stream);
                if (parse_angled(stream, &cmd)) {
                    free_func_builder(builder);
                    return NULL;
                }
                break;
            case '/':
                name = parse_name(stream);
                if (name == NULL) {
                    free_func_builder(builder);
                    return NULL;
                }
                builder_start_loop(builder, name);
                free_string(name);
                break;
            case '\\':
                if (builder_end_loop(builder)) {
                    return NULL;
                }
                break;
            default:
                if (isalpha(c)) {
                    cmd.type = CMD_PUSH_NAME;
                    cmd.str = string_from_char(c);
                }
                else if (isdigit(c)) {
                    cmd.type = CMD_DUPLICATE;
                    cmd.index = c - '0';
                }
                else {
                    free_func_builder(builder);
                    return NULL;
                }
                break;
        }

        if (cmd.type != CMD_NOP) {
            builder_add_command(builder, &cmd);
        }

        if (cmd.type == CMD_PUSH_NAME  || cmd.type == CMD_PUSH_STR) {
            free_string(cmd.str);
        }

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

Map *classes_from_files(List *filenames, bool include_builtins) {
    Map *classes;

    if (include_builtins) {
        classes = get_builtin_classes();
    }
    else {
        classes = new_map(STRING_HASH_OPS, CLASS_COPY_OPS);
    }

    for (size_t i = 0; i < list_len(filenames); i++) {
        String *str = list_get_mutable(filenames, i);
        FILE *fp = fopen(string_get_c_str(str), "r");
        free_string(str);

        if (fp == NULL) {
            fprintf(stderr, "Unable to open %s!\n", string_get_c_str(str));
            free_map(classes);
            return NULL;
        }

        Stream *file_stream = stream_from_file(fp);
        fclose(fp);

        Map *new_classes = parse_classes(file_stream);
        free_stream(file_stream);

        if (new_classes == NULL) {
            free_map(classes);
            return NULL;
        }

        List *class_names = map_get_keys(new_classes);
        for (size_t j = 0; j < list_len(class_names); j++) {
            String *cname = list_get_mutable(class_names, j);

            if (map_has(classes, cname)) {
                fprintf(stderr, "Error! %s defined multiple times!\n", 
                        string_get_c_str(cname));
                free_list(class_names);
                free_map(new_classes);
                free_map(classes);
                return NULL;
            }

            const GlassClass *gclass = map_get(new_classes, cname);
            map_set(classes, cname, gclass);
        }

        free_list(class_names);
        free_map(new_classes);
    }

    return classes;
}