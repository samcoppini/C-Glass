#include "interpreter/glass-value.h"
#include "interpreter/glass-instance.h"

#include "utils/copy-interface.h"
#include "utils/string.h"

#include <stdlib.h>

static GlassValue *new_glass_value(ValueType type) {
    GlassValue *val = malloc(sizeof(GlassValue));
    val->type = type;
    return val;
}

GlassValue *new_func_value(GlassInstance *inst, const String *name) {
    GlassValue *val = new_glass_value(VALUE_FUNCTION);
    val->inst = inst;
    val->str = copy_string(name);
    return val;
}

GlassValue *new_inst_value(GlassInstance *inst) {
    GlassValue *val = new_glass_value(VALUE_INSTANCE);
    val->inst = inst;
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
            copy->inst = value->inst;
            break;
        
        case VALUE_FUNCTION:
            copy->inst = value->inst;
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
        case VALUE_FUNCTION:
        case VALUE_STRING:
        case VALUE_NAME:
            free_string(value->str);
            break;
        
        default:
            break;
    }

    free(value);
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
