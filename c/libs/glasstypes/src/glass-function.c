#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "utils/copy-interface.h"
#include "utils/list.h"
#include "utils/string.h"

#include <stdlib.h>

struct GlassFunction {
    String *name;

    List *cmds;
};

struct GlassFuncBuilder {
    String *name;

    List *cmds;

    List *loop_starts;
};

GlassFuncBuilder *new_func_builder(const String *name) {
    GlassFuncBuilder *builder = malloc(sizeof(GlassFuncBuilder));
    builder->name = copy_string(name);
    builder->cmds = new_list(CMD_COPY_OPS);
    builder->loop_starts = new_list(SIZE_T_COPY_OPS);
    return builder;
}

GlassFunction *build_glass_function(const GlassFuncBuilder *builder) {
    if (!list_empty(builder->loop_starts)) {
        return NULL;
    }

    GlassFunction *func = malloc(sizeof(GlassFunction));
    func->name = copy_string(builder->name);
    func->cmds = copy_list(builder->cmds);
    return func;
}

GlassFunction *copy_glass_func(const GlassFunction *func) {
    GlassFunction *copy = malloc(sizeof(GlassFunction));
    copy->name = copy_string(func->name);
    copy->cmds = copy_list(func->cmds);
    return copy;
}

void free_glass_func(GlassFunction *func) {
    free_string(func->name);
    free_list(func->cmds);
    free(func);
}

void free_func_builder(GlassFuncBuilder *builder) {
    free_string(builder->name);
    free_list(builder->cmds);
    free_list(builder->loop_starts);
    free(builder);
}

const String *func_get_name(const GlassFunction *func) {
    return func->name;
}

const GlassCommand *func_get_command(const GlassFunction *func, size_t index) {
    return list_get(func->cmds, index);
}

void builder_add_command(GlassFuncBuilder *builder, const GlassCommand *cmd) {
    list_add(builder->cmds, cmd);
}

void builder_start_loop(GlassFuncBuilder *builder, const String *name) {
    const GlassCommand cmd = { CMD_LOOP_BEGIN, .str = (String *) name };
    size_t index = list_len(builder->cmds);
    list_add(builder->loop_starts, &index);
    list_add(builder->cmds, &cmd);
}

bool builder_end_loop(GlassFuncBuilder *builder) {
    if (list_empty(builder->loop_starts)) {
        return true;
    }
    size_t *index = list_pop(builder->loop_starts);
    GlassCommand *loop_start = list_get_mutable(builder->cmds, *index);
    loop_start->index = list_len(builder->cmds);
    GlassCommand loop_end = { CMD_LOOP_END, .str = loop_start->str, .index = *index };
    list_add(builder->cmds, &loop_end);
    free(index);
    return false;
}

static void *copy_glass_func_generic(const void *func) {
    return copy_glass_func(func);
}

static void free_glass_func_generic(void *func) {
    free_glass_func(func);
}

const CopyInterface *FUNC_COPY_OPS = &(CopyInterface) {
    copy_glass_func_generic,
    free_glass_func_generic,
};
