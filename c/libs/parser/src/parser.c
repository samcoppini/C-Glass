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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void parser_error(const Stream *stream, const char *msg, ...) {
    String *name = copy_string(stream_get_name(stream));

    fprintf(stderr, "Error in %s on line %u, column %u:\n    ",
                    string_get_c_str(name),
                    stream_get_line(stream),
                    stream_get_col(stream));
    
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    fprintf(stderr, "\n");
}

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
            if (!isalnum(c) && c != '_') {
                parser_error(stream, "Invalid char '%c' encountered in the middle of a name.", c);
                free_string(name);
                return NULL;
            }
            string_add_char(name, c);
            c = stream_get_char(stream);
        }

        return name;
    }
    else {
        parser_error(stream, "Invalid char '%c' encountered when expecting a name.", c);
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
                parser_error(stream, "Unexpected '%c' encountered while parsing name.", c);
                return true;
            }
            string_add_char(cmd->str, c);
            c = stream_get_char(stream);
        }

        if (c != ')') {
            parser_error(stream, "File ended unexpectedly while parsing name.");
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
                parser_error(stream, "Expected a digit, encountered '%c", c);
                return true;
            }
            cmd->index = (cmd->index * 10) + c - '0';
            c = stream_get_char(stream);
        }

        if (c != ')') {
            parser_error(stream, "File ended unexpectedly while parsing dup expression.");
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
                parser_error(stream, "File ended uexpectedly in the middle of a string!");
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
        parser_error(stream, "File ended uexpectedly in the middle of a string!");
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

    unsigned line = stream_get_line(stream);
    unsigned col = stream_get_col(stream);

    String *name = parse_name(stream);
    if (name == NULL) {
        return NULL;
    }

    GlassFuncBuilder *builder = new_func_builder(name, stream_get_name(stream), line, col);
    free_string(name);

    skip_whitespace(stream);
    c = stream_get_char(stream);
    while (!stream_ended(stream) && c != ']') {
        GlassCommand cmd;
        cmd.filename = copy_string(stream_get_name(stream));
        cmd.line = stream_get_line(stream);
        cmd.col = stream_get_col(stream);

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
                cmd.type = CMD_LOOP_BEGIN;
                cmd.str = parse_name(stream);
                if (cmd.str == NULL) {
                    free_func_builder(builder);
                    return NULL;
                }
                break;
            case '\\':
                cmd.type = CMD_LOOP_END;
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
                    parser_error(stream, "Invalid char %c encountered!", c);
                    free_func_builder(builder);
                    return NULL;
                }
                break;
        }

        if (builder_add_command(builder, &cmd)) {
            // The only way adding a command can fail is if it's a loop ending
            // that doesn't match with a loop beginning, so we can put this error
            // message here
            parser_error(stream, "Encountered a '\\' without a matching '/'.");
            return NULL;
        }

        if (cmd.type == CMD_PUSH_NAME  || cmd.type == CMD_PUSH_STR || cmd.type == CMD_LOOP_BEGIN) {
            free_string(cmd.str);
        }

        free_string(cmd.filename);

        skip_whitespace(stream);
        c = stream_get_char(stream);
    }

    if (c != ']') {
        free_func_builder(builder);
        return NULL;
    }

    GlassFunction *func = build_glass_function(builder);
    if (func == NULL) {
        // The only way the building can fail is if there's a unclosed loop
        parser_error(stream, "Loop not closed before function ended!");
    }
    free_func_builder(builder);
    return func;
}

static GlassClassBuilder *parse_class(Stream *stream) {
    char c = stream_get_char(stream);
    assert(c == '{');

    unsigned line = stream_get_line(stream);
    unsigned col = stream_get_col(stream);

    String *name = parse_name(stream);
    if (name == NULL) {
        return NULL;
    }

    GlassClassBuilder *builder = new_class_builder(name, stream_get_name(stream), line, col);
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
            builder_add_func(builder, func);
        }
        else if (c == '(') {
            stream_unget(stream);
            String *parent = parse_name(stream);
            builder_add_parent(builder, parent);
            free_string(parent);
        }
        else if (isalpha(c)) {
            String *parent = string_from_char(c);
            builder_add_parent(builder, parent);
            free_string(parent);
        }
        else if (c != '}') {
            parser_error(stream, "Error! Unexpected char %c in a class definition!", c);
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

    return builder;
}

bool parse_classes(GlassProgramBuilder *builder, Stream *stream) {
    while (skip_whitespace(stream)) {
        char c = stream_get_char(stream);
        if (c == '{') {
            stream_unget(stream);
            GlassClassBuilder *gclass = parse_class(stream);
            if (gclass == NULL) {
                return true;
            }
            builder_add_class(builder, gclass);
            free_class_builder(gclass);
        }
        else {
            parser_error(stream, "Unexpected char '%c' encountered when expecting '{'.", c);
            return true;
        }
    }

    return false;
}

Map *classes_from_files(List *filenames,
                        bool include_builtins,
                        bool resolve_inheritance)
{
    GlassProgramBuilder *builder = new_program_builder();

    if (include_builtins) {
        add_builtin_classes(builder);
    }

    for (size_t i = 0; i < list_len(filenames); i++) {
        String *filename = list_get_mutable(filenames, i);
        FILE *fp = fopen(string_get_c_str(filename), "r");

        if (fp == NULL) {
            fprintf(stderr, "Unable to open %s!\n", string_get_c_str(filename));
            free_program_builder(builder);
            return NULL;
        }

        Stream *file_stream = stream_from_file(fp);
        stream_set_name(file_stream, filename);
        fclose(fp);

        bool failed = parse_classes(builder, file_stream);
        free_stream(file_stream);

        if (failed) {
            free_program_builder(builder);
            return NULL;
        }
    }

    Map *classes = build_glass_program(builder, resolve_inheritance);
    free_program_builder(builder);

    return classes;
}
