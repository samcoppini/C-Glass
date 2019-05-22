#include "interpreter/glass-value.h"
#include "interpreter/glass-instance.h"

#include "glasstypes/glass-class.h"
#include "utils/copy-interface.h"
#include "utils/string.h"

#include <stdio.h>
#include <stdlib.h>

static GlassValue *new_glass_value(ValueType type) {
    GlassValue *val = malloc(sizeof(GlassValue));
    val->type = type;
    return val;
}

GlassValue *new_func_value(GlassInstance inst, const String *name) {
    GlassValue *val = new_glass_value(VALUE_FUNCTION);
    val->inst = copy_glass_instance(inst);
    val->str = copy_string(name);
    return val;
}

GlassValue *new_inst_value(GlassInstance inst) {
    GlassValue *val = new_glass_value(VALUE_INSTANCE);
    val->inst = copy_glass_instance(inst);
    return val;
}

GlassValue *new_name_value(const String *name) {
    GlassValue *val = new_glass_value(VALUE_NAME);
    val->str = copy_string(name);
    return val;
}

GlassValue *new_number_value(double num) {
    GlassValue *val = new_glass_value(VALUE_NUMBER);
    val->num = num;
    return val;
}

GlassValue *new_str_value(const String *str) {
    GlassValue *val = new_glass_value(VALUE_STRING);
    val->str = copy_string(str);
    return val;
}

GlassValue *copy_glass_value(const GlassValue *value) {
    GlassValue *copy = new_glass_value(value->type);

    switch (value->type) {
        case VALUE_INSTANCE:
            copy->inst = copy_glass_instance(value->inst);
            break;
        
        case VALUE_FUNCTION:
            copy->inst = copy_glass_instance(value->inst);
            copy->str = copy_string(value->str);
            break;

        case VALUE_NAME:
        case VALUE_STRING:
            copy->str = copy_string(value->str);
            break;

        case VALUE_NUMBER:
            copy->num = value->num;
            break;
    }

    return copy;
}

void free_glass_value(GlassValue *value) {
    switch (value->type) {
        case VALUE_INSTANCE:
            release_glass_instance(value->inst);
            break;

        case VALUE_FUNCTION:
            release_glass_instance(value->inst);
            free_string(value->str);
            break;

        case VALUE_STRING:
        case VALUE_NAME:
            free_string(value->str);
            break;
        
        default:
            break;
    }

    free(value);
}

String *value_get_string(const GlassValue *val) {
    switch (val->type) {
        case VALUE_FUNCTION: {
            String *str = string_from_chars("{(");
            const GlassClass *gclass = instance_get_class(val->inst);
            const String *cname = class_get_name(gclass);
            string_add_str(str, cname);
            string_add_chars(str, ")[(");
            string_add_str(str, val->str);
            string_add_chars(str, ")]}");
            return str;
        }

        case VALUE_INSTANCE: {
            String *str = string_from_chars("{(");
            const GlassClass *gclass = instance_get_class(val->inst);
            const String *cname = class_get_name(gclass);
            string_add_str(str, cname);
            string_add_chars(str, ")}");
            return str;
        }

        case VALUE_NAME: {
            String *str = string_from_char('(');
            string_add_str(str, val->str);
            string_add_char(str, ')');
            return str;
        }

        case VALUE_NUMBER: {
            char num_buf[30];
            sprintf(num_buf, "<%g>", val->num);
            String *str = string_from_chars(num_buf);
            return str;
        }

        case VALUE_STRING: {
            String *str = string_from_char('"');
            for (size_t i = 0; i < string_len(val->str); i++) {
                char c = string_get(val->str, i);
                switch (c) {
                    case '\\':
                        string_add_chars(str, "\\\\");
                        break;

                    case '"':
                        string_add_chars(str, "\\\"");
                        break;

                    case '\n':
                        string_add_chars(str, "\\n");
                        break;
                    
                    case '\r':
                        string_add_chars(str, "\\r");
                        break;
                    
                    case '\t':
                        string_add_chars(str, "\\t");
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
            return string_from_chars("<unknown>");
    }
}

void *copy_glass_value_generic(const void *val) {
    return copy_glass_value(val);
}

void free_glass_value_generic(void *val) {
    free_glass_value(val);
}

const CopyInterface *VALUE_COPY_OPS = &(CopyInterface) {
    copy_glass_value_generic,
    free_glass_value_generic,
};
