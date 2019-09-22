#include "compiler/compiler.h"
#include "glasstypes/glass-class.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/set.h"
#include "utils/string.h"

#include <ctype.h>
#include <stdio.h>

extern const char *RUNTIME_LIBRARY[];

extern const size_t RUNTIME_LIBRARY_LINES;

extern const char *MAIN_FUNC[];

extern const size_t MAIN_FUNC_LINES;

extern const char *BUILTIN_FUNCS[];

extern const size_t BUILTIN_FUNCS_LINES;

String *mangle_name(const String *class_name, const String *func_name) {
    String *new_name = string_from_char('f');
    char buf[80];

    sprintf(buf, "%zu", string_len(class_name));
    string_add_chars(new_name, buf);
    string_add_str(new_name, class_name);

    sprintf(buf, "%zu", string_len(func_name));
    string_add_chars(new_name, buf);
    string_add_str(new_name, func_name);

    return new_name;
}

String *convert_str_to_identifier(const String *str) {
    String *ident = new_string();

    for (size_t i = 0; i < string_len(str); i++) {
        char c = string_get(str, i);

        if (isalnum(c)) {
            string_add_char(ident, c);
        }
        else {
            char buf[10];
            sprintf(buf, "_%03d", c);
            string_add_chars(ident, buf);
        }
    }

    return ident;
}

String *make_quoted_string(const String *str) {
    String *quoted = string_from_char('"');

    for (size_t i = 0; i < string_len(str); i++) {
        char c = string_get(str, i);
        if (c == '\\') {
            string_add_chars(quoted, "\\\\");
        }
        else if (isprint(c)) {
            string_add_char(quoted, c);
        }
        else {
            const char HEX_CHARS[16] = "0123456789ABCDEF";
            string_add_chars(quoted, "\\x");
            string_add_char(quoted, HEX_CHARS[(c & 0xF0) >>  4]);
            string_add_char(quoted, HEX_CHARS[c & 0x0F]);
        }
    }

    string_add_char(quoted, '"');
    return quoted;
}

Set *get_all_names(const Map *classes) {
    List *class_names = map_get_keys(classes);
    Set *all_names = new_set(STRING_HASH_OPS);

    for (size_t i = 0; i < list_len(class_names); i++) {
        const String *class_name = list_get(class_names, i);
        set_add(all_names, class_name);

        const GlassClass *gclass = map_get(classes, class_name);
        List *func_names = class_get_func_names(gclass);

        for (size_t j = 0; j < list_len(func_names); j++) {
            const String *func_name = list_get(func_names, j);
            set_add(all_names, func_name);

            const GlassFunction *func = class_get_func(gclass, func_name);

            for (size_t k = 0; k < func_len(func); k++) {
                const GlassCommand *cmd = func_get_command(func, k);

                if (cmd->type == CMD_PUSH_NAME || cmd->type == CMD_LOOP_BEGIN) {
                    set_add(all_names, cmd->str);
                }
            }
        }

        free_list(func_names);
    }

    free_list(class_names);
    return all_names;
}

Set *get_all_strings(const Map *classes) {
    List *class_names = map_get_keys(classes);
    Set *all_strings = new_set(STRING_HASH_OPS);

    for (size_t i = 0; i < list_len(class_names); i++) {
        const String *class_name = list_get(class_names, i);
        const GlassClass *gclass = map_get(classes, class_name);
        List *func_names = class_get_func_names(gclass);

        for (size_t j = 0; j < list_len(func_names); j++) {
            const String *func_name = list_get(func_names, j);
            const GlassFunction *func = class_get_func(gclass, func_name);

            for (size_t k = 0; k < func_len(func); k++) {
                const GlassCommand *cmd = func_get_command(func, k);

                if (cmd->type == CMD_PUSH_STR) {
                    set_add(all_strings, cmd->str);
                }
            }
        }

        free_list(func_names);
    }

    free_list(class_names);
    return all_strings;
}

void generate_name_enum(String *code, const Map *classes) {
    Set *name_set = get_all_names(classes);
    List *names = set_to_list(name_set);

    string_add_chars(code, "typedef enum Name {\n    NO_NAME,\n");

    for (size_t i = 0; i < list_len(names); i++) {
        string_add_chars(code, "    NAME_");
        string_add_str(code, list_get(names, i));
        string_add_chars(code, ",\n");
    }

    string_add_chars(code, "    NUM_NAMES\n");
    string_add_chars(code, "\n} Name;\n\n");

    string_add_chars(code, "typedef enum NameScope {\n");
    string_add_chars(code, "    SCOPE_LOCAL,\n");
    string_add_chars(code, "    SCOPE_CLASSWIDE,\n");
    string_add_chars(code, "    SCOPE_GLOBAL\n");
    string_add_chars(code, "} NameScope;\n\n");

    string_add_chars(code, "NameScope get_name_scope(Name name) {\n");
    string_add_chars(code, "    switch (name) {\n");

    for (size_t i = 0; i < list_len(names); i++) {
        const String *name = list_get(names, i);
        
        string_add_chars(code, "        case NAME_");
        string_add_str(code, name);
        string_add_chars(code, ": return ");

        char c = string_get(name, 0);

        if (isupper(c)) {
            string_add_chars(code, "SCOPE_GLOBAL;\n");
        }
        else if (islower(c)) {
            string_add_chars(code, "SCOPE_CLASSWIDE;\n");
        }
        else {
            string_add_chars(code, "SCOPE_LOCAL;\n");
        }
    }

    string_add_chars(code, "        default: return SCOPE_GLOBAL;\n");
    string_add_chars(code, "    }\n}\n\n");

    free_set(name_set);
    free_list(names);
}

void add_runtime_library(String *code) {
    for (size_t i = 0; i < RUNTIME_LIBRARY_LINES; i++) {
        string_add_chars(code, RUNTIME_LIBRARY[i]);
        string_add_char(code, '\n');
    }
}

void add_builtin_funcs(String *code) {
    for (size_t i = 0; i < BUILTIN_FUNCS_LINES; i++) {
        string_add_chars(code, BUILTIN_FUNCS[i]);
        string_add_char(code, '\n');
    }
}

void add_main_func(String *code) {
    for (size_t i = 0; i < MAIN_FUNC_LINES; i++) {
        string_add_chars(code, MAIN_FUNC[i]);
        string_add_char(code, '\n');
    }
}

void add_indents(String *code, int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        string_add_chars(code, "    ");
    }
}

void generate_name_literals(String *code, const Map *classes) {
    Set *name_set = get_all_names(classes);
    List *names = set_to_list(name_set);

    for (size_t i = 0; i < list_len(names); i++) {
        const String *name = list_get(names, i);

        string_add_chars(code, "GlassValue nameValue_");
        string_add_str(code, name);
        string_add_chars(code, " = { .ref_count = 1, .type = TYPE_NAME, .name = NAME_");
        string_add_str(code, name);
        string_add_chars(code, "};\n");
    }

    free_list(names);
    free_set(name_set);
}

void generate_string_literals(String *code, const Map *classes) {
    Set *string_set = get_all_strings(classes);
    List *string_list = set_to_list(string_set);

    for (size_t i = 0; i < list_len(string_list); i++) {
        const String *str = list_get(string_list, i);
        String *str_ident = convert_str_to_identifier(str);

        string_add_chars(code, "String strLiteral_");
        string_add_str(code, str_ident);
        string_add_chars(code, " = {");

        String *quoted = make_quoted_string(str);
        string_add_str(code, quoted);

        string_add_chars(code, ", 1, 0, ");
        char buf[80];
        sprintf(buf, "%d};\n", string_len(str));
        string_add_chars(code, buf);

        string_add_chars(code, "GlassValue strValue_");
        string_add_str(code, str_ident);
        string_add_chars(code, " = { .ref_count = 1, .type = TYPE_STRING, .str = &strLiteral_");
        string_add_str(code, str_ident);
        string_add_chars(code, "};\n");

        free_string(str_ident);
        free_string(quoted);
    }

    free_list(string_list);
    free_set(string_set);
}

void generate_function(String *code, const GlassClass *gclass, const GlassFunction *func) {
    const String *class_name = class_get_name(gclass);
    const String *func_name = func_get_name(func);
    String *mangled_name = mangle_name(class_name, func_name);

    string_add_chars(code, "void ");
    string_add_str(code, mangled_name);
    string_add_chars(code, "(size_t inst_index) {\n");

    int indent_level = 1;

    add_indents(code, indent_level);
    string_add_chars(code, "Map *local_vars = new_map();\n");
    add_indents(code, indent_level);
    string_add_chars(code, "GlassValue *tmp, *tmp2, *tmp3;\n");
    add_indents(code, indent_level);
    string_add_chars(code, "size_t index;\n");

    for (size_t i = 0; i < func_len(func); i++) {
        const GlassCommand *cmd = func_get_command(func, i);

        add_indents(code, indent_level);

        switch (cmd->type) {
            case CMD_BUILTIN: {
                String *builtin_name = builtin_func_name(cmd->builtin);
                string_add_str(code, builtin_name);
                string_add_chars(code, "();\n");
                free_string(builtin_name);
                break;
            }

            case CMD_EXECUTE_FUNC: {
                string_add_chars(code, "tmp = stack_pop();\n");
                add_indents(code, indent_level);
                string_add_chars(code, "tmp->func.func(tmp->func.index);\n");
                add_indents(code, indent_level);
                string_add_chars(code, "free_value(tmp);\n");
                break;
            }

            case CMD_GET_FUNC: {
                string_add_chars(code, "tmp2 = stack_pop();\n");
                add_indents(code, indent_level);
                string_add_chars(code, "tmp = stack_pop();\n");
                add_indents(code, indent_level);
                string_add_chars(code, "tmp3 = get_var(tmp->name, local_vars, inst_index);\n");
                add_indents(code, indent_level);
                string_add_chars(code, "free_value(tmp);\n");
                add_indents(code, indent_level);
                string_add_chars(code, "tmp = make_func(tmp3->inst_index, tmp2->name);\n");
                add_indents(code, indent_level);
                string_add_chars(code, "stack_push(tmp);\n");
                add_indents(code, indent_level);
                string_add_chars(code, "free_value(tmp2);\n");
                break;
            }

            case CMD_NEW_INST: {
                string_add_chars(code, "tmp2 = stack_pop();\n");
                add_indents(code, indent_level);
                string_add_chars(code, "tmp = stack_pop();\n");
                add_indents(code, indent_level);
                string_add_chars(code, "index = new_instance(CLASSES_ARRAY[tmp2->name]);\n");
                add_indents(code, indent_level);
                string_add_chars(code, "free_value(tmp2);\n");
                add_indents(code, indent_level);
                string_add_chars(code, "tmp2 = malloc(sizeof(GlassValue));\n");
                add_indents(code, indent_level);
                string_add_chars(code, "tmp2->type = TYPE_INST;\n");
                add_indents(code, indent_level);
                string_add_chars(code, "tmp2->inst_index = index;\n");
                add_indents(code, indent_level);
                string_add_chars(code, "tmp2->ref_count = 1;\n");
                add_indents(code, indent_level);
                string_add_chars(code, "set_var(tmp->name, tmp2, local_vars, inst_index);\n");
                break;
            }

            case CMD_PUSH_NAME: { 
                string_add_chars(code, "nameValue_");
                string_add_str(code, cmd->str);
                string_add_chars(code, ".ref_count++;\n");
                add_indents(code, indent_level);
                string_add_chars(code, "stack_push(&nameValue_");
                string_add_str(code, cmd->str);
                string_add_chars(code, ");\n");
                break;
            }

            case CMD_PUSH_STR: {
                String *str_ident = convert_str_to_identifier(cmd->str);
                string_add_chars(code, "strValue_");
                string_add_str(code, str_ident);
                string_add_chars(code, ".ref_count++;\n");
                add_indents(code, indent_level);
                string_add_chars(code, "stack_push(&strValue_");
                string_add_str(code, str_ident);
                string_add_chars(code, ");\n");
                free_string(str_ident);
                break;
            }

            case CMD_RETURN: {
                string_add_chars(code, "free_map(local_vars);\n");
                add_indents(code, indent_level);
                string_add_chars(code, "return;\n");
            }
        }
    }

    string_add_chars(code, "free_map(local_vars);\n");
    string_add_chars(code, "}\n\n");

    free_string(mangled_name);
}

void generate_functions(String *code, const Map *classes) {
    List *class_names = map_get_keys(classes);

    for (size_t i = 0; i < list_len(class_names); i++) {
        const String *class_name = list_get(class_names, i);
        const GlassClass *gclass = map_get(classes, class_name);
        List *func_names = class_get_func_names(gclass);

        for (size_t j = 0; j < list_len(func_names); j++) {
            const String *func_name = list_get(func_names, j);
            const GlassFunction *func = class_get_func(gclass, func_name);

            generate_function(code, gclass, func);
        }

        free_list(func_names);
    }

    free_list(class_names);
}

void generate_class_definitions(String *code, const Map *classes) {
    List *class_names = map_get_keys(classes);

    String *classes_array = string_from_chars("const GlassClass *CLASSES_ARRAY[NUM_NAMES] = {\n");

    for (size_t i = 0; i < list_len(class_names); i++) {
        const String *class_name = list_get(class_names, i);
        const GlassClass *gclass = map_get(classes, class_name);
        List *func_names = class_get_func_names(gclass);

        string_add_chars(classes_array, "    [NAME_");
        string_add_str(classes_array, class_name);
        string_add_chars(classes_array, "] = &C_");
        string_add_str(classes_array, class_name);
        string_add_chars(classes_array, ",\n");

        String *forward_decls = new_string();
        String *gclass_def = new_string();

        string_add_chars(gclass_def, "const GlassClass C_");
        string_add_str(gclass_def, class_name);
        string_add_chars(gclass_def, " = {\n    {\n");

        for (size_t j = 0; j < list_len(func_names); j++) {
            const String *func_name = list_get(func_names, j);
            String *mangled_name = mangle_name(class_name, func_name);

            string_add_chars(forward_decls, "void ");
            string_add_str(forward_decls, mangled_name);
            string_add_chars(forward_decls, "(size_t);\n");

            string_add_chars(gclass_def, "        [NAME_");
            string_add_str(gclass_def, func_name);
            string_add_chars(gclass_def, "] = ");
            string_add_str(gclass_def, mangled_name);
            string_add_chars(gclass_def, ",\n");

            free_string(mangled_name);
        }

        string_add_chars(gclass_def, "    }\n};\n\n");

        string_add_str(code, forward_decls);
        string_add_str(code, gclass_def);

        free_string(gclass_def);
        free_string(forward_decls);
        free_list(func_names);
    }

    string_add_chars(classes_array, "};\n\n");
    string_add_str(code, classes_array);

    free_string(classes_array);
    free_list(class_names);
}

String *compile_classes(const Map *classes) {
    String *code = new_string();

    generate_name_enum(code, classes);
    add_runtime_library(code);
    add_builtin_funcs(code);
    generate_name_literals(code, classes);
    generate_string_literals(code, classes);
    generate_class_definitions(code, classes);
    generate_functions(code, classes);
    add_main_func(code);

    return code;
}
