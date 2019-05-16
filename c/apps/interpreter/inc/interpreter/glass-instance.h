#ifndef INTERPRETER_GLASS_INSTANCE_H
#define INTERPRETER_GLASS_INSTANCE_H

#include <stdbool.h>

typedef struct GlassInstance GlassInstance;
struct GlassClass;
struct GlassValue;
struct String;

GlassInstance *new_glass_instance(const struct GlassClass *gclass);

bool glass_has_var(const GlassInstance *inst, const struct String *name);

const struct GlassValue *glass_get_var(const GlassInstance *inst, const struct String *name);

void glass_set_var(GlassInstance *inst, const struct String *name, const struct GlassValue *val);

#endif
