#include "interpreter/glass-instance.h"
#include "interpreter/glass-value.h"

#include "glasstypes/glass-class.h"
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

bool glass_has_var(const GlassInstance *inst, const String *name) {
    return map_has(inst->vars, name);
}

const GlassValue *glass_get_var(const GlassInstance *inst, const String *name) {
    return map_get(inst->vars, name);
}

void glass_set_var(GlassInstance *inst, const String *name, const GlassValue *val) {
    map_set(inst->vars, name, val);
}
