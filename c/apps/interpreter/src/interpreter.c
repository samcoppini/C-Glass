#include "interpreter/interpreter.h"
#include "interpreter/glass-value.h"

#include "utils/map.h"
#include "utils/string.h"

int run_interpreter(const Map *classes) {
    Map *globals = new_map(STRING_HASH_OPS, VALUE_COPY_OPS);

    free_map(globals);

    return 0;
}
