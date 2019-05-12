#ifndef GLASSTYPES_GLASS_FUNCTION_H
#define GLASSTYPES_GLASS_FUNCTION_H

#include <stddef.h>

typedef struct GlassFunction GlassFunction;
struct CopyInterface;
struct GlassCommand;
struct String;

extern const struct CopyInterface *FUNC_COPY_OPS;

GlassFunction *copy_glass_func(const GlassFunction *func);

void free_glass_func(GlassFunction *func);

const struct String *func_get_name(const GlassFunction *func);

const struct GlassCommand *func_get_command(const GlassFunction *func, size_t index);

#endif
