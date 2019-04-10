/**
 * Copyright 2018 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "shell/opt.h"

static struct carbon_cmdopt *option_by_name(struct carbon_cmdopt_mgr *manager, const char *name);

bool carbon_cmdopt_mgr_create(struct carbon_cmdopt_mgr *manager, char *module_name, char *module_desc,
                              enum carbon_mod_arg_policy policy, bool (*fallback)(int argc, char **argv, FILE *file,
                                                                               struct carbon_cmdopt_mgr *manager))
{
    NG5_NON_NULL_OR_ERROR(manager)
    NG5_NON_NULL_OR_ERROR(module_name)
    NG5_NON_NULL_OR_ERROR(fallback)
    manager->module_name = strdup(module_name);
    manager->module_desc = module_desc ? strdup(module_desc) : NULL;
    manager->policy = policy;
    manager->fallback = fallback;
    NG5_CHECK_SUCCESS(carbon_vec_create(&manager->groups, NULL, sizeof(struct carbon_cmdopt_group), 5));
    return true;
}

bool carbon_cmdopt_mgr_drop(struct carbon_cmdopt_mgr *manager)
{
    NG5_NON_NULL_OR_ERROR(manager);
    for (size_t i = 0; i < manager->groups.num_elems; i++) {
        struct carbon_cmdopt_group *cmdGroup = vec_get(&manager->groups, i, struct carbon_cmdopt_group);
        for (size_t j = 0; j < cmdGroup->cmd_options.num_elems; j++) {
            struct carbon_cmdopt *option = vec_get(&cmdGroup->cmd_options, j, struct carbon_cmdopt);
            free(option->opt_name);
            free(option->opt_desc);
            free(option->opt_manfile);
        }
        NG5_CHECK_SUCCESS(carbon_vec_drop(&cmdGroup->cmd_options));
        free(cmdGroup->desc);
    }

    NG5_CHECK_SUCCESS(carbon_vec_drop(&manager->groups));
    free(manager->module_name);
    if (manager->module_desc) {
        free(manager->module_desc);
    }
    return true;
}

bool carbon_cmdopt_mgr_process(struct carbon_cmdopt_mgr *manager, int argc, char **argv, FILE *file)
{
    NG5_NON_NULL_OR_ERROR(manager)
    NG5_NON_NULL_OR_ERROR(argv)
    NG5_NON_NULL_OR_ERROR(file)

    if (argc == 0) {
        if (manager->policy == NG5_MOD_ARG_REQUIRED) {
            carbon_cmdopt_mgr_show_help(file, manager);
        } else {
            return manager->fallback(argc, argv, file, manager);
        }
    } else {
        const char *arg = argv[0];
        struct carbon_cmdopt *option = option_by_name(manager, arg);
        if (option) {
            return option->callback(argc - 1, argv + 1, file);
        } else {
            return manager->fallback(argc, argv, file, manager);
        }
    }

    return true;
}

bool carbon_cmdopt_mgr_create_group(struct carbon_cmdopt_group **group,
                                    const char *desc,
                                    struct carbon_cmdopt_mgr *manager)
{
    NG5_NON_NULL_OR_ERROR(group)
    NG5_NON_NULL_OR_ERROR(desc)
    NG5_NON_NULL_OR_ERROR(manager)
    struct carbon_cmdopt_group *cmdGroup = VECTOR_NEW_AND_GET(&manager->groups, struct carbon_cmdopt_group);
    cmdGroup->desc = strdup(desc);
    NG5_CHECK_SUCCESS(carbon_vec_create(&cmdGroup->cmd_options, NULL, sizeof(struct carbon_cmdopt), 10));
    *group = cmdGroup;
    return true;
}

bool carbon_cmdopt_group_add_cmd(struct carbon_cmdopt_group *group, const char *opt_name, char *opt_desc, char *opt_manfile,
                                 int (*callback)(int argc, char **argv, FILE *file))
{
    NG5_NON_NULL_OR_ERROR(group)
    NG5_NON_NULL_OR_ERROR(opt_name)
    NG5_NON_NULL_OR_ERROR(opt_desc)
    NG5_NON_NULL_OR_ERROR(opt_manfile)
    NG5_NON_NULL_OR_ERROR(callback)

    struct carbon_cmdopt *command = VECTOR_NEW_AND_GET(&group->cmd_options, struct carbon_cmdopt);
    command->opt_desc = strdup(opt_desc);
    command->opt_manfile = strdup(opt_manfile);
    command->opt_name = strdup(opt_name);
    command->callback = callback;

    return true;
}

bool carbon_cmdopt_mgr_show_help(FILE *file, struct carbon_cmdopt_mgr *manager)
{
    NG5_NON_NULL_OR_ERROR(file)
    NG5_NON_NULL_OR_ERROR(manager)

    if (manager->groups.num_elems > 0) {
        fprintf(file, "usage: %s <command> %s\n\n", manager->module_name,
                (manager->policy == NG5_MOD_ARG_REQUIRED ? "<args>" :
                 manager->policy == NG5_MOD_ARG_MAYBE_REQUIRED ? "[<args>]":
                 ""));

        if (manager->module_desc) {
            fprintf(file, "%s\n\n", manager->module_desc);
        }
        fprintf(file, "These are common commands used in various situations:\n\n");
        for (size_t i = 0; i < manager->groups.num_elems; i++) {
            struct carbon_cmdopt_group *cmdGroup = vec_get(&manager->groups, i, struct carbon_cmdopt_group);
            fprintf(file, "%s\n", cmdGroup->desc);
            for (size_t j = 0; j < cmdGroup->cmd_options.num_elems; j++) {
                struct carbon_cmdopt *option = vec_get(&cmdGroup->cmd_options, j, struct carbon_cmdopt);
                fprintf(file, "   %-15s%s\n", option->opt_name, option->opt_desc);
            }
            fprintf(file, "\n");
        }
        fprintf(file, "\n'%s help' show this help, and '%s help <command>' to open \nmanpage of specific command.\n",
                manager->module_name, manager->module_name);
    } else {
        fprintf(file, "usage: %s %s\n\n", manager->module_name,
                (manager->policy == NG5_MOD_ARG_REQUIRED ? "<args>" :
                 manager->policy == NG5_MOD_ARG_MAYBE_REQUIRED ? "[<args>]":
                 ""));

        fprintf(file, "%s\n\n", manager->module_desc);
    }

    return true;
}

static struct carbon_cmdopt *option_by_name(struct carbon_cmdopt_mgr *manager, const char *name)
{
    for (size_t i = 0; i < manager->groups.num_elems; i++) {
        struct carbon_cmdopt_group *cmdGroup = vec_get(&manager->groups, i, struct carbon_cmdopt_group);
        for (size_t j = 0; j < cmdGroup->cmd_options.num_elems; j++) {
            struct carbon_cmdopt *option = vec_get(&cmdGroup->cmd_options, j, struct carbon_cmdopt);
            if (strcmp(option->opt_name, name) == 0) {
                return option;
            }
        }
    }
    return NULL;
}