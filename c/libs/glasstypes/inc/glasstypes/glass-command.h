#ifndef GLASSTYPES_GLASS_COMMAND_H
#define GLASSTYPES_GLASS_COMMAND_H

#include <stddef.h>

struct CopyInterface;
struct String;

typedef enum CommandType {
    CMD_ASSIGN_VAL,   // =
    CMD_ASSIGN_SELF,  // $
    CMD_DUPLICATE,    // (1)
    CMD_EXECUTE_FUNC, // ?
    CMD_GET_FUNC,     // .
    CMD_GET_VAL,      // *
    CMD_LOOP_BEGIN,   // /(name)
    CMD_LOOP_END,     // \ (paired with a loop begin)
    CMD_NEW_INST,     // !
    CMD_POP_STACK,    // ,
    CMD_PUSH_NAME,    // (name)
    CMD_PUSH_NUM,     // <42>
    CMD_PUSH_STR,     // "str"
    CMD_RETURN,       // ^

    CMD_BUILTIN,
    CMD_NOP,
} CommandType;

typedef enum BuiltinFunc {
    BUILTIN_OUTPUT_STR, // O.o
} BuiltinFunc;

typedef struct GlassCommand {
    CommandType type;

    union {
        struct {
            struct String *str;

            size_t index;
        };

        double number;

        BuiltinFunc builtin;
    };
} GlassCommand;

extern const struct CopyInterface *CMD_COPY_OPS;

#endif
