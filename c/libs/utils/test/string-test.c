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
    ASSERT_FALSE(memcmp(string_data(str), "XHowdy!", 7));

    String *str2 = string_from_chars("0123456789abcdef");
    ASSERT_EQUAL(string_len(str2), 16);
    ASSERT_FALSE(memcmp(string_data(str2), "0123456789abcdef", 16));

    string_add_str(str, str2);
    ASSERT_EQUAL(string_len(str), 23);
    ASSERT_FALSE(memcmp(string_data(str), "XHowdy!0123456789abcdef", 23));

    ASSERT_EQUAL(strlen(string_get_c_str(str)), 23);
    ASSERT_FALSE(strcmp(string_get_c_str(str), "XHowdy!0123456789abcdef"));

    String *str3 = string_from_chars("abcdefghijklmnopqrstuvwxyz");
    
    String *sub1 = string_substr(str3, 0, 32);
    ASSERT_EQUAL(string_len(sub1), 26);
    ASSERT_FALSE(memcmp(string_data(sub1), "abcdefghijklmnopqrstuvwxyz", 26));

    String *sub2 = string_substr(str3, 10, 10);
    ASSERT_EQUAL(string_len(sub2), 10);
    ASSERT_FALSE(memcmp(string_data(sub2), "klmnopqrst", 10));

    free_string(str);
    free_string(str2);
    free_string(str3);
    free_string(sub1);
    free_string(sub2);

    return test_status();
}
