#include "interpreter/interpreter.h"
#include "glasstypes/glass-class.h"
#include "parser/parser.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/string.h"
#include "utils/stream.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct Options {
    List *files;

    List *args;
} Options;

void usage(const char *exe_name) {
    printf("Usage: %s <glass files> ... -- <program args>\n", exe_name);
}

bool parse_command_line(Options *opts, int argc, char **argv) {
    opts->files = new_list(STRING_COPY_OPS);
    opts->args = new_list(STRING_COPY_OPS);

    bool collecting_args = false;

    for (int i = 1; i < argc; i++) {
        if (!collecting_args && strcmp(argv[i], "--") == 0) {
            collecting_args = true;
            continue;
        }

        String *str = string_from_chars(argv[i]);

        if (collecting_args) {
            list_add(opts->args, str);
        }
        else if (strcmp(argv[i], "--help")) {
            usage(argv[i]);
            return true;
        }
        else {
            list_add(opts->files, str);
        }

        free_string(str);
    }

    if (list_len(opts->files) == 0) {
        usage(argv[0]);
        return true;
    }

    return false;
}

void free_options(Options *opts) {
    free_list(opts->files);
    free_list(opts->args);
}

int main(int argc, char **argv) {
    Options opts;

    if (parse_command_line(&opts, argc, argv)) {
        return 1;
    }

    Map *classes = classes_from_files(opts.files, true, true);
    if (classes == NULL) {
        return 1;
    }

    int ret_code = run_interpreter(classes, opts.args);
    free_options(&opts);
    free_map(classes);

    return ret_code;
}
