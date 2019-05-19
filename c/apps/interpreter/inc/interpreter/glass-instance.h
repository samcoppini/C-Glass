#ifndef INTERPRETER_GLASS_INSTANCE_H
#define INTERPRETER_GLASS_INSTANCE_H

#include <stdbool.h>

typedef struct GlassInstance GlassInstance;
struct GlassClass;
struct GlassFunction;
struct GlassValue;
struct String;

GlassInstance *new_glass_instance(const struct GlassClass *gclass);

bool instance_has_var(const GlassInstance *inst, const struct String *name);

bool instance_has_func(const GlassInstance *inst, const struct String *name);

const struct GlassFunction *instance_get_func(const GlassInstance *inst, const struct String *name);

const struct GlassValue *instance_get_var(const GlassInstance *inst, const struct String *name);

const struct GlassClass *instance_get_class(const GlassInstance *inst);

void instance_set_var(GlassInstance *inst, const struct String *name, const struct GlassValue *val);

#endif
