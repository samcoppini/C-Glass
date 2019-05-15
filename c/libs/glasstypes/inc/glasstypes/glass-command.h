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
    BUILTIN_INPUT_CHAR,            // I.c
    BUILTIN_INPUT_EOF,             // I.e
    BUILTIN_INPUT_LINE,            // I.l
    BUILTIN_MATH_ADD,              // A.a
    BUILTIN_MATH_DIVIDE,           // A.d
    BUILTIN_MATH_EQUAL,            // A.e
    BUILTIN_MATH_FLOOR,            // A.f
    BUILTIN_MATH_GREATER_OR_EQUAL, // A.ge
    BUILTIN_MATH_GREATER_THAN,     // A.gt
    BUILTIN_MATH_LESS_OR_EQUAL,    // A.le
    BUILTIN_MATH_LESS_THAN,        // A.lt
    BUILTIN_MATH_MODULO,           // A.mod
    BUILTIN_MATH_MULTIPLY,         // A.m
    BUILTIN_MATH_NOT_EQUAL,        // A.ne
    BUILTIN_MATH_SUBTRACT,         // A.s
    BUILTIN_OUTPUT_NUM,            // O.on
    BUILTIN_OUTPUT_STR,            // O.o
    BUILTIN_STR_APPEND,            // S.a
    BUILTIN_STR_EQUAL,             // S.e
    BUILTIN_STR_INDEX,             // S.i
    BUILTIN_STR_LENGTH,            // S.l
    BUILTIN_STR_NUM_TO_STR,        // S.ns
    BUILTIN_STR_REPLACE,           // S.si
    BUILTIN_STR_SPLIT,             // S.d
    BUILTIN_STR_STR_TO_NUM,        // S.sn
    BUILTIN_VAR_DELETE,            // V.d
    BUILTIN_VAR_NEW,               // V.n
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
