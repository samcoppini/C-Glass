#ifndef INTERPRETER_GLASS_VALUE_H
#define INTERPRETER_GLASS_VALUE_H

struct CopyInterface;
struct GlassInstance;
struct String;

typedef enum ValueType {
    VALUE_FUNCTION,
    VALUE_INSTANCE,
    VALUE_NAME,
    VALUE_NUMBER,
    VALUE_STRING,
} ValueType;

typedef struct GlassValue {
    ValueType type;

    union {
        struct {
            struct GlassInstance *inst;

            struct String *str;
        };

        double num;
    };
} GlassValue;

extern const struct CopyInterface *VALUE_COPY_OPS;

GlassValue *new_func_value(struct GlassInstance *inst, const struct String *name);

GlassValue *new_inst_value(struct GlassInstance *inst);

GlassValue *new_name_value(const struct String *name);

GlassValue *new_number_value(double num);

GlassValue *new_str_value(const struct String *str);

GlassValue *copy_value(const GlassValue *val);

void free_glass_value(GlassValue *value);

struct String *value_get_string(const GlassValue *val);

#endif
