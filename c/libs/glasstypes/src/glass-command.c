#include "glasstypes/glass-command.h"
#include "utils/copy-interface.h"
#include "utils/string.h"

#include <stdlib.h>
#include <stdio.h>

void *copy_command(const GlassCommand *cmd) {
    GlassCommand *copy = malloc(sizeof(GlassCommand));
    copy->type = cmd->type;
    copy->filename = copy_string(cmd->filename);
    copy->line = cmd->line;
    copy->col = cmd->col;

    switch (cmd->type) {
        case CMD_DUPLICATE:
            copy->index = cmd->index;
            break;

        case CMD_PUSH_NAME:
        case CMD_PUSH_STR:
            copy->str = copy_string(cmd->str);
            break;

        case CMD_PUSH_NUM:
            copy->number = cmd->number;
            break;

        case CMD_LOOP_BEGIN:
        case CMD_LOOP_END:
            copy->index = cmd->index;
            copy->str = copy_string(cmd->str);
            break;

        case CMD_BUILTIN:
            copy->builtin = cmd->builtin;
            break;
        
        default:
            break;
    }

    return copy;
}

void free_command(GlassCommand *cmd) {
    switch (cmd->type) {
        case CMD_LOOP_BEGIN:
        case CMD_LOOP_END:
        case CMD_PUSH_NAME:
        case CMD_PUSH_STR:
            free_string(cmd->str);
            break;
        
        default:
            break;
    }

    free_string(cmd->filename);
    free(cmd);
}

String *command_to_str(const GlassCommand *cmd) {
    switch (cmd->type) {
        case CMD_ASSIGN_SELF:
            return string_from_char('$');
        case CMD_ASSIGN_VAL:
            return string_from_char('=');
        case CMD_EXECUTE_FUNC:
            return string_from_char('?');
        case CMD_GET_FUNC:
            return string_from_char('.');
        case CMD_GET_VAL:
            return string_from_char('*');
        case CMD_LOOP_END:
            return string_from_char('\\');
        case CMD_NEW_INST:
            return string_from_char('!');
        case CMD_POP_STACK:
            return string_from_char(',');
        case CMD_RETURN:
            return string_from_char('^');
        case CMD_LOOP_BEGIN: {
            String *str = string_from_char('/');
            if (string_len(cmd->str) == 1) {
                string_add_str(str, cmd->str);
            }
            else {
                string_add_char(str, '(');
                string_add_str(str, cmd->str);
                string_add_char(str, ')');
            }
            return str;
        }
        case CMD_DUPLICATE:
            if (cmd->index < 10) {
                return string_from_char(cmd->index + '0');
            }
            else {
                char buf[1000];
                sprintf(buf, "(%u)", (unsigned) cmd->index);
                return string_from_chars(buf);
            }
        case CMD_PUSH_NAME:
            if (string_len(cmd->str) == 1) {
                return copy_string(cmd->str);
            }
            else {
                String *str = string_from_char('(');
                string_add_str(str, cmd->str);
                string_add_char(str, ')');
                return str;
            }
        case CMD_PUSH_NUM: {
            char buf[1000];
            sprintf(buf, "<%g>", cmd->number);
            return string_from_chars(buf);
        }
        case CMD_PUSH_STR: {
            String *str = string_from_char('"');
            for (size_t i = 0; i < string_len(cmd->str); i++) {
                char c = string_get(cmd->str, i);
                switch (c) {
                    case '\n':
                        string_add_chars(str, "\\n");
                        break;
                    case '\r':
                        string_add_chars(str, "\\r");
                        break;
                    case '\t':
                        string_add_chars(str, "\\t");
                        break;
                    case '"':
                        string_add_chars(str, "\\\"");
                        break;
                    default:
                        string_add_char(str, c);
                        break;
                }
            }
            string_add_char(str, '"');
            return str;
        }
        default:
            return new_string();
    }
}

String *builtin_func_name(BuiltinFunc func) {
    switch (func) {
        case BUILTIN_MATH_ADD:
            return string_from_chars("add_numbers");
        case BUILTIN_MATH_SUBTRACT:
            return string_from_chars("subtract_numbers");
        case BUILTIN_MATH_MULTIPLY:
            return string_from_chars("multiply_numbers");
        case BUILTIN_MATH_DIVIDE:
            return string_from_chars("divide_numbers");
        case BUILTIN_MATH_MODULO:
            return string_from_chars("modulo_numbers");
        case BUILTIN_MATH_FLOOR:
            return string_from_chars("floor_number");
        case BUILTIN_MATH_EQUAL:
            return string_from_chars("numbers_equal");
        case BUILTIN_MATH_NOT_EQUAL:
            return string_from_chars("numbers_not_equal");
        case BUILTIN_MATH_GREATER_THAN:
            return string_from_chars("numbers_greater_than");
        case BUILTIN_MATH_GREATER_OR_EQUAL:
            return string_from_chars("numbers_greater_or_equal");
        case BUILTIN_MATH_LESS_THAN:
            return string_from_chars("numbers_less_than");
        case BUILTIN_MATH_LESS_OR_EQUAL:
            return string_from_chars("numbers_less_or_equal");
        case BUILTIN_OUTPUT_NUM:
            return string_from_chars("output_num");
        case BUILTIN_OUTPUT_STR:
            return string_from_chars("output_string");
        default:
            return string_from_chars("unimplemented");
    }
}

static void *copy_command_generic(const void *cmd) {
    return copy_command(cmd);
}

static void free_command_generic(void *cmd) {
    free_command(cmd);
}

const CopyInterface *CMD_COPY_OPS = &(CopyInterface) {
    copy_command_generic,
    free_command_generic,
};
