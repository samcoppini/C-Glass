#ifndef INTERPRETER_GLASS_INSTANCE_H
#define INTERPRETER_GLASS_INSTANCE_H

#include <stdbool.h>
#include <stddef.h>

typedef size_t GlassInstance;
struct GlassClass;
struct GlassFunction;
struct GlassValue;
struct Map;
struct String;

void init_instances(const struct Map *globals);

void free_instances(void);

void register_new_scope(const struct Map *local_vars, GlassInstance inst);

void exit_scope(void);

GlassInstance new_glass_instance(const struct GlassClass *gclass);

GlassInstance copy_glass_instance(GlassInstance inst);

void release_glass_instance(GlassInstance inst);

bool instance_has_var(const GlassInstance inst, const struct String *name);

bool instance_has_func(const GlassInstance inst, const struct String *name);

const struct GlassFunction *instance_get_func(const GlassInstance inst, const struct String *name);

const struct GlassValue *instance_get_var(const GlassInstance inst, const struct String *name);

const struct GlassClass *instance_get_class(const GlassInstance inst);

void instance_set_var(GlassInstance inst, const struct String *name, const struct GlassValue *val);

#endif
