#include "glasstypes/glass-class.h"
#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-function.h"
#include "utils/map.h"
#include "utils/string.h"

#include <stdlib.h>

struct GlassClass {
    String *name;

    Map *funcs;
};

struct GlassClassBuilder {
    GlassClass *gclass;
};

GlassClassBuilder *new_class_builder(const String *name) {
    GlassClassBuilder *builder = malloc(sizeof(GlassClassBuilder));
    builder->gclass = malloc(sizeof(GlassClass));
    builder->gclass->name = copy_string(name);
    builder->gclass->funcs = new_map(STRING_HASH_OPS, FUNC_COPY_OPS);
    return builder;
}

void free_glass_class(GlassClass *gclass) {
    free_string(gclass->name);
    free_map(gclass->funcs);
    free(gclass);
}

void free_class_builder(GlassClassBuilder *builder) {
    free_glass_class(builder->gclass);
    free(builder);
}

GlassClass *build_glass_class(const GlassClassBuilder *builder) {
    return copy_glass_class(builder->gclass);
}

void builder_add_func(GlassClassBuilder *builder, const GlassFunction *func) {
    const String *name = func_get_name(func);
    map_set(builder->gclass->funcs, name, func);
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
