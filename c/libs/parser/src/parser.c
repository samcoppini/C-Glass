#include "parser/parser.h"
#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-class.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "utils/map.h"
#include "utils/stream.h"
#include "utils/string.h"

#include <assert.h>
//#include <ctype.h>

bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_space(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f';
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
        else if (!is_space(c)) {
            stream_unget(stream);
            return true;
        }
    }

    return false;
}

static String *parse_name(Stream *stream) {
    skip_whitespace(stream);
    char c = stream_get_char(stream);

    if (is_alpha(c)) {
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

    if (is_alpha(c) || c == '_') {
        cmd->type = CMD_PUSH_NAME;
        cmd->str = string_from_char(c);

        c = stream_get_char(stream);
        while (c != ')' && !stream_ended(stream)) {
            if (!is_alpha(c) && c != '_') {
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
    else if (is_digit(c)) {
        cmd->type = CMD_DUPLICATE;
        cmd->index = c - '0';
        c = stream_get_char(stream);

        while (c != ')' && !stream_ended(stream)) {
            if (!is_digit(c)) {
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
        return true;
    }

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
                if (is_alpha(c)) {
                    cmd.type = CMD_PUSH_NAME;
                    cmd.str = string_from_char(c);
                }
                else if (is_digit(c)) {
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
