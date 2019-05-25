#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "utils/copy-interface.h"
#include "utils/list.h"
#include "utils/string.h"

#include <assert.h>
#include <stdlib.h>

struct GlassFunction {
    String *name;

    List *cmds;

    String *filename;

    unsigned line, col;
};

struct GlassFuncBuilder {
    String *name;

    List *cmds;

    List *loop_starts;

    String *filename;

    unsigned line, col;
};

GlassFuncBuilder *new_func_builder(const struct String *name,
                                   const struct String *filename,
                                   unsigned line, unsigned col)
{
    GlassFuncBuilder *builder = malloc(sizeof(GlassFuncBuilder));
    builder->name = copy_string(name);
    builder->cmds = new_list(CMD_COPY_OPS);
    builder->loop_starts = new_list(SIZE_T_COPY_OPS);
    builder->filename = copy_string(filename);
    builder->line = line;
    builder->col = col;
    return builder;
}

GlassFunction *build_glass_function(const GlassFuncBuilder *builder) {
    if (!list_empty(builder->loop_starts)) {
        return NULL;
    }

    GlassFunction *func = malloc(sizeof(GlassFunction));
    func->name = copy_string(builder->name);
    func->cmds = copy_list(builder->cmds);
    func->filename = copy_string(builder->filename);
    func->line = builder->line;
    func->col = builder->col;
    return func;
}

GlassFunction *copy_glass_func(const GlassFunction *func) {
    GlassFunction *copy = malloc(sizeof(GlassFunction));
    copy->name = copy_string(func->name);
    copy->cmds = copy_list(func->cmds);
    copy->filename = copy_string(func->filename);
    copy->line = func->line;
    copy->col = func->col;
    return copy;
}

void free_glass_func(GlassFunction *func) {
    free_string(func->name);
    free_string(func->filename);
    free_list(func->cmds);
    free(func);
}

void free_func_builder(GlassFuncBuilder *builder) {
    free_string(builder->name);
    free_string(builder->filename);
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

size_t func_len(const GlassFunction *func) {
    return list_len(func->cmds);
}

static void builder_start_loop(GlassFuncBuilder *builder, const GlassCommand *cmd) {
    size_t index = list_len(builder->cmds);
    list_add(builder->loop_starts, &index);
    list_add(builder->cmds, cmd);
}

static bool builder_end_loop(GlassFuncBuilder *builder, const GlassCommand *cmd) {
    if (list_empty(builder->loop_starts)) {
        return true;
    }

    size_t *index = list_pop(builder->loop_starts);

    GlassCommand *loop_start = list_get_mutable(builder->cmds, *index);
    
    assert(loop_start->type == CMD_LOOP_BEGIN);
    
    GlassCommand *loop_end = &(GlassCommand) {
        .type = CMD_LOOP_END,
        .filename = copy_string(cmd->filename),
        .line = cmd->line,
        .col = cmd->col,
        .str = copy_string(loop_start->str),
        .index = *index,
    };

    loop_start->index = list_len(builder->cmds);
    list_add(builder->cmds, loop_end);

    free_string(loop_end->filename);
    free_string(loop_end->str);
    free(index);

    return false;
}

bool builder_add_command(GlassFuncBuilder *builder, const GlassCommand *cmd) {
    if (cmd->type == CMD_LOOP_BEGIN) {
        builder_start_loop(builder, cmd);
    }
    else if (cmd->type == CMD_LOOP_END) {
        return builder_end_loop(builder, cmd);
    }
    else {
        list_add(builder->cmds, cmd);
    }

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
