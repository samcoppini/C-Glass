#ifndef GLASSTYPES_GLASS_BUILDERS_H
#define GLASSTYPES_GLASS_BUILDERS_H

#include <stdbool.h>

typedef struct GlassClassBuilder GlassClassBuilder;
typedef struct GlassFuncBuilder GlassFuncBuilder;
typedef struct GlassProgramBuilder GlassProgramBuilder;
struct CopyInterface;
struct GlassClass;
struct GlassCommand;
struct GlassFunction;
struct Map;
struct String;

extern const struct CopyInterface *CLASS_BUILDER_COPY_OPS;

GlassClassBuilder *new_class_builder(const struct String *name,
                                     const struct String *filename,
                                     unsigned line, unsigned col);

GlassFuncBuilder *new_func_builder(const struct String *name,
                                   const struct String *filename,
                                   unsigned line, unsigned col);

GlassProgramBuilder *new_program_builder(void);

void free_class_builder(GlassClassBuilder *builder);

void free_func_builder(GlassFuncBuilder *builder);

void free_program_builder(GlassProgramBuilder *builder);

struct GlassClass *build_glass_class(const GlassClassBuilder *builder);

struct GlassFunction *build_glass_function(const GlassFuncBuilder *builder);

struct Map *build_glass_program(const GlassProgramBuilder *builder, bool handle_inheritance);

void builder_add_class(GlassProgramBuilder *builder, const GlassClassBuilder *free_class_builder);

void builder_add_func(GlassClassBuilder *builder, const struct GlassFunction *func);

void builder_add_parent(GlassClassBuilder *builder, const struct String *name);

bool builder_add_command(GlassFuncBuilder *builder, const struct GlassCommand *cmd);

void add_builtin_classes(GlassProgramBuilder *prog_builder);

#endif
