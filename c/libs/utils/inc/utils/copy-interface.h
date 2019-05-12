#ifndef UTILS_COPY_INTERFACE_H
#define UTILS_COPY_INTERFACE_H

typedef struct CopyInterface {
    void *(*copy_val)(const void *val);

    void (*free_val)(void *val);
} CopyInterface;

#define COPY_OPS_DECL(NAME) extern const CopyInterface *NAME ## _COPY_OPS

#define COPY_OPS_DEFINITION(TYPE, NAME)                         \
    static void *copy_ ## TYPE (const void *val) {               \
        TYPE *copy = malloc(sizeof(TYPE));                       \
        *copy = * (TYPE *) val;                                  \
        return copy;                                             \
    }                                                           \
                                                                \
    const CopyInterface *NAME ## _COPY_OPS = &(CopyInterface) { \
        copy_ ## TYPE, free,                                    \
    }                                                           \

COPY_OPS_DECL(INT);
COPY_OPS_DECL(SIZE_T);

#endif
