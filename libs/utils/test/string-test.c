#include "test/test.h"
#include "utils/string.h"

#include <string.h>

int main() {
    String *str = new_string();
    ASSERT_EQUAL(string_len(str), 0);

    string_add_char(str, 'X');
    ASSERT_EQUAL(string_len(str), 1);
    ASSERT_EQUAL(string_get(str, 0), 'X');

    string_add_chars(str, "Howdy!");
    ASSERT_EQUAL(string_len(str), 7);
    ASSERT_FALSE(strcmp(string_data(str), "XHowdy!"));

    String *str2 = string_from_chars("0123456789abcdef");
    ASSERT_EQUAL(string_len(str2), 16);
    ASSERT_FALSE(strcmp(string_data(str2), "0123456789abcdef"));

    string_add_str(str, str2);
    ASSERT_EQUAL(string_len(str), 23);
    ASSERT_FALSE(strcmp(string_data(str), "XHowdy!0123456789abcdef"));

    free_string(str);
    free_string(str2);

    return test_status();
}
