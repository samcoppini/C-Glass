#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-class.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "utils/map.h"
#include "utils/string.h"

#define NUM_BUILTIN_CLASSES  5
#define MAX_BUILTIN_FUNCS   15

typedef struct BuiltinFuncInfo {
    const char *func_name;

    BuiltinFunc builtin_func;
} BuiltinFuncInfo;

typedef struct BuiltinInfo {
    const char *class_name;

    BuiltinFuncInfo funcs[MAX_BUILTIN_FUNCS];
} BuiltinInfo;

const BuiltinInfo BUILTIN_CLASS_INFO[NUM_BUILTIN_CLASSES] = {
    {"A", {
        {"a",   BUILTIN_MATH_ADD},
        {"d",   BUILTIN_MATH_DIVIDE},
        {"e",   BUILTIN_MATH_EQUAL},
        {"f",   BUILTIN_MATH_FLOOR},
        {"ge",  BUILTIN_MATH_GREATER_OR_EQUAL},
        {"gt",  BUILTIN_MATH_GREATER_THAN},
        {"le",  BUILTIN_MATH_LESS_OR_EQUAL},
        {"lt",  BUILTIN_MATH_LESS_THAN},
        {"m",   BUILTIN_MATH_MULTIPLY},
        {"mod", BUILTIN_MATH_MODULO},
        {"ne",  BUILTIN_MATH_NOT_EQUAL},
        {"s",   BUILTIN_MATH_SUBTRACT}},
    },
    {"I", {
        {"c", BUILTIN_INPUT_CHAR},
        {"e", BUILTIN_INPUT_EOF},
        {"l", BUILTIN_INPUT_LINE}},
    },
    {"O", {
        {"o",  BUILTIN_OUTPUT_STR},
        {"on", BUILTIN_OUTPUT_NUM}},
    },
    {"S", {
        {"a",  BUILTIN_STR_APPEND},
        {"e",  BUILTIN_STR_EQUAL},
        {"d",  BUILTIN_STR_SPLIT},
        {"i",  BUILTIN_STR_INDEX},
        {"l",  BUILTIN_STR_LENGTH},
        {"ns", BUILTIN_STR_NUM_TO_STR},
        {"si", BUILTIN_STR_REPLACE},
        {"sn", BUILTIN_STR_STR_TO_NUM}},
    },
    {"V", {
        {"d", BUILTIN_VAR_DELETE},
        {"n", BUILTIN_VAR_NEW}}
    },
};

Map *get_builtin_classes(void) {
    Map *classes = new_map(STRING_HASH_OPS, CLASS_COPY_OPS);

    for (size_t i = 0; i < NUM_BUILTIN_CLASSES; i++) {
        BuiltinInfo builtin_class = BUILTIN_CLASS_INFO[i];

        String *class_name = string_from_chars(builtin_class.class_name);
        GlassClassBuilder *class_builder = new_class_builder(class_name);

        for (size_t j = 0; j < MAX_BUILTIN_FUNCS; j++) {
            BuiltinFuncInfo func_info = builtin_class.funcs[j];

            if (func_info.func_name == NULL) {
                break;
            }

            String *func_name = string_from_chars(func_info.func_name);
            GlassFuncBuilder *func_builder = new_func_builder(func_name);
            GlassCommand cmd = { CMD_BUILTIN, .builtin = func_info.builtin_func };

            builder_add_command(func_builder, &cmd);
            GlassFunction *func = build_glass_function(func_builder);
            builder_add_func(class_builder, func);

            free_glass_func(func);
            free_func_builder(func_builder);
            free_string(func_name);
        }

        GlassClass *gclass = build_glass_class(class_builder);
        map_set(classes, class_name, gclass);

        free_string(class_name);
        free_class_builder(class_builder);
        free_glass_class(gclass);
    }

    return classes;
}
