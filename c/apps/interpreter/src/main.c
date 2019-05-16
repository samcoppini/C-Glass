#include "interpreter/interpreter.h"
#include "glasstypes/glass-class.h"
#include "parser/parser.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/string.h"
#include "utils/stream.h"

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

void free_options(Options *opts) {
    free_list(opts->files);
}

int main(int argc, char **argv) {
    Options opts;

    if (parse_command_line(&opts, argc, argv)) {
        return 1;
    }

    Map *classes = classes_from_files(opts.files, true);
    if (classes == NULL) {
        return 1;
    }

    int ret_code = run_interpreter(classes);
    free_options(&opts);
    free_map(classes);

    return ret_code;
}
