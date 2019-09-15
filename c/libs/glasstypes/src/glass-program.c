#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-class.h"
#include "glasstypes/glass-function.h"

#include "utils/copy-interface.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/string.h"

#include <stdio.h>
#include <stdlib.h>

enum InheritanceState {
    INHERITANCE_UNHANDLED,
    INHERITANCE_HANDLING,
    INHERITANCE_HANDLED,
};

struct GlassClassBuilder {
    String *name;

    List *parents;

    enum InheritanceState state;

    List *funcs;

    String *filename;

    unsigned line, col;
};

struct GlassProgramBuilder {
    List *classes;
};

GlassProgramBuilder *new_program_builder(void) {
    GlassProgramBuilder *builder = malloc(sizeof(GlassProgramBuilder));
    builder->classes = new_list(CLASS_BUILDER_COPY_OPS);
    return builder;
}

void free_program_builder(GlassProgramBuilder *builder) {
    free_list(builder->classes);
    free(builder);
}

void builder_add_class(GlassProgramBuilder *builder, const GlassClassBuilder *class_builder) {
    list_add(builder->classes, class_builder);
}

bool func_matches_name(const void *str, const void *val) {
    const String *func_name = (const String *) str;
    const GlassFunction *func = (const GlassFunction *) val;
    return strings_equal(func_get_name(func), func_name);
}

bool resolve_inheritance(Map *class_builders, const String *class_name, List *class_chain) {
    GlassClassBuilder *builder = map_get_mutable(class_builders, class_name);
    
    if (builder == NULL) {
        fprintf(stderr, "Error! Cannot inherit from non existent class!\n");
        return true;
    }
    else if (builder->state == INHERITANCE_HANDLED) {
        return false;
    }
    else if (builder->state == INHERITANCE_HANDLING) {
        fprintf(stderr, "Error! Inheritance cycle detected!\n");
        return true;
    }
    else if (list_len(builder->parents) == 0) {
        return false;
    }
    else {
        builder->state = INHERITANCE_HANDLING;
        list_add(class_chain, class_name);

        for (size_t i = 0; i < list_len(builder->parents); i++) {
            String *parent_name = list_get_mutable(builder->parents, i);

            // Make sure we don't inherit from the same parent twice
            for (size_t j = 0; j < i; j++) {
                const String *other_parent_name = list_get(builder->parents, j);
                if (strings_equal(parent_name, other_parent_name)) {
                    fprintf(stderr, "Error! %s inherits from %s multiple times!\n",
                                    string_get_c_str(builder->name),
                                    string_get_c_str(parent_name));
                    return true;
                }
            }

            if (resolve_inheritance(class_builders, parent_name, class_chain)) {
                return true;
            }

            const GlassClassBuilder *parent = map_get(class_builders, parent_name);

            for (size_t j = 0; j < list_len(parent->funcs); j++) {
                const GlassFunction *func = list_get(parent->funcs, j);
                const String *func_name = func_get_name(func);

                if (!list_has(builder->funcs, func_name, func_matches_name)) {
                    list_add(builder->funcs, func);
                }
            }
        }

        builder->state = INHERITANCE_HANDLED;
        free_string(list_pop(builder->parents));
    }

    return false;
}

Map *build_classes(Map *builders_map, bool handle_inheritance) {
    Map *classes = new_map(STRING_HASH_OPS, CLASS_COPY_OPS);
    List *class_names = map_get_keys(builders_map);
    List *parent_chain = new_list(STRING_COPY_OPS);

    for (size_t i = 0; i < list_len(class_names); i++) {
        const String *class_name = list_get(class_names, i);

        if (handle_inheritance &&
            resolve_inheritance(builders_map, class_name, parent_chain))
        {
            free_map(classes);
            free_list(class_names);
            free_list(parent_chain);
            return NULL;
        }

        const GlassClassBuilder *builder = map_get(builders_map, class_name);

        GlassClass *gclass = build_glass_class(builder);
        if (gclass == NULL) {
            return NULL;
        }

        map_set(classes, class_name, gclass);
        free_glass_class(gclass);
    }

    return classes;
}

Map *build_glass_program(const GlassProgramBuilder *builder, bool handle_inheritance) {
    Map *builders_map = new_map(STRING_HASH_OPS, CLASS_BUILDER_COPY_OPS);
    Map *unique_classes = new_map(STRING_HASH_OPS, LIST_COPY_OPS);

    for (size_t i = 0; i < list_len(builder->classes); i++) {
        const GlassClassBuilder *gclass = list_get(builder->classes, i);
        const String *class_name = gclass->name;

        if (!map_has(unique_classes, class_name)) {
            List *idx_list = new_list(SIZE_T_COPY_OPS);
            list_add(idx_list, &i);

            map_set(builders_map, class_name, gclass);
            map_set(unique_classes, class_name, idx_list);

            free_list(idx_list);
        }
        else {
            List *idx_list = map_get_mutable(unique_classes, class_name);
            list_add(idx_list, &i);
        }
    }

    if (map_size(unique_classes) != list_len(builder->classes)) {
        List *class_names = map_get_keys(unique_classes);

        for (size_t i = 0; i < list_len(class_names); i++) {
            String *class_name = copy_string(list_get(class_names, i));
            const List *class_list = map_get(unique_classes, class_name);

            if (list_len(class_list) > 1) {
                fprintf(stderr, "Error! Class %s defined multiple times:\n",
                                string_get_c_str(class_name));

                for (size_t j = 0; j < list_len(class_list); j++) {
                    const size_t *idx = list_get(class_list, j);
                    const GlassClassBuilder *gclass = list_get(builder->classes, *idx);
                    String *filename = copy_string(gclass->filename);

                    fprintf(stderr, "    Defined in '%s' on line %d, column %d\n",
                                    string_get_c_str(filename),
                                    gclass->line, gclass->col);

                    free_string(filename);
                }
            }

            free_string(class_name);
        }
        
        free_list(class_names);
        free_map(unique_classes);
        free_map(builders_map);
        return NULL;
    }

    free_map(unique_classes);
    Map *classes = build_classes(builders_map, handle_inheritance);
    free_map(builders_map);

    return classes;
}
