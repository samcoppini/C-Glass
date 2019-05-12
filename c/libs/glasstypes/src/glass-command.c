#include "glasstypes/glass-command.h"
#include "utils/copy-interface.h"
#include "utils/string.h"

#include <stdlib.h>

void *copy_command(const GlassCommand *cmd) {
    GlassCommand *copy = malloc(sizeof(GlassCommand));
    copy->type = cmd->type;

    switch (cmd->type) {
        case CMD_DUPLICATE:
            copy->index = cmd->index;
            break;

        case CMD_PUSH_NAME:
        case CMD_PUSH_STR:
            copy->str = copy_string(cmd->str);
            break;

        case CMD_PUSH_NUM:
            copy->number = cmd->number;
            break;

        case CMD_LOOP_BEGIN:
        case CMD_LOOP_END:
            copy->index = cmd->index;
            copy->str = copy_string(cmd->str);
            break;

        case CMD_BUILTIN:
            copy->builtin = cmd->builtin;
            break;
        
        default:
            break;
    }

    return copy;
}

void free_command(GlassCommand *cmd) {
    switch (cmd->type) {
        case CMD_LOOP_BEGIN:
        case CMD_LOOP_END:
        case CMD_PUSH_NAME:
        case CMD_PUSH_STR:
            free_string(cmd->str);
            break;
        
        default:
            break;
    }

    free(cmd);
}

static void *copy_command_generic(const void *cmd) {
    return copy_command(cmd);
}

static void free_command_generic(void *cmd) {
    free_command(cmd);
}

const CopyInterface *CMD_COPY_OPS = &(CopyInterface) {
    copy_command_generic,
    free_command_generic,
};
