#include "interpreter/glass-instance.h"
#include "interpreter/glass-value.h"

#include "glasstypes/glass-class.h"
#include "glasstypes/glass-function.h"
#include "utils/map.h"
#include "utils/string.h"

#include <stdlib.h>
#include <string.h>

typedef struct GlassInstImpl {
    const GlassClass *gclass;

    Map *vars;

    unsigned ref_count;
} GlassInstImpl;

static GlassInstImpl *inst_array;
static size_t cur_inst;
static size_t used_insts;
static size_t alloc_insts;

#define INIT_ALLOC_INSTS 1024

void init_instances(void) {
    inst_array = calloc(INIT_ALLOC_INSTS, sizeof(GlassInstImpl));
    alloc_insts = INIT_ALLOC_INSTS;
    cur_inst = 0;
    used_insts = 0;
}

void free_instances(void) {
    for (size_t i = 0; i < alloc_insts; i++) {
        if (inst_array[i].ref_count > 0) {
            free_map(inst_array[i].vars);
        }
    }
}

static void do_garbage_collection(void) {
    if (used_insts > alloc_insts / 2) {
        GlassInstImpl *new_insts = calloc(alloc_insts * 2, sizeof(GlassInstImpl));
        memcpy(new_insts, inst_array, sizeof(GlassInstImpl) * alloc_insts);
        free(inst_array);
        inst_array = new_insts;
        alloc_insts *= 2;
    }
    cur_inst = 0;
}

static size_t get_free_inst_index(void) {
    while (cur_inst < alloc_insts && inst_array[cur_inst].ref_count > 0) {
        cur_inst++;
    }
    if (cur_inst < alloc_insts) {
        return cur_inst;
    }
    do_garbage_collection();
    return get_free_inst_index();
}

GlassInstance new_glass_instance(const GlassClass *gclass) {
    size_t index = get_free_inst_index();
    GlassInstImpl *inst = &inst_array[index];
    inst->gclass = gclass;
    inst->vars = new_map(STRING_HASH_OPS, VALUE_COPY_OPS);
    inst->ref_count = 1;
    used_insts++;
    return index;
}

GlassInstance copy_glass_instance(GlassInstance inst) {
    inst_array[inst].ref_count++;
    return inst;
}

void release_glass_instance(GlassInstance inst) {
    inst_array[inst].ref_count--;
    if (inst_array[inst].ref_count == 0) {
        free_map(inst_array[inst].vars);
        used_insts--;
    }
}

bool instance_has_var(const GlassInstance inst, const String *name) {
    return map_has(inst_array[inst].vars, name);
}

bool instance_has_func(const GlassInstance inst, const String *name) {
    return class_has_func(inst_array[inst].gclass, name);
}

const GlassFunction *instance_get_func(const GlassInstance inst, const String *name) {
    return class_get_func(inst_array[inst].gclass, name);
}

const GlassValue *instance_get_var(const GlassInstance inst, const String *name) {
    return map_get(inst_array[inst].vars, name);
}

const GlassClass *instance_get_class(const GlassInstance inst) {
    return inst_array[inst].gclass;
}

void instance_set_var(GlassInstance inst, const String *name, const GlassValue *val) {
    map_set(inst_array[inst].vars, name, val);
}
