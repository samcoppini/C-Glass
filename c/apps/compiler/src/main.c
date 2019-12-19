#include "compiler/compiler.h"
#include "parser/parser.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/string.h"

#include <stdio.h>
#include <string.h>

typedef struct Options {
    List *files;

    String *out_name;
} Options;

void usage(const char *exe_name) {
    printf("Usage: %s <glass files...> [--out out_file]\n", exe_name);
}

void free_options(Options *opts) {
    free_list(opts->files);
    free_string(opts->out_name);
}

bool parse_command_line(Options *opts, int argc, char **argv) {
    opts->files = new_list(STRING_COPY_OPS);
    opts->out_name = new_string();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            free_options(opts);
            usage(argv[0]);
            return true;
        }
        else if (strcmp(argv[i], "--out") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "--out must be followed by a filename.\n");
                free_options(opts);
                return true;
            }
            free_string(opts->out_name);
            opts->out_name = string_from_chars(argv[i + 1]);
            i++;
        }
        else {
            String *arg = string_from_chars(argv[i]);
            list_add(opts->files, arg);
            free_string(arg);
        }
    }

    if (list_empty(opts->files)) {
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

    Map *classes = classes_from_files(opts.files, true, true);
    String *compiled = compile_classes(classes);

    if (string_len(opts.out_name) > 0) {
        const char *filename = string_get_c_str(opts.out_name);
        FILE *fp = fopen(filename, "w");

        if (fp == NULL) {
            fprintf(stderr, "Unable to open %s!\n", filename);
            free_string(compiled);
            free_map(classes);
            free_options(&opts);
            return 1; 
        }

        const char *compiled_str = string_data(compiled);
        fwrite(compiled_str, sizeof(char), string_len(compiled), fp);

        fclose(fp);
    }
    else {
        for (size_t i = 0; i < string_len(compiled); i++) {
            putchar(string_get(compiled, i));
        }
    }

    free_string(compiled);
    free_map(classes);
    free_options(&opts);

    return 0;
}
