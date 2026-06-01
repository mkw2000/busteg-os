#ifndef CYBERDECK_MODULE_MANAGER_H
#define CYBERDECK_MODULE_MANAGER_H

#include <stdbool.h>
#include "core/event_system.h"

struct App;

#define MODULE_MANAGER_MAX_MODULES 8

/*
 * A Module is one app screen/tool.
 *
 * Only render() is required. The other function pointers can be NULL when a
 * module does not need setup, per-frame updates, input handling, or cleanup.
 */
typedef struct Module {
    const char *name;
    bool (*init)(struct Module *module, struct App *app);
    void (*update)(struct Module *module, struct App *app, float dt);
    void (*render)(struct Module *module, struct App *app);
    void (*handle_event)(struct Module *module, struct App *app, NavEvent event);
    void (*shutdown)(struct Module *module, struct App *app);
    void *data;
} Module;

typedef struct ModuleManager {
    Module modules[MODULE_MANAGER_MAX_MODULES];
    int count;
} ModuleManager;

void ModuleManager_Init(ModuleManager *manager);
bool ModuleManager_Add(ModuleManager *manager, Module module);
void ModuleManager_InitModules(ModuleManager *manager, struct App *app);
void ModuleManager_Update(ModuleManager *manager, struct App *app, float dt);
void ModuleManager_ShutdownModules(ModuleManager *manager, struct App *app);
Module *ModuleManager_Get(ModuleManager *manager, int index);

#endif
