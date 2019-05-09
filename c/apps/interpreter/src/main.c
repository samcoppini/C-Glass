#include "utils/list.h"
#include "utils/string.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct Options {
    List *files;
} Options;

bool parse_command_line(Options *opts, int argc, char **argv) {
    opts->files = new_list(STRING_COPY_OPS);

    for (int i = 1; i < argc; i++) {
        String *str = string_from_chars(argv[i]);

        list_add(opts->files, str);

        free_string(str);
    }

    return false;
}

int main(int argc, char **argv) {
    Options opts;

    if (parse_command_line(&opts, argc, argv)) {
        return 1;
    }

    String *all_code = new_string();
    for (size_t i = 0; i < list_len(opts.files); i++) {
        String *filename = list_get_mutable(opts.files, i);
        FILE *fp = fopen(string_get_c_str(filename), "r");
        if (fp == NULL) {
            fprintf(stderr, "Error! Unable to open '%s'\n",
                    string_get_c_str(filename));
            return 1;
        }

        int c;
        while ((c = fgetc(fp)) != EOF) {
            string_add_char(all_code, c);
        }
    }

    puts(string_get_c_str(all_code));

    return 0;
}
