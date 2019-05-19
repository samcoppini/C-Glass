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
#include <stdio.h>

typedef enum VarScope {
    SCOPE_LOCAL,
    SCOPE_CLASS,
    SCOPE_GLOBAL,
} VarScope;

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

int execute_builtin(BuiltinFunc func, List *stack) {
    switch (func) {
        case BUILTIN_MATH_ADD: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.a!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot add non-numbers!\n");
                return 1;
            }
            val1->num += val2->num;
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_DIVIDE: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.d!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot compare non-numbers!\n");
                return 1;
            }
            val1->num = val2->num / val1->num;
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_EQUAL: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.e!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot compare non-numbers!\n");
                return 1;
            }
            val1->num = (val2->num == val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_FLOOR: {
            if (list_len(stack) == 0) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.f!\n");
                return 1;
            }
            GlassValue *val = list_pop(stack);
            if (val->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot floor non-number!\n");
                return 1;
            }
            val->num = floor(val->num);
            list_add(stack, val);
            free_glass_value(val);
            break;
        }

        case BUILTIN_MATH_GREATER_THAN: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.gt!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot compare non-numbers!\n");
                return 1;
            }
            val1->num = (val2->num > val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_GREATER_OR_EQUAL: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.ge!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot compare non-numbers!\n");
                return 1;
            }
            val1->num = (val2->num >= val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_LESS_OR_EQUAL: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.le!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot compare non-numbers!\n");
                return 1;
            }
            val1->num = (val2->num <= val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }
        
        case BUILTIN_MATH_LESS_THAN: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.lt!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot compare non-numbers!\n");
                return 1;
            }
            val1->num = (val2->num < val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_MODULO: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.mod!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot compare non-numbers!\n");
                return 1;
            }
            val1->num = fmod(val2->num, val1->num);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_MULTIPLY: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.m!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot compare non-numbers!\n");
                return 1;
            }
            val1->num = val2->num * val1->num;
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_NOT_EQUAL: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.ne!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot compare non-numbers!\n");
                return 1;
            }
            val1->num = (val2->num != val1->num ? 1.0 : 0.0);
            list_add(stack, val1);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_MATH_SUBTRACT: {
            if (list_len(stack) < 2) {
                fprintf(stderr, "Error! Not enough arguments on the stack for A.s!\n");
                return 1;
            }
            GlassValue *val1 = list_pop(stack);
            GlassValue *val2 = list_pop(stack);
            if (val1->type != VALUE_NUMBER || val2->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Cannot subtract non-numbers!\n");
                return 1;
            }
            val2->num -= val1->num;
            list_add(stack, val2);
            free_glass_value(val1);
            free_glass_value(val2);
            break;
        }

        case BUILTIN_OUTPUT_NUM: {
            if (list_len(stack) == 0) {
                fprintf(stderr, "Error! Stack is empty!\n");
                return 1;
            }
            GlassValue *val = list_pop(stack);
            if (val->type != VALUE_NUMBER) {
                fprintf(stderr, "Error! Tried to output non-number as number!\n");
                return 1;
            }
            printf("%g", val->num);
            free_glass_value(val);
            break;
        }

        case BUILTIN_OUTPUT_STR: {
            if (list_len(stack) == 0) {
                fprintf(stderr, "Error! Stack is empty!\n");
                return 1;
            }
            GlassValue *val = list_pop(stack);
            if (val->type != VALUE_STRING) {
                fprintf(stderr, "Error! Cannot output non-string!\n");
                return 1;
            }
            printf("%s", string_get_c_str(val->str));
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
                if (list_len(stack) == 0) {
                    fprintf(stderr, "Error! $ operation cannot be done on an empty stack.\n");
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *name_val = list_pop(stack);
                if (name_val->type != VALUE_NAME) {
                    fprintf(stderr, "Error! Cannot assign to a non-name!\n");
                    free_glass_value(name_val);
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *self_val = new_inst_value(inst);
                set_var(name_val->str, self_val, globals, inst, local_vars);
                free_glass_value(self_val);
                free_glass_value(name_val);
                break;
            }

            case CMD_ASSIGN_VAL: {
                if (list_len(stack) < 2) {
                    fprintf(stderr, "Error! = operation needs more arguments on the stack!\n");
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *val = list_pop(stack);
                GlassValue *name_val = list_pop(stack);
                if (name_val->type != VALUE_NAME) {
                    fprintf(stderr, "Error! Unable to assign to a non-name!\n");
                    free_glass_value(name_val);
                    free_glass_value(val);
                    free_map(local_vars);
                    return 1;
                }
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
                if (list_len(stack) < 2) {
                    return 1;
                }
                GlassValue *fname_val = list_pop(stack);
                GlassValue *oname_val = list_pop(stack);
                if (fname_val->type != VALUE_NAME || oname_val->type != VALUE_NAME) {
                    free_glass_value(fname_val);
                    free_glass_value(oname_val);
                    free_map(local_vars);
                    return 1;
                }
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
                if (list_len(stack) == 0) {
                    fprintf(stderr, "Error! * operation cannot be done on an empty stack.\n");
                    free_map(local_vars);
                    return 1;
                }
                GlassValue *name_val = list_pop(stack);
                if (name_val->type != VALUE_NAME) {
                    fprintf(stderr, "Error! Cannot dereference a non-name!\n");
                    free_glass_value(name_val);
                    free_map(local_vars);
                    return 1;
                }
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
                if (list_len(stack) < 1) {
                    free_map(local_vars);
                    fprintf(stderr, "Not enough arguments on the stack for ? operation.\n");
                    return 1;
                }
                GlassValue *func_val = list_pop(stack);
                if (func_val->type != VALUE_FUNCTION) {
                    fprintf(stderr, "Cannot execute non-function!\n");
                    free_map(local_vars);
                    free_glass_value(func_val);
                    return 1;
                }
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
                if (list_len(stack) < 2) {
                    return 1;
                }
                GlassValue *cname_val = list_pop(stack);
                GlassValue *oname_val = list_pop(stack);
                if (cname_val->type != VALUE_NAME || oname_val->type != VALUE_NAME) {
                    free_map(local_vars);
                    free_glass_value(cname_val);
                    free_glass_value(oname_val);
                    return 1;
                }
                const GlassClass *gclass = map_get(classes, cname_val->str);
                GlassInstance *inst = new_glass_instance(gclass);
                GlassValue *inst_val = new_inst_value(inst);
                set_var(oname_val->str, inst_val, globals, inst, local_vars);
                free_glass_value(inst_val);
                break;
            }

            case CMD_POP_STACK: {
                if (list_len(stack) == 0) {
                    fprintf(stderr, "Unable to pop an empty stack!\n");
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
