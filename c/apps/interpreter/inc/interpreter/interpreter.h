#ifndef INTERPRETER_INTERPRETER_H
#define INTERPRETER_INTERPRETER_H

struct List;
struct Map;

int run_interpreter(const struct Map *classes, const struct List *args);

#endif
