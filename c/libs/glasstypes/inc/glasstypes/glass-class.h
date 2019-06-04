#ifndef GLASSTYPES_GLASS_CLASS_H
#define GLASSTYPES_GLASS_CLASS_H

#include <stdbool.h>

typedef struct GlassClass GlassClass;
struct CopyInterface;
struct GlassFunction;
struct List;
struct Map;
struct String;

extern const struct CopyInterface *CLASS_COPY_OPS;

struct Map *get_builtin_classes(void);

GlassClass *copy_glass_class(const GlassClass *gclass);

void free_glass_class(GlassClass *gclass);

const struct String *class_get_name(const GlassClass *gclass);

const struct List *class_get_parents(const GlassClass *gclass);

bool class_has_func(const GlassClass *gclass, const struct String *name);

const struct GlassFunction *class_get_func(const GlassClass *gclass,
                                           const struct String *name);

struct List *class_get_func_names(const GlassClass *gclass);

#endif
