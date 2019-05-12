#ifndef GLASSTYPES_GLASS_BUILDERS_H
#define GLASSTYPES_GLASS_BUILDERS_H

#include <stdbool.h>

typedef struct GlassClassBuilder GlassClassBuilder;
typedef struct GlassFuncBuilder GlassFuncBuilder;
struct GlassClass;
struct GlassCommand;
struct GlassFunction;
struct String;

GlassClassBuilder *new_class_builder(const struct String *name);

GlassFuncBuilder *new_func_builder(const struct String *name);

void free_class_builder(GlassClassBuilder *builder);

void free_func_builder(GlassFuncBuilder *builder);

struct GlassClass *build_glass_class(const GlassClassBuilder *builder);

struct GlassFunction *build_glass_function(const GlassFuncBuilder *builder);

void builder_add_func(GlassClassBuilder *builder, const struct GlassFunction *func);

void builder_add_command(GlassFuncBuilder *builder, const struct GlassCommand *cmd);

void builder_start_loop(GlassFuncBuilder *builder, const struct String *name);

bool builder_end_loop(GlassFuncBuilder *builder);

#endif
