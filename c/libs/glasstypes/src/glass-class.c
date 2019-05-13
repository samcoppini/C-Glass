#include "glasstypes/glass-class.h"
#include "glasstypes/glass-builders.h"
#include "glasstypes/glass-function.h"
#include "utils/copy-interface.h"
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

GlassClass *copy_glass_class(const GlassClass *gclass) {
    GlassClass *copy = malloc(sizeof(GlassClass));
    copy->name = copy_string(gclass->name);
    copy->funcs = copy_map(gclass->funcs);
    return copy;
}

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

bool builder_add_func(GlassClassBuilder *builder, const GlassFunction *func) {
    const String *name = func_get_name(func);
    if (map_has(builder->gclass->funcs, name)) {
        return true;
    }
    map_set(builder->gclass->funcs, name, func);
    return false;
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

const CopyInterface *CLASS_COPY_OPS = &(CopyInterface) {
    copy_glass_class_generic,
    free_glass_class_generic,
};
