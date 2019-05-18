#include "interpreter/glass-instance.h"
#include "interpreter/glass-value.h"

#include "glasstypes/glass-class.h"
#include "glasstypes/glass-function.h"
#include "utils/map.h"
#include "utils/string.h"

#include <stdlib.h>

struct GlassInstance {
    const GlassClass *gclass;

    Map *vars;
};

GlassInstance *new_glass_instance(const GlassClass *gclass) {
    GlassInstance *inst = malloc(sizeof(GlassInstance));
    inst->gclass = gclass;
    inst->vars = new_map(STRING_HASH_OPS, VALUE_COPY_OPS);
    return inst;
}

bool instance_has_var(const GlassInstance *inst, const String *name) {
    return map_has(inst->vars, name);
}

bool instance_has_func(const GlassInstance *inst, const String *name) {
    return class_has_func(inst->gclass, name);
}

const GlassFunction *instance_get_func(const GlassInstance *inst, const String *name) {
    return class_get_func(inst->gclass, name);
}

const GlassValue *instance_get_var(const GlassInstance *inst, const String *name) {
    return map_get(inst->vars, name);
}

void instance_set_var(GlassInstance *inst, const String *name, const GlassValue *val) {
    map_set(inst->vars, name, val);
}
