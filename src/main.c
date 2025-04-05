#include <stdlib.h>
#include <stdio.h>
#include <psxcd.h>

#include "render.h"
#include "input.h"
#include "screen_manager.h"
#include "timer.h"

uint8_t paused = 0;

int
main(void)
{
    render_init();
    CdInit();
    pad_init();
    screen_init();
    timer_init();
    printf("Hello world! From console\n");

    screen_change(SCREEN_MAINMENU);
    
    while(1) {
        timer_update();
        pad_update();
        screen_update();
        screen_draw();
        swap_buffers();
    }
    return 0;
}
