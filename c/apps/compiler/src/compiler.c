#include "compiler/compiler.h"
#include "glasstypes/glass-class.h"
#include "glasstypes/glass-command.h"
#include "glasstypes/glass-function.h"
#include "utils/list.h"
#include "utils/map.h"
#include "utils/set.h"
#include "utils/string.h"

#include <stdio.h>

extern const char *RUNTIME_LIBRARY[];

extern const size_t RUNTIME_LIBRARY_LINES;

extern const char *MAIN_FUNC[];

extern const size_t MAIN_FUNC_LINES;

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

void generate_name_enum(String *code, const Map *classes) {
    Set *name_set = get_all_names(classes);
    List *names = set_to_list(name_set);

    string_add_chars(code, "typedef enum Name {");

    for (size_t i = 0; i < list_len(names); i++) {
        string_add_chars(code, "\n    NAME_");
        string_add_str(code, list_get(names, i));
        string_add_char(code, ',');
    }

    string_add_chars(code, "    NUM_NAMES\n");
    string_add_chars(code, "\n} Name;\n\n");

    free_set(name_set);
    free_list(names);
}

void add_runtime_library(String *code) {
    for (size_t i = 0; i < RUNTIME_LIBRARY_LINES; i++) {
        string_add_chars(code, RUNTIME_LIBRARY[i]);
        string_add_char(code, '\n');
    }
}

void add_main_func(String *code) {
    for (size_t i = 0; i < MAIN_FUNC_LINES; i++) {
        string_add_chars(code, MAIN_FUNC[i]);
        string_add_char(code, '\n');
    }
}

void generate_function(String *code, const GlassClass *gclass, const GlassFunction *func) {
    const String *class_name = class_get_name(gclass);
    const String *func_name = func_get_name(func);
    String *mangled_name = mangle_name(class_name, func_name);

    string_add_chars(code, "void ");
    string_add_str(code, mangled_name);
    string_add_chars(code, "(size_t inst_index) {\n");

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
    generate_class_definitions(code, classes);
    generate_functions(code, classes);
    add_main_func(code);

    return code;
}
