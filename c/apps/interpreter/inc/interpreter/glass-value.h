#ifndef INTERPRETER_GLASS_VALUE_H
#define INTERPRETER_GLASS_VALUE_H

#include "interpreter/glass-instance.h"

#include <stdio.h>

struct CopyInterface;
struct String;

typedef enum ValueType {
    VALUE_FUNCTION,
    VALUE_INSTANCE,
    VALUE_INPUT_FILE,
    VALUE_NAME,
    VALUE_NUMBER,
    VALUE_OUTPUT_FILE,
    VALUE_STRING,
} ValueType;

typedef struct GlassValue {
    ValueType type;

    union {
        struct {
            union {
                GlassInstance inst;

                FILE *file;
            };

            struct String *str;
        };

        double num;
    };
} GlassValue;

extern const struct CopyInterface *VALUE_COPY_OPS;

GlassValue *new_func_value(GlassInstance inst, const struct String *name);

GlassValue *new_in_file_value(const struct String *name);

GlassValue *new_inst_value(GlassInstance inst);

GlassValue *new_name_value(const struct String *name);

GlassValue *new_number_value(double num);

GlassValue *new_out_file_value(const struct String *name);

GlassValue *new_str_value(const struct String *str);

GlassValue *copy_value(const GlassValue *val);

void free_glass_value(GlassValue *value);

struct String *value_get_string(const GlassValue *val);

#endif
