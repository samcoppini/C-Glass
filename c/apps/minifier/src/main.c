#include "minifier/minification.h"
#include "parser/parser.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/string.h"

#include <stdio.h>
#include <string.h>

typedef struct Options {
    List *files;
} Options;

void usage(const char *exe_name) {
    printf("Usage: %s <glass files...>\n", exe_name);
}

void free_options(Options *opts) {
    free_list(opts->files);
}

bool parse_command_line(Options *opts, int argc, char **argv) {
    opts->files = new_list(STRING_COPY_OPS);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            free_options(opts);
            usage(argv[0]);
            return true;
        }

        String *arg = string_from_chars(argv[i]);

        list_add(opts->files, arg);

        free_string(arg);
    }

    if (list_len(opts->files) == 0) {
        free_options(opts);
        usage(argv[0]);
        return true;
    }

    return false;
}

int main(int argc, char **argv) {
    Options opts;

    if (parse_command_line(&opts, argc, argv)) {
        return 1;
    }

    Map *classes = classes_from_files(opts.files, false, false);
    if (classes == NULL) {
        return 1;
    }

    String *source = minify_glass_classes(classes);
    for (size_t i = 0; i < string_len(source); i++) {
        putchar(string_get(source, i));
    }

    return 0;
}
