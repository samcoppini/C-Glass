#include "glasstypes/glass-builders.h"
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
    stream_set_name(stream, str);

    GlassProgramBuilder *builder = new_program_builder();

    if (parse_classes(builder, stream)) {
        free_program_builder(builder);
        free_stream(stream);
        free_string(str);
        return NULL;
    }

    Map *classes = build_glass_program(builder, true);

    free_program_builder(builder);
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

    classes = get_classes("  {    M    [   m   /  (_m)  \\  ]   }    ");
    if (ASSERT_NOT_NULL(classes)) {
        ASSERT_EQUAL(map_size(classes), 1);
        if (ASSERT_TRUE(map_has(classes, capital_m))) {
            const GlassClass *gclass = map_get(classes, capital_m);
            ASSERT_TRUE(class_has_func(gclass, lower_m));
        }
        free_map(classes);
    }

    classes = get_classes("{M[m.?!*^mM3(_name)(42)\"_name\"/(m)/M\\\\$<100>]}");
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
                ASSERT_EQUAL(func_get_command(func, 10)->type, CMD_PUSH_STR);
                ASSERT_TRUE(strings_equal(func_get_command(func, 10)->str, under_name));
                ASSERT_EQUAL(func_get_command(func, 11)->type, CMD_LOOP_BEGIN);
                ASSERT_EQUAL(func_get_command(func, 11)->index, 14);
                ASSERT_TRUE(strings_equal(func_get_command(func, 11)->str, lower_m));
                ASSERT_EQUAL(func_get_command(func, 12)->type, CMD_LOOP_BEGIN);
                ASSERT_EQUAL(func_get_command(func, 12)->index, 13);
                ASSERT_TRUE(strings_equal(func_get_command(func, 12)->str, capital_m));
                ASSERT_EQUAL(func_get_command(func, 13)->type, CMD_LOOP_END);
                ASSERT_EQUAL(func_get_command(func, 13)->index, 12);
                ASSERT_TRUE(strings_equal(func_get_command(func, 13)->str, capital_m));
                ASSERT_EQUAL(func_get_command(func, 14)->type, CMD_LOOP_END);
                ASSERT_EQUAL(func_get_command(func, 14)->index, 11);
                ASSERT_TRUE(strings_equal(func_get_command(func, 14)->str, lower_m));
                ASSERT_EQUAL(func_get_command(func, 15)->type, CMD_ASSIGN_SELF);
                ASSERT_EQUAL(func_get_command(func, 16)->type, CMD_PUSH_NUM);
                ASSERT_EQUAL(func_get_command(func, 16)->number, 100);
            }
        }
    }

    free_string(capital_m);
    free_string(lower_m);
    free_string(under_name);

    // Invalid Glass programs, should return NULL
    ASSERT_NULL(get_classes("{"));
    ASSERT_NULL(get_classes("{}"));
    ASSERT_NULL(get_classes("{M"));
    ASSERT_NULL(get_classes("{(Name)"));
    ASSERT_NULL(get_classes("{(Name)[}"));
    ASSERT_NULL(get_classes("{(Name)[n}"));
    ASSERT_NULL(get_classes("{(Name)[(name)"));
    ASSERT_NULL(get_classes("{(Name"));
    ASSERT_NULL(get_classes("!$^*"));
    ASSERT_NULL(get_classes("{M !}"));
    ASSERT_NULL(get_classes("{M[m & ]}"));
    ASSERT_NULL(get_classes("{M[m(a]}"));
    ASSERT_NULL(get_classes("{M[m<2]}"));
    ASSERT_NULL(get_classes("{M[m/]}"));
    ASSERT_NULL(get_classes("{M[m/(_name]}"));
    ASSERT_NULL(get_classes("{M[m/(_name)]}"));
    ASSERT_NULL(get_classes("{M[m\\]}"));
    ASSERT_NULL(get_classes("{M[m($)]}"));
    ASSERT_NULL(get_classes("{M[m(n$)]}"));
    ASSERT_NULL(get_classes("{M[m(0$)]}"));
    ASSERT_NULL(get_classes("{M[m\"]}"));
    ASSERT_NULL(get_classes("{M[m][m]}"));
    ASSERT_NULL(get_classes("{M[m]}{M[m]}"));
    ASSERT_NULL(get_classes("{MNN[m]}{N}"));
    ASSERT_NULL(get_classes("{MZ[m]}"));
    ASSERT_NULL(get_classes("{MN[m]}{N}"));

    return test_status();
}
