#include <stdio.h>
#include <string.h>
#include "render.h"
#include "input.h"
#include "util.h"
#include "timer.h"

#include "screen_manager.h"
#include "screen/mainmenu.h"

#define NUM_OPTIONS 2

static char *options[] = {
    "New Game",
    "Options"
};

typedef struct {
    char text_buffer[80];
    int8_t selection;
    int16_t menu_x;
} mainmenu_data;

void
screen_mainmenu_load()
{
    mainmenu_data *data = screen_alloc(sizeof(mainmenu_data));
    data->selection = 0;

    int16_t max_size = 2;
    for(int i = 0; i < NUM_OPTIONS; i++) {
        int16_t len = strlen(options[i]) + 2;
        if(len > max_size) max_size = len;
    }
    data->menu_x = (SCREEN_XRES - (max_size << 3)) >> 1;
}

void
screen_mainmenu_unload(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;
    screen_free();
}

void
screen_mainmenu_update(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;

    if(pad_pressed(PAD_UP)) data->selection--;
    else if(pad_pressed(PAD_DOWN)) data->selection++;
    data->selection =
        (data->selection < 0)
        ? (NUM_OPTIONS - 1)
        : data->selection % NUM_OPTIONS;

    if(pad_pressed(PAD_CROSS) || pad_pressed(PAD_START)) {
        switch(data->selection) {
        case 0:
            screen_change(SCREEN_GAMEPLAY);
            break;
        default: break;
        }
    }
}

void
screen_mainmenu_draw(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;

    draw_text(84, 60, 0, "SUPER BRICKBREAK CD");

    for(int i = 0; i < NUM_OPTIONS; i++) {
        snprintf(data->text_buffer, 80, "%c %s",
                 (data->selection == i) ? '>' : ' ',
                 options[i]);
        draw_text(data->menu_x, 96 + (12 * i), 0, data->text_buffer);
    }
}
