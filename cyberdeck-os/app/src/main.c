#include "core/app.h"

int main(void) {
    App app;
    if (!App_Init(&app, "config.ini")) {
        return 1;
    }

    App_Run(&app);
    App_Shutdown(&app);
    return 0;
}
