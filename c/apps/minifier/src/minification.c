#include "minifier/minification.h"

#include "glasstypes/glass-class.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "parser/parser.h"
#include "utils/copy-interface.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/set.h"
#include "utils/string.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct NameCount {
    String *name;

    int count;
} NameCount;

void *copy_name_count(const void *name_count) {
    NameCount *copy = malloc(sizeof(NameCount));
    copy->name = copy_string(((NameCount *) name_count)->name);
    copy->count = ((NameCount *) name_count)->count;
    return copy;
}

void free_name_count(void *name_count) {
    free_string(((NameCount *) name_count)->name);
    free(name_count);
}

const CopyInterface *NAME_COUNT_COPY_OPS = &(CopyInterface) {
    copy_name_count, free_name_count,
};

int compare_names_descending(const void *name1, const void *name2) {
    return ((NameCount *) name1)->count - ((NameCount *) name2)->count;
}

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

        const List *parents = class_get_parents(gclass);
        for (size_t i = 0; i < list_len(parents); i++) {
            const String *parent_name = list_get(parents, i);
            add_name(classes, name_counts, class_names, func_names, parent_name);
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

Set *get_fixed_names(const Map *name_counts) {
    List *empty_list = new_list(STRING_COPY_OPS);
    Map *builtins = classes_from_files(empty_list, true, false);
    Set *fixed_names = new_set(STRING_HASH_OPS);

    String *main_class_name = string_from_char('M');
    String *main_func_name = string_from_char('m');
    String *ctor_name = string_from_chars("c__");

    set_add(fixed_names, main_class_name);
    set_add(fixed_names, main_func_name);
    set_add(fixed_names, ctor_name);

    List *builtin_classes = map_get_keys(builtins);
    for (size_t i = 0; i < list_len(builtin_classes); i++) {
        const String *class_name = list_get(builtin_classes, i);
        set_add(fixed_names, class_name);

        if (map_has(name_counts, class_name)) {
            const GlassClass *builtin_class = map_get(builtins, class_name);
            List *func_names = class_get_func_names(builtin_class);

            for (size_t j = 0; j < list_len(func_names); j++) {
                const String *func_name = list_get(func_names, j);
                if (map_has(name_counts, func_name)) {
                    set_add(fixed_names, func_name);
                }
            }

            free_list(func_names);
        }
    }

    free_string(main_class_name);
    free_string(main_func_name);
    free_string(ctor_name);
    free_list(builtin_classes);
    free_list(empty_list);
    free_map(builtins);

    return fixed_names;
}

Map *reassign_names(const Map *name_counts) {
    Set *fixed_names = get_fixed_names(name_counts);

    List *sorted_names = new_list(NAME_COUNT_COPY_OPS);

    List *names_list = map_get_keys(name_counts);
    for (size_t i = 0; i < list_len(names_list); i++) {
        String *name = list_get_mutable(names_list, i);
        NameCount count = { name, *(int *) map_get(name_counts, name) };
        list_add(sorted_names, &count);
    }

    list_sort(sorted_names, compare_names_descending);

    Map *reassigned_names = new_map(STRING_HASH_OPS, STRING_COPY_OPS);

    String *local_name = string_from_chars("_`");
    String *classwide_name = string_from_char('`');
    String *global_name = string_from_char('@');

    for (size_t i = 0; i < list_len(sorted_names); i++) {
        const NameCount *name_count = list_get(sorted_names, i);
        const String *name = name_count->name;

        if (set_has(fixed_names, name)) {
            map_set(reassigned_names, name, name);
            continue;
        }

        String *reassigned_name = string_from_char('M');
        while (set_has(fixed_names, reassigned_name)) {
            free_string(reassigned_name);
            switch (get_name_scope(name)) {
                case NAME_LOCAL:
                    reassigned_name = inc_name(local_name, NAME_LOCAL);
                    free_string(local_name);
                    local_name = copy_string(reassigned_name);
                    break;
                
                case NAME_CLASSWIDE:
                    reassigned_name = inc_name(classwide_name, NAME_CLASSWIDE);
                    free_string(classwide_name);
                    classwide_name = copy_string(reassigned_name);
                    break;
                
                case NAME_GLOBAL:
                    reassigned_name = inc_name(global_name, NAME_GLOBAL);
                    free_string(global_name);
                    global_name = copy_string(reassigned_name);
                    break;
            }
        }
        map_set(reassigned_names, name, reassigned_name);
        free_string(reassigned_name);
    }

    free_list(sorted_names);
    free_set(fixed_names);

    return reassigned_names;
}

void add_name_to_source(String *source, const String *name, const Map *reassigned) {
    name = map_get(reassigned, name);
    if (string_len(name) == 1) {
        string_add_str(source, name);
    }
    else {
        string_add_char(source, '(');
        string_add_str(source, name);
        string_add_char(source, ')');
    }
}

String *minify_source(const Map *classes, const Map *reassigned_names) {
    String *minified = new_string();

    List *names_list = map_get_keys(reassigned_names);
    for (size_t i = 0; i < list_len(names_list); i++) {
        const String *class_name = list_get(names_list, i);
        if (!map_has(classes, class_name)) {
            continue;
        }
        const GlassClass *gclass = map_get(classes, class_name);
        string_add_char(minified, '{');
        add_name_to_source(minified, class_name, reassigned_names);

        for (size_t j = 0; j < list_len(names_list); j++) {
            const String *func_name = list_get(names_list, j);
            if (!class_has_func(gclass, func_name)) {
                continue;
            }
            const GlassFunction *func = class_get_func(gclass, func_name);
            string_add_char(minified, '[');
            add_name_to_source(minified, func_name, reassigned_names);

            for (size_t k = 0; k < func_len(func); k++) {
                const GlassCommand *cmd = func_get_command(func, k);
                if (cmd->type == CMD_PUSH_NAME) {
                    add_name_to_source(minified, cmd->str, reassigned_names);
                }
                else if (cmd->type == CMD_LOOP_BEGIN) {
                    string_add_char(minified, '/');
                    add_name_to_source(minified, cmd->str, reassigned_names);
                }
                else {
                    String *str = command_to_str(cmd);
                    string_add_str(minified, str);
                    free_string(str);
                }
            }

            string_add_char(minified, ']');
        }

        string_add_char(minified, '}');
    }
    free_list(names_list);

    return minified;
}

String *minify_glass_classes(const Map *classes) {
    Map *name_counts = get_reachable_names(classes);
    Map *name_assignments = reassign_names(name_counts);

    String *minified = minify_source(classes, name_assignments);

    free_map(name_counts);
    free_map(name_assignments);

    return minified;
}
