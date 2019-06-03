#include "glasstypes/glass-class.h"
#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-function.h"
#include "utils/copy-interface.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/string.h"

#include <stdlib.h>
#include <stdio.h>

struct GlassClass {
    String *name;

    Map *funcs;

    List *parents;

    String *filename;

    unsigned line, col;
};

enum InheritanceState {
    INHERITANCE_UNHANDLED,
    INHERITANCE_HANDLING,
    INHERITANCE_HANDLED,
};

struct GlassClassBuilder {
    String *name;

    List *parents;

    enum InheritanceState inheritance;

    List *funcs;

    String *filename;

    unsigned line, col;
};

GlassClass *copy_glass_class(const GlassClass *gclass) {
    GlassClass *copy = malloc(sizeof(GlassClass));
    copy->name = copy_string(gclass->name);
    copy->funcs = copy_map(gclass->funcs);
    copy->parents = copy_list(gclass->parents);
    copy->filename = copy_string(gclass->filename);
    copy->line = gclass->line;
    copy->col = gclass->col;
    return copy;
}

GlassClassBuilder *new_class_builder(const String *name, const String *filename,
                                     unsigned line, unsigned col)
{
    GlassClassBuilder *builder = malloc(sizeof(GlassClassBuilder));
    builder->name = copy_string(name);
    builder->parents = new_list(STRING_COPY_OPS);
    builder->inheritance = INHERITANCE_UNHANDLED;
    builder->funcs = new_list(FUNC_COPY_OPS);
    builder->filename = copy_string(filename);
    builder->line = line;
    builder->col = col;
    return builder;
}

GlassClassBuilder *copy_class_builder(const GlassClassBuilder *builder) {
    GlassClassBuilder *copy = malloc(sizeof(GlassClassBuilder));
    copy->name = copy_string(builder->name);
    copy->parents = copy_list(builder->parents);
    copy->inheritance = builder->inheritance;
    copy->funcs = copy_list(builder->funcs);
    copy->filename = copy_string(builder->filename);
    copy->line = builder->line;
    copy->col = builder->col;
    return copy;
}

void free_glass_class(GlassClass *gclass) {
    free_string(gclass->name);
    free_map(gclass->funcs);
    free(gclass);
}

void free_class_builder(GlassClassBuilder *builder) {
    free_string(builder->name);
    free_list(builder->parents);
    free_list(builder->funcs);
    free(builder);
}

GlassClass *build_glass_class(const GlassClassBuilder *builder) {
    Map *func_map = new_map(STRING_HASH_OPS, FUNC_COPY_OPS);
    Map *unique_funcs = new_map(STRING_HASH_OPS, LIST_COPY_OPS);

    for (size_t i = 0; i < list_len(builder->funcs); i++) {
        const GlassFunction *func = list_get(builder->funcs, i);
        const String *func_name = func_get_name(func);

        if (!map_has(unique_funcs, func_name)) {
            List *idx_list = new_list(SIZE_T_COPY_OPS);
            list_add(idx_list, &i);

            map_set(unique_funcs, func_name, idx_list);
            map_set(func_map, func_name, func);
            
            free_list(idx_list);
        }
        else {
            List *idx_list = map_get_mutable(unique_funcs, func_name);
            list_add(idx_list, &i);
        }
    }

    if (map_size(unique_funcs) != list_len(builder->funcs)) {
        List *funcs = map_get_keys(unique_funcs);

        fprintf(stderr, "Error in %s class!\n", string_get_c_str(builder->name));

        for (size_t i = 0; i < list_len(funcs); i++) {
            String *func_name = copy_string(list_get(funcs, i));
            const List *func_list = map_get(unique_funcs, func_name);

            if (list_len(func_list) > 1) {
                fprintf(stderr, "    Function '%s' defined multiple times:\n", string_get_c_str(func_name));
                String *filename = copy_string(builder->filename);

                for (size_t j = 0; j < list_len(func_list); j++) {
                    const size_t *idx = list_get(func_list, j);
                    const GlassFunction *func = list_get(builder->funcs, *idx);

                    fprintf(stderr, "        Defined in '%s' on line %d, column %d\n",
                                    string_get_c_str(filename),
                                    func_get_line(func),
                                    func_get_col(func));
                }
            }

            free_string(func_name);
        }

        free_list(funcs);
        free_map(func_map);
        free_map(unique_funcs);
        return NULL;
    }

    GlassClass *gclass = malloc(sizeof(GlassClass));
    gclass->filename = copy_string(builder->filename);
    gclass->name = copy_string(builder->name);
    gclass->funcs = func_map;
    gclass->parents = copy_list(builder->parents);
    gclass->line = builder->line;
    gclass->col = builder->col;

    free_map(unique_funcs);

    return gclass;
}

void builder_add_func(GlassClassBuilder *builder, const GlassFunction *func) {
    list_add(builder->funcs, func);
}

void builder_add_parent(GlassClassBuilder *builder, const String *name) {
    list_add(builder->parents, name);
}

const String *class_get_name(const GlassClass *gclass) {
    return gclass->name;
}

bool class_has_func(const GlassClass *gclass, const String *name) {
    return map_has(gclass->funcs, name);
}

const GlassFunction *class_get_func(const GlassClass *gclass, const String *name) {
    return map_get(gclass->funcs, name);
}

static void *copy_glass_class_generic(const void *gclass) {
    return copy_glass_class(gclass);
}

static void free_glass_class_generic(void *gclass) {
    free_glass_class(gclass);
}

static void *copy_class_builder_generic(const void *builder) {
    return copy_class_builder(builder);
}

static void free_class_builder_generic(void *builder) {
    free_class_builder(builder);
}

const CopyInterface *CLASS_COPY_OPS = &(CopyInterface) {
    copy_glass_class_generic,
    free_glass_class_generic,
};

const CopyInterface *CLASS_BUILDER_COPY_OPS = &(CopyInterface) {
    copy_class_builder_generic,
    free_class_builder_generic,
};
