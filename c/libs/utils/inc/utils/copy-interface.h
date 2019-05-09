#ifndef UTILS_COPY_INTERFACE_H
#define UTILS_COPY_INTERFACE_H

typedef struct CopyInterface {
    void *(*copy_val)(const void *val);

    void (*free_val)(void *val);
} CopyInterface;

#endif
