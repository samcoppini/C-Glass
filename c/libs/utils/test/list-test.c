#include "test/test.h"
#include "utils/list.h"
#include "utils/string.h"

#include <string.h>

int main() {
    List *list = new_list(STRING_COPY_OPS);

    ASSERT_EQUAL(list_len(list), 0);

    const char *TEST_STRINGS[6] = {
        "Hello", "What", "Zoo", "Zip", "????", "Yargh",
    };

    for (size_t i = 0; i < 6; i++) {
        String *str = string_from_chars(TEST_STRINGS[i]);

        list_add(list, str);

        free_string(str);
    }

    ASSERT_EQUAL(list_len(list), 6);

    for (size_t i = 0; i < list_len(list); i++) {
        const String *str = list_get(list, i);

        ASSERT_FALSE(strcmp(string_data(str), TEST_STRINGS[i]));
    }

    free_list(list);

    return test_status();
}
