#ifndef UTILS_COPY_INTERFACE_H
#define UTILS_COPY_INTERFACE_H

// A struct to use for Lists/Maps to copy and free data
typedef struct CopyInterface {
    // Returns a copy of some data
    void *(*copy_val)(const void *val);

    // Frees the memory associated with the data
    void (*free_val)(void *val);
} CopyInterface;

#define COPY_OPS_DECL(NAME) extern const CopyInterface *NAME ## _COPY_OPS

// Macro for defining a CopyInterface for a simple data type
#define COPY_OPS_DEFINITION(TYPE, NAME)                          \
    static void *copy_ ## NAME (const void *val) {               \
        TYPE *copy = malloc(sizeof(TYPE));                       \
        *copy = * (TYPE *) val;                                  \
        return copy;                                             \
    }                                                            \
                                                                 \
    const CopyInterface *NAME ## _COPY_OPS = &(CopyInterface) {  \
        copy_ ## NAME, free,                                     \
    }                                                            \

COPY_OPS_DECL(INT);
COPY_OPS_DECL(SIZE_T);
COPY_OPS_DECL(VOID_PTR);

#endif
