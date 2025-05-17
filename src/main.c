#include <stdlib.h>
#include <stdio.h>
#include <psxcd.h>
#include <psxspu.h>

#include "render.h"
#include "input.h"
#include "screen_manager.h"
#include "timer.h"
#include "cdda.h"

// Name: Best Game Console
// Author: DJARTMUSIC
// https://pixabay.com/music/video-games-best-game-console-301284/

uint8_t paused = 0;

int
main(void)
{
    render_init();
    SpuInit();
    CdInit();
    pad_init();
    screen_init();
    timer_init();

    cdda_init();

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
