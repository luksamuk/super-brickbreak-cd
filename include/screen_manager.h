#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <stdint.h>

typedef enum {
    SCREEN_MAINMENU,
    SCREEN_GAMEPLAY,
} ScreenIndex;

void screen_change(ScreenIndex src);

void screen_init();
void screen_load();
void screen_unload();
void screen_update();
void screen_draw();

void *screen_alloc(uint32_t size);
void screen_free();
void *screen_get_data();

#endif
