#include <stdio.h>
#include "render.h"
#include "input.h"
#include "util.h"
#include "timer.h"

#include "screen_manager.h"
#include "screen/mainmenu.h"

typedef struct {
    char text_buffer[256];
    int32_t counter;
} mainmenu_data;

void
screen_mainmenu_load()
{
    mainmenu_data *data = screen_alloc(sizeof(mainmenu_data));
    data->counter = 0;
}

void
screen_mainmenu_unload(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;
}

void
screen_mainmenu_update(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;

    if(pad_pressed(PAD_UP)) data->counter++;
    if(pad_pressed(PAD_DOWN)) data->counter--;

    snprintf(data->text_buffer, 256,
             "Hello, world!\n"
             "Counter: %d\n"
             "Git commit ref: %s\n"
             "FPS: %02d",
             data->counter,
             GIT_COMMIT,
             get_frame_rate());
}

void
screen_mainmenu_draw(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;

    draw_text(10, 10, 0, data->text_buffer);
}
