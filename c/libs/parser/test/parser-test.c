#include "glasstypes/glass-class.h"
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

    classes = get_classes("{M[m]}");
    if (ASSERT_NOT_NULL(classes)) {
        ASSERT_EQUAL(map_size(classes), 1);
        if (ASSERT_TRUE(map_has(classes, capital_m))) {
            const GlassClass *gclass = map_get(classes, capital_m);
            ASSERT_TRUE(class_has_func(gclass, lower_m));
        }
        free_map(classes);
    }

    free_string(capital_m);
    free_string(lower_m);

    // Invalid Glass programs, should return NULL
    ASSERT_NULL(get_classes("{"));
    ASSERT_NULL(get_classes("{M"));
    ASSERT_NULL(get_classes("{(Name)"));
    ASSERT_NULL(get_classes("{(Name)[}"));
    ASSERT_NULL(get_classes("{(Name)[n}"));
    ASSERT_NULL(get_classes("{(Name)[(name)"));
    ASSERT_NULL(get_classes("{(Name"));

    return test_status();
}