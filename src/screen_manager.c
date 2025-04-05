#include "screen_manager.h"
#include "memalloc.h"
#include "render.h"

#include <stdint.h>

#include "screen/mainmenu.h"
#include "screen/gameplay.h"


// 64k of RAM reserved for screen storage
#define SCREEN_BUFFER_LENGTH 65536
static uint8_t *screen_data[SCREEN_BUFFER_LENGTH] = { 0 };
static ArenaAllocator screen_arena;

static int8_t current_screen = -1;

void
screen_init()
{
    alloc_arena_init(&screen_arena, screen_data, SCREEN_BUFFER_LENGTH);
}

static void
_render_loading_text()
{
    set_clear_color(0, 0, 0);
    swap_buffers();
    force_clear();
    draw_text(208, 220, 0, "Now Loading...");
    swap_buffers();
}

void
screen_change(ScreenIndex src)
{
    _render_loading_text();
    if(current_screen >= 0)
        screen_unload();
    current_screen = src;
    screen_load();
    screen_update();
}

void
screen_load()
{
    switch(current_screen) {
    case SCREEN_MAINMENU: screen_mainmenu_load(); break;
    case SCREEN_GAMEPLAY: screen_gameplay_load(); break;
    default: break;
    }
}

void
screen_unload()
{
    switch(current_screen) {
    case SCREEN_MAINMENU: screen_mainmenu_unload(screen_data); break;
    case SCREEN_GAMEPLAY: screen_gameplay_unload(screen_data); break;
    default: break;
    }
}

void
screen_update()
{
    switch(current_screen) {
    case SCREEN_MAINMENU: screen_mainmenu_update(screen_data); break;
    case SCREEN_GAMEPLAY: screen_gameplay_update(screen_data); break;
    default: break;
    }
}

void
screen_draw()
{
    switch(current_screen) {
    case SCREEN_MAINMENU: screen_mainmenu_draw(screen_data); break;
    case SCREEN_GAMEPLAY: screen_gameplay_draw(screen_data); break;
    default: break;
    }
}

void *
screen_alloc(uint32_t size)
{
    return alloc_arena_malloc(&screen_arena, size);
}

void
screen_free()
{
    alloc_arena_free(&screen_arena);
}

void *
screen_get_data()
{
    return screen_data;
}

