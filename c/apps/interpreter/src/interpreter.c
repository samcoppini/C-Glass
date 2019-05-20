#include "interpreter/interpreter.h"
#include "interpreter/glass-instance.h"
#include "interpreter/glass-value.h"

#include "glasstypes/glass-command.h"
#include "glasstypes/glass-class.h"
#include "glasstypes/glass-function.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/string.h"

#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#define MAX_OP_ARGS 5

typedef enum VarScope {
    SCOPE_LOCAL,
    SCOPE_CLASS,
    SCOPE_GLOBAL,
} VarScope;

typedef enum ArgType {
    ARG_ANY,
    ARG_CHAR,
    ARG_FUNC,
    ARG_INST,
    ARG_INT,
    ARG_NAME,
    ARG_NUM,
    ARG_STR,
} ArgType;

const char *arg_name_str(ArgType type) {
    switch (type) {
        case ARG_ANY:  return "any";
        case ARG_CHAR: return "character";
        case ARG_FUNC: return "function";
        case ARG_INT:  return "integer";
        case ARG_INST: return "instance";
        case ARG_NAME: return "name";
        case ARG_NUM:  return "number";
        case ARG_STR:  return "string";
        default:       return "unknown";
    }
}

VarScope get_var_scope(const String *str) {
    char c = string_get(str, 0);
    if (c == '_') {
        return SCOPE_LOCAL;
    }
    else if (islower(c)) {
        return SCOPE_CLASS;
    }
    else {
        return SCOPE_GLOBAL;
    }
}

bool check_stack(const List *stack, const char *op_name, size_t size, ...) {
    if (list_len(stack) < size) {
        fprintf(stderr, "Error! %s requires %u elements on the stack, but the stack only has %u elements!\n",
                op_name, size, list_len(stack));
        return true;
    }

    va_list ap;
    ArgType types[MAX_OP_ARGS];

    va_start(ap, size);
    for (size_t i = 0; i < size; i++) {
        types[i] = va_arg(ap, ArgType);
    }
    va_end(ap);
    
    bool types_matched = true;
    for (size_t i = 0; i < size; i++) {
        const GlassValue *val = list_get(stack, list_len(stack) - size + i);
        switch (types[i]) {
            case ARG_ANY:
                break;
            
            case ARG_CHAR:
                types_matched &= (val->type == VALUE_STRING &&
                                  string_len(val->str) == 1);
                break;

            case ARG_FUNC:
                types_matched &= (val->type == VALUE_FUNCTION);
                break;
            
            case ARG_INST:
                types_matched &= (val->type == VALUE_INSTANCE);
                break;

            case ARG_INT:
                types_matched &= (val->type == VALUE_NUMBER &&
                                  val->num == floor(val->num));
                break;

            case ARG_NAME:
                types_matched &= (val->type == VALUE_NAME);
                break;
            
            case ARG_NUM:
                types_matched &= (val->type == VALUE_NUMBER);
                break;

            case ARG_STR:
                types_matched &= (val->type == VALUE_STRING);
                break;
        }
    }

    if (!types_matched) {
        fprintf(stderr, "Wrong arguments given for %s operation!\n", op_name);
        fprintf(stderr, "Expected:");
        for (size_t i = 0; i < size; i++) {
            fprintf(stderr, " %s", arg_name_str(types[i]));
        }
        fprintf(stderr, "\nReceived:");
        for (size_t i = 0; i < size; i++) {
            const GlassValue *val = list_get(stack, list_len(stack) - size + i);
            String *str = value_get_string(val);
            fprintf(stderr, " %s", string_get_c_str(str));
            free_string(str);
        }
        fprintf(stderr, "\n");
        return true;
    }

    return false;
}

int execute_builtin(BuiltinFunc func, List *stack) {
    switch (func) {
        case BUILTIN_MATH_ADD: {
            if (check_stack(stack, "A.a", 1, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num += val2->num;
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_DIVIDE: {
            if (check_stack(stack, "A.d", 1, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num = val2->num / val1->num;
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_EQUAL: {
            if (check_stack(stack, "A.e", 1, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num = (val2->num == val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_FLOOR: {
            if (check_stack(stack, "A.f", 1, ARG_NUM)) {
                return 1;
            }
            GlassValue *val = list_pop(stack);
            val->num = floor(val->num);
            list_add(stack, val);
            free_glass_value(val);
            break;
        }

        case BUILTIN_MATH_GREATER_THAN: {
            if (check_stack(stack, "A.gt", 2, ARG_NUM, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num = (val2->num > val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_GREATER_OR_EQUAL: {
            if (check_stack(stack, "A.ge", 2, ARG_NUM, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num = (val2->num >= val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_LESS_OR_EQUAL: {
            if (check_stack(stack, "A.le", 2, ARG_NUM, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num = (val2->num <= val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }
        
        case BUILTIN_MATH_LESS_THAN: {
            if (check_stack(stack, "A.lt", 2, ARG_NUM, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num = (val2->num < val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_MODULO: {
            if (check_stack(stack, "A.mod", 2, ARG_NUM, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num = fmod(val2->num, val1->num);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_MULTIPLY: {
            if (check_stack(stack, "A.m", 2, ARG_NUM, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num = val2->num * val1->num;
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_NOT_EQUAL: {
            if (check_stack(stack, "A.ne", 2, ARG_NUM, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val1->num = (val2->num != val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_SUBTRACT: {
            if (check_stack(stack, "A.s", 2, ARG_NUM, ARG_NUM)) {
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            val2->num -= val1->num;
            list_add(stack, val2);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_OUTPUT_NUM: {
            if (check_stack(stack, "O.on", 1, ARG_NUM)) {
                return 1;
            }
            GlassValue *val = list_pop(stack);
            printf("%g", val->num);
            free_glass_value(val);
            break;
        }

        case BUILTIN_OUTPUT_STR: {
            if (check_stack(stack, "O.o", 1, ARG_STR)) {
                return 1;
            }
            GlassValue *val = list_pop(stack);
            printf("%s", string_get_c_str(val->str));
            free_glass_value(val);
            break;
        }

        case BUILTIN_STR_APPEND: {
            if (check_stack(stack, "S.a", 2, ARG_STR, ARG_STR)) {
                return 1;
            }
            GlassValue *str1 = list_pop(stack);
            GlassValue *str2 = list_pop(stack);
            string_add_str(str2->str, str1->str);
            list_add(stack, str2);
            free_glass_value(str1);
            free_glass_value(str2);
            break;
        }

        case BUILTIN_STR_EQUAL: {
            if (check_stack(stack, "S.e", 2, ARG_STR, ARG_STR)) {
                return 1;
            }
            GlassValue *str1 = list_pop(stack);
            GlassValue *str2 = list_pop(stack);
            GlassValue *res = new_number_value(strings_equal(str1->str, str2->str) ? 1.0 : 0.0);
            list_add(stack, res);
            free_glass_value(str1);
            free_glass_value(str2);
            free_glass_value(res);
            break;
        }

        case BUILTIN_STR_INDEX: {
            if (check_stack(stack, "S.i", 2, ARG_STR, ARG_INT)) {
                return 1;
            }
            GlassValue *int_val = list_pop(stack);
            GlassValue *str_val = list_pop(stack);
            if (int_val->num < 0 || int_val->num >= string_len(str_val->str)) {
                fprintf(stderr,
                        "Error! Index %g is out of range for S.i operation with string of length %u.\n",
                        int_val->num, string_len(str_val->str));
                return 1;
            }
            String *char_str = string_from_char(string_get(str_val->str, (size_t) int_val->num));
            GlassValue *char_val = new_str_value(char_str);
            list_add(stack, char_val);
            free_string(char_str);
            free_glass_value(char_val);
            free_glass_value(int_val);
            free_glass_value(str_val);
            break;
        }

        case BUILTIN_STR_LENGTH: {
            if (check_stack(stack, "S.l", 1, ARG_STR)) {
                return 1;
            }
            GlassValue *str_val = list_pop(stack);
            GlassValue *len_val = new_number_value(string_len(str_val->str));
            list_add(stack, len_val);
            free_glass_value(len_val);
            free_glass_value(str_val);
            break;
        }

        case BUILTIN_STR_NUM_TO_STR: {
            if (check_stack(stack, "S.ns", 1, ARG_INT)) {
                return 1;
            }
            GlassValue *num_val = list_pop(stack);
            String *str = string_from_char((char) num_val->num);
            GlassValue *str_val = new_str_value(str);
            list_add(stack, str_val);
            free_glass_value(str_val);
            free_glass_value(num_val);
            free_string(str);
            break;
        }

        case BUILTIN_STR_REPLACE: {
            if (check_stack(stack, "S.si", 3, ARG_STR, ARG_INT, ARG_CHAR)) {
                return 1;
            }
            GlassValue *char_val = list_pop(stack);
            GlassValue *int_val = list_pop(stack);
            GlassValue *str_val = list_pop(stack);
            if (int_val->num < 0 || int_val->num >= string_len(str_val->str)) {
                fprintf(stderr,
                        "Error! Index %u is out of range for S.si operation with string of length %u.\n",
                        (unsigned) int_val->num, string_len(str_val->str));
                return 1;
            }
            string_set(str_val->str, int_val->num, string_get(char_val->str, 0));
            list_add(stack, str_val);
            free_glass_value(char_val);
            free_glass_value(int_val);
            free_glass_value(str_val);
            break;
        }

        case BUILTIN_STR_STR_TO_NUM: {
            if (check_stack(stack, "S.sn", 1, ARG_CHAR)) {
                return 1;
            }
            GlassValue *char_val = list_pop(stack);
            char c = string_get(char_val->str, 0);
            GlassValue *num_val = new_number_value((double) c);
            list_add(stack, num_val);
            free_glass_value(num_val);
            free_glass_value(char_val);
            break;
        }

        case BUILTIN_VAR_NEW: {
            static int var_index = 0;
            char buf[80];
            sprintf(buf, "<Anonymous Var %d>", var_index);
            var_index++;
            String *str = string_from_chars(buf);
            GlassValue *val = new_name_value(str);
            list_add(stack, val);
            free_glass_value(val);
            free_string(str);
            break;
        }

        case BUILTIN_VAR_DELETE: {
            if (check_stack(stack, "V.d", 1, ARG_NAME)) {
                return 1;
            }
            GlassValue *val = list_pop(stack);
            free_glass_value(val);
            break;
        }

        default:
            break;
    }

    return 0;
}

bool value_is_truthy(const GlassValue *val) {
    if (val->type == VALUE_NUMBER) {
        return val->num != 0.0;
    }
    else if (val->type == VALUE_STRING) {
        return string_len(val->str) != 0;
    }
    else {
        return false;
    }
}

const GlassValue *get_var(const String *name, const Map *globals, const GlassInstance *inst, const Map *locals) {
    switch (get_var_scope(name)) {
        case SCOPE_LOCAL:
            return map_get(locals, name);
        case SCOPE_CLASS:
            return instance_get_var(inst, name);
        case SCOPE_GLOBAL:
            return map_get(globals, name);
    }
    return NULL;
}

void set_var(const String *name, const GlassValue *val, Map *globals, GlassInstance *inst, Map *locals) {
    switch (get_var_scope(name)) {
        case SCOPE_LOCAL:
            map_set(locals, name, val);
            break;
        case SCOPE_CLASS:
            instance_set_var(inst, name, val);
            break;
        case SCOPE_GLOBAL:
            map_set(globals, name, val);
            break;
    }
}

int execute_function(GlassValue *func_val, List *stack, Map *globals, const Map *classes) {
    const GlassFunction *func = instance_get_func(func_val->inst, func_val->str);
    GlassInstance *inst = func_val->inst;

    Map *local_vars = new_map(STRING_HASH_OPS, VALUE_COPY_OPS);

    for (size_t cmd_idx = 0; cmd_idx < func_len(func); cmd_idx++) {
        const GlassCommand *cmd = func_get_command(func, cmd_idx);
        switch (cmd->type) {
            case CMD_ASSIGN_SELF: {
                if (check_stack(stack, "$", 1, ARG_NAME)) {
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *name_val = list_pop(stack);
                GlassValue *self_val = new_inst_value(inst);
                set_var(name_val->str, self_val, globals, inst, local_vars);
                free_glass_value(self_val);
                free_glass_value(name_val);
                break;
            }

            case CMD_ASSIGN_VAL: {
                if (check_stack(stack, "=", 2, ARG_NAME, ARG_ANY)) {
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *val = list_pop(stack);
                GlassValue *name_val = list_pop(stack);
                set_var(name_val->str, val, globals, inst, local_vars);
                free_glass_value(name_val);
                free_glass_value(val);
                break;
            }

            case CMD_BUILTIN: {
                int ret = execute_builtin(cmd->builtin, stack);
                if (ret != 0) {
                    free_map(local_vars);
                    return ret;
                }
                break;
            }

            case CMD_DUPLICATE: {
                if (list_len(stack) <= cmd->index) {
                    fprintf(stderr, "Cannot duplicate out-of-range stack element!\n");
                    free_map(local_vars);
                    return 1;
                }
                const GlassValue *val = list_get(stack, list_len(stack) - cmd->index - 1);
                list_add(stack, val);
                break;
            }

            case CMD_GET_FUNC: {
                if (check_stack(stack, ".", 2, ARG_NAME, ARG_NAME)) {
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *fname_val = list_pop(stack);
                GlassValue *oname_val = list_pop(stack);
                const GlassValue *obj_val = get_var(oname_val->str, globals, inst, local_vars);
                if (obj_val == NULL) {
                    fprintf(stderr, "Error! %s not defined!\n",
                            string_get_c_str(oname_val->str));
                    free_glass_value(fname_val);
                    free_glass_value(oname_val);
                    free_map(local_vars);
                    return 1;
                }
                else if (obj_val->type != VALUE_INSTANCE) {
                    fprintf(stderr, "Error! %s is not an instance of a class.",
                            string_get_c_str(oname_val->str));
                    free_glass_value(fname_val);
                    free_glass_value(oname_val);
                    free_map(local_vars);
                    return 1;
                }
                else if (!instance_has_func(obj_val->inst, fname_val->str)) {
                    fprintf(stderr, "Error! %s has no %s function!\n",
                            string_get_c_str(oname_val->str),
                            string_get_c_str(fname_val->str));
                    free_glass_value(fname_val);
                    free_glass_value(oname_val);
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *func_val = new_func_value(obj_val->inst, fname_val->str);
                list_add(stack, func_val);
                free_glass_value(func_val);
                break;
            }

            case CMD_GET_VAL: {
                if (check_stack(stack, "*", 1, ARG_NAME)) {
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *name_val = list_pop(stack);
                const GlassValue *val = get_var(name_val->str, globals, inst, local_vars);
                if (val == NULL) {
                    fprintf(stderr, "Error! %s is not defined!\n",
                            string_get_c_str(name_val->str));
                    free_map(local_vars);
                    return 1;
                }
                list_add(stack, val);
                free_glass_value(name_val);
                break;
            }

            case CMD_EXECUTE_FUNC: {
                if (check_stack(stack, "?", 1, ARG_FUNC)) {
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *func_val = list_pop(stack);
                int ret = execute_function(func_val, stack, globals, classes);
                free_glass_value(func_val);
                if (ret != 0) {
                    return ret;
                }
                break;
            }

            case CMD_LOOP_BEGIN: {
                const GlassValue *val = get_var(cmd->str, globals, inst, local_vars);
                if (val == NULL) {
                    fprintf(stderr, "Error! %s is undefined!\n", string_get_c_str(cmd->str));
                    free_map(local_vars);
                    return 1;
                }
                if (!value_is_truthy(val)) {
                    cmd_idx = cmd->index;
                }
                break;
            }
            
            case CMD_LOOP_END: {
                const GlassValue *val = get_var(cmd->str, globals, inst, local_vars);
                if (val == NULL) {
                    fprintf(stderr, "Error! %s is undefined!\n", string_get_c_str(cmd->str));
                    free_map(local_vars);
                    return 1;
                }
                if (value_is_truthy(val)) {
                    cmd_idx = cmd->index;
                }
                break;
            }

            case CMD_NEW_INST: {
                if (check_stack(stack, "!", 2, ARG_NAME, ARG_NAME)) {
                    return 1;
                }
                GlassValue *cname_val = list_pop(stack);
                GlassValue *oname_val = list_pop(stack);
                const GlassClass *gclass = map_get(classes, cname_val->str);
                if (gclass == NULL) {
                    fprintf(stderr, "Error! (%s) is not a class!\n",
                            string_get_c_str(cname_val->str));
                    free_glass_value(cname_val);
                    free_glass_value(oname_val);
                    free_map(local_vars);
                    return 1;
                }
                GlassInstance *inst = new_glass_instance(gclass);
                GlassValue *inst_val = new_inst_value(inst);
                set_var(oname_val->str, inst_val, globals, inst, local_vars);
                free_glass_value(inst_val);
                break;
            }

            case CMD_POP_STACK: {
                if (check_stack(stack, ",", 1, ARG_ANY)) {
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *val = list_pop(stack);
                free_glass_value(val);
                break;
            }

            case CMD_PUSH_NAME: {
                GlassValue *name_val = new_name_value(cmd->str);
                list_add(stack, name_val);
                free_glass_value(name_val);
                break;
            }

            case CMD_PUSH_NUM: {
                GlassValue *num_val = new_number_value(cmd->number);
                list_add(stack, num_val);
                free_glass_value(num_val);
                break;
            }

            case CMD_PUSH_STR: {
                GlassValue *str_val = new_str_value(cmd->str);
                list_add(stack, str_val);
                free_glass_value(str_val);
                break;
            }

            case CMD_RETURN: {
                free_map(local_vars);
                return 0;
            }

            default:
                // Unimplemented, as of yet
                break;
        }
    }

    free_map(local_vars);
    return 0;
}

int run_interpreter(const Map *classes) {
    String *main_class_name = string_from_char('M');

    if (!map_has(classes, main_class_name)) {
        fprintf(stderr, "No M class defined!\n");
        free_string(main_class_name);
        return 1;
    }

    const GlassClass *main_class = map_get(classes, main_class_name);
    free_string(main_class_name);
    String *main_func_name = string_from_char('m');

    if (!class_has_func(main_class, main_func_name)) {
        fprintf(stderr, "M class has no m function defined!");
        return 1;
    }

    List *stack = new_list(VALUE_COPY_OPS);
    Map *globals = new_map(STRING_HASH_OPS, VALUE_COPY_OPS);

    GlassInstance *main_inst = new_glass_instance(main_class);

    GlassValue val = {
        VALUE_FUNCTION, .inst = main_inst, .str = copy_string(main_func_name)
    };

    int ret_val = execute_function(&val, stack, globals, classes);

    free_map(globals);
    free_list(stack);
    free_string(main_func_name);

    return ret_val;
}
