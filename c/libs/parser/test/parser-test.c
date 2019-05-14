#include "glasstypes/glass-class.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "parser/parser.h"
#include "test/test.h"
#include "utils/map.h"
#include "utils/stream.h"
#include "utils/string.h"

Map *get_classes(const char *chars) {
    String *str = string_from_chars(chars);
    Stream *stream = stream_from_string(str);

    Map *classes = parse_classes(stream);

    free_stream(stream);
    free_string(str);

    return classes;
}

int main() {
    Map *classes = get_classes("");
    if (ASSERT_NOT_NULL(classes)) {
        ASSERT_EQUAL(map_size(classes), 0);
        free_map(classes);
    }

    String *capital_m = string_from_chars("M");
    String *lower_m = string_from_chars("m");
    String *under_name = string_from_chars("_name");

    classes = get_classes("{M[m]}");
    if (ASSERT_NOT_NULL(classes)) {
        ASSERT_EQUAL(map_size(classes), 1);
        if (ASSERT_TRUE(map_has(classes, capital_m))) {
            const GlassClass *gclass = map_get(classes, capital_m);
            ASSERT_TRUE(class_has_func(gclass, lower_m));
        }
        free_map(classes);
    }

    classes = get_classes("{(M)[(m)]}");
    if (ASSERT_NOT_NULL(classes)) {
        ASSERT_EQUAL(map_size(classes), 1);
        if (ASSERT_TRUE(map_has(classes, capital_m))) {
            const GlassClass *gclass = map_get(classes, capital_m);
            ASSERT_TRUE(class_has_func(gclass, lower_m));
        }
        free_map(classes);
    }

    classes = get_classes("  {    M    [   m   ]   }    ");
    if (ASSERT_NOT_NULL(classes)) {
        ASSERT_EQUAL(map_size(classes), 1);
        if (ASSERT_TRUE(map_has(classes, capital_m))) {
            const GlassClass *gclass = map_get(classes, capital_m);
            ASSERT_TRUE(class_has_func(gclass, lower_m));
        }
        free_map(classes);
    }

    classes = get_classes("{M[m.?!*^mM3(_name)(42)]}");
    if (ASSERT_NOT_NULL(classes)) {
        ASSERT_EQUAL(map_size(classes), 1);
        if (ASSERT_TRUE(map_has(classes, capital_m))) {
            const GlassClass *gclass = map_get(classes, capital_m);
            if (ASSERT_TRUE(class_has_func(gclass, lower_m))) {
                const GlassFunction *func = class_get_func(gclass, lower_m);
                ASSERT_EQUAL(func_get_command(func, 0)->type, CMD_GET_FUNC);
                ASSERT_EQUAL(func_get_command(func, 1)->type, CMD_EXECUTE_FUNC);
                ASSERT_EQUAL(func_get_command(func, 2)->type, CMD_NEW_INST);
                ASSERT_EQUAL(func_get_command(func, 3)->type, CMD_GET_VAL);
                ASSERT_EQUAL(func_get_command(func, 4)->type, CMD_RETURN);
                ASSERT_EQUAL(func_get_command(func, 5)->type, CMD_PUSH_NAME);
                ASSERT_TRUE(strings_equal(func_get_command(func, 5)->str, lower_m));
                ASSERT_EQUAL(func_get_command(func, 6)->type, CMD_PUSH_NAME);
                ASSERT_TRUE(strings_equal(func_get_command(func, 6)->str, capital_m));
                ASSERT_EQUAL(func_get_command(func, 7)->type, CMD_DUPLICATE);
                ASSERT_EQUAL(func_get_command(func, 7)->index, 3);
                ASSERT_EQUAL(func_get_command(func, 8)->type, CMD_PUSH_NAME);
                ASSERT_TRUE(strings_equal(func_get_command(func, 8)->str, under_name));
                ASSERT_EQUAL(func_get_command(func, 9)->type, CMD_DUPLICATE);
                ASSERT_EQUAL(func_get_command(func, 9)->index, 42);
            }
        }
    }

    free_string(capital_m);
    free_string(lower_m);
    free_string(under_name);

    // Invalid Glass programs, should return NULL
    ASSERT_NULL(get_classes("{"));
    ASSERT_NULL(get_classes("{M"));
    ASSERT_NULL(get_classes("{(Name)"));
    ASSERT_NULL(get_classes("{(Name)[}"));
    ASSERT_NULL(get_classes("{(Name)[n}"));
    ASSERT_NULL(get_classes("{(Name)[(name)"));
    ASSERT_NULL(get_classes("{(Name"));
    ASSERT_NULL(get_classes("!$^*"));
    ASSERT_NULL(get_classes("{M !}"));
    ASSERT_NULL(get_classes("{M[m & ]}"));

    return test_status();
}