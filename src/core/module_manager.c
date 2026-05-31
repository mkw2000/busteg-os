#include "core/module_manager.h"
#include <stddef.h>

void ModuleManager_Init(ModuleManager *manager) {
    manager->count = 0;
}

bool ModuleManager_Add(ModuleManager *manager, Module module) {
    if (manager->count >= 8) {
        return false;
    }
    manager->modules[manager->count++] = module;
    return true;
}

void ModuleManager_InitModules(ModuleManager *manager, struct App *app) {
    for (int i = 0; i < manager->count; ++i) {
        if (manager->modules[i].init) {
            manager->modules[i].init(&manager->modules[i], app);
        }
    }
}

void ModuleManager_Update(ModuleManager *manager, struct App *app, float dt) {
    for (int i = 0; i < manager->count; ++i) {
        if (manager->modules[i].update) {
            manager->modules[i].update(&manager->modules[i], app, dt);
        }
    }
}

void ModuleManager_ShutdownModules(ModuleManager *manager, struct App *app) {
    for (int i = manager->count - 1; i >= 0; --i) {
        if (manager->modules[i].shutdown) {
            manager->modules[i].shutdown(&manager->modules[i], app);
        }
    }
}

Module *ModuleManager_Get(ModuleManager *manager, int index) {
    if (index < 0 || index >= manager->count) {
        return NULL;
    }
    return &manager->modules[index];
}
