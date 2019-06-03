#include "minifier/minification.h"

#include "glasstypes/glass-class.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "utils/copy-interface.h"
#include "utils/hash-interface.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/set.h"
#include "utils/string.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct NamePair {
    String *class_name, *method_name;
} NamePair;

void *copy_name_pair(const void *pair) {
    NamePair *copy = malloc(sizeof(NamePair));
    copy->class_name = ((NamePair *) pair)->class_name;
    copy->method_name = ((NamePair *) pair)->method_name;
    return copy;
}

void free_name_pair(void *pair) {
    NamePair *name_pair = pair;
    free_string(name_pair->class_name);
    free_string(name_pair->method_name);
    free(name_pair);
}

size_t hash_name_pair(const void *pair) {
    const NamePair *name_pair = pair;
    return hash_string(name_pair->class_name) ^ hash_string(name_pair->method_name);
}

bool name_pairs_equal(const void *pair1, const void *pair2) {
    const NamePair *names1 = pair1;
    const NamePair *names2 = pair2;

    return strings_equal(names1->class_name, names2->class_name) &&
           strings_equal(names1->method_name, names2->method_name);
}

const HashInterface *NAME_PAIR_HASH_OPS = &(HashInterface) {
    copy_name_pair, free_name_pair, hash_name_pair, name_pairs_equal,
};

typedef enum NameScope {
    NAME_LOCAL,
    NAME_CLASSWIDE,
    NAME_GLOBAL,
} NameScope;

NameScope get_name_scope(const String *name) {
    char c = string_get(name, 0);

    if (isupper(c)) {
        return NAME_GLOBAL;
    }
    else if (islower(c)) {
        return NAME_CLASSWIDE;
    }
    else {
        return NAME_LOCAL;
    }
}

String *inc_name(const String *name, NameScope scope) {
    String *new_name = copy_string(name);

    for (size_t i = 0; i < string_len(name) - 1; i++) {
        size_t index = string_len(name) - i - 1;
        char c = string_get(name, index);
        if (c == 'z') {
            string_set(new_name, index, 'A');
            return new_name;
        }
        else if (c == 'Z') {
            string_set(new_name, index, '0');
            return new_name;
        }
        else if (c == '9') {
            string_set(new_name, index, 'a');
        }
        else {
            string_set(new_name, index, c + 1);
            return new_name;
        }
    }

    // First character in the name, handled differently based on the scope
    // of the name
    if (scope == NAME_LOCAL) {
        string_add_char(new_name, 'a');
    }
    else if (scope == NAME_CLASSWIDE) {
        if (string_get(new_name, 0) == 'z') {
            string_set(new_name, 0, 'a');
            string_add_char(new_name, 'a');
        }
        else {
            string_set(new_name, 0, string_get(new_name, 0) + 1);
        }
    }
    else if (scope == NAME_GLOBAL) {
        if (string_get(new_name, 0) == 'Z') {
            string_set(new_name, 0, 'A');
            string_add_char(new_name, 'a');
        }
        else {
            string_set(new_name, 0, string_get(new_name, 0) + 1);
        }
    }

    return new_name;
}

void add_name(const Map *classes, Map *name_counts, List *class_names,
              List *func_names, const String *name);

void count_names_in_func(const Map *classes, Map *name_counts, List *class_names,
                         List *func_names, const GlassFunction *func)
{
    for (size_t i = 0; i < func_len(func); i++) {
        const GlassCommand *cmd = func_get_command(func, i);

        if (cmd->type == CMD_PUSH_NAME || cmd->type == CMD_LOOP_BEGIN) {
            add_name(classes, name_counts, class_names, func_names, cmd->str);
        }
    }
}

void add_name(const Map *classes, Map *name_counts, List *class_names,
              List *func_names, const String *name)
{
    if (map_has(name_counts, name)) {
        int *count = map_get_mutable(name_counts, name);
        *count += 1;
        return;
    }

    int count = 1;
    map_set(name_counts, name, &count);

    NameScope scope = get_name_scope(name);

    if (scope == NAME_CLASSWIDE) {
        list_add(func_names, name);

        List *class_name_copy = copy_list(class_names);

        for (size_t i = 0; i < list_len(class_name_copy); i++) {
            const String *class_name = list_get(class_name_copy, i);
            const GlassClass *gclass = map_get(classes, class_name);           

            if (class_has_func(gclass, name)) {
                const GlassFunction *func = class_get_func(gclass, name);
                count_names_in_func(classes, name_counts, class_names, func_names, func);
            }
        }

        free_list(class_name_copy);
    }
    else if (map_has(classes, name)) {
        list_add(class_names, name);

        List *func_name_copy = copy_list(func_names);
        const GlassClass *gclass = map_get(classes, name);

        for (size_t i = 0; i < list_len(func_name_copy); i++) {
            const String *func_name = list_get(func_name_copy, i);

            if (class_has_func(gclass, func_name)) {
                const GlassFunction *func = class_get_func(gclass, func_name);
                count_names_in_func(classes, name_counts, class_names, func_names, func);
            }
        }

        free_list(func_name_copy);
    }
}

Map *get_reachable_names(const Map *classes) {
    Map *name_counts = new_map(STRING_HASH_OPS, INT_COPY_OPS);
    List *class_names = new_list(STRING_COPY_OPS);
    List *func_names = new_list(STRING_COPY_OPS);

    String *main_class_name = string_from_char('M');
    String *main_func_name = string_from_char('m');
    String *ctor_name = string_from_chars("c__");

    add_name(classes, name_counts, class_names, func_names, ctor_name);
    add_name(classes, name_counts, class_names, func_names, main_func_name);
    add_name(classes, name_counts, class_names, func_names, main_class_name);

    free_string(main_class_name);
    free_string(main_func_name);
    free_list(class_names);
    free_list(func_names);

    return name_counts;
}

String *minify_glass_classes(const Map *classes) {
    Map *name_counts = get_reachable_names(classes);
    List *names = map_get_keys(name_counts);

    for (size_t i = 0; i < list_len(names); i++) {
        String *name = list_get_mutable(names, i);
        printf("%s: %d\n", string_get_c_str(name), *(int *) map_get(name_counts, name));
    }

    free_map(name_counts);
    free_list(names);

    return NULL;
}
