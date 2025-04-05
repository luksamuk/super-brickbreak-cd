#include "render.h"
#include "input.h"
#include <stdio.h>

#include "screen_manager.h"
#include "screen/gameplay.h"

#define PADDLE_WIDTH ((SCREEN_XRES << 9) >> 12)
#define PADDLE_HEIGHT 10

#define PADDLE_BASE_SPEED   4
#define PADDLE_ACCEL_SPEED  8

typedef struct {
    int16_t paddle_pos[2];
} gameplay_data;

void
screen_gameplay_load()
{
    gameplay_data *data = screen_alloc(sizeof(gameplay_data));
    data->paddle_pos[0] = CENTERX - (PADDLE_WIDTH >> 1);
    data->paddle_pos[1] = SCREEN_YRES - 30 - PADDLE_HEIGHT;
}

void
screen_gameplay_unload(void *d)
{
    gameplay_data *data = (gameplay_data *)d;
    screen_free();
}

void
screen_gameplay_update(void *d)
{
    gameplay_data *data = (gameplay_data *)d;

    int16_t paddle_speed =
        pad_pressing(PAD_SQUARE)
        ? PADDLE_ACCEL_SPEED
        : PADDLE_BASE_SPEED;
    
    if(pad_pressing(PAD_LEFT)) data->paddle_pos[0] -= paddle_speed;
    else if(pad_pressing(PAD_RIGHT)) data->paddle_pos[0] += paddle_speed;

    if(data->paddle_pos[0] < 0)
        data->paddle_pos[0] = 0;
    else if(data->paddle_pos[0] > SCREEN_XRES - PADDLE_WIDTH)
        data->paddle_pos[0] = SCREEN_XRES - PADDLE_WIDTH;

    // ---
    
}

void
screen_gameplay_draw(void *d)
{
    gameplay_data *data = (gameplay_data *)d;

    // Draw paddle
    TILE *tile = (TILE *)get_next_prim();
    setTile(tile);
    setRGB0(tile, 0xff, 0xff, 0xff);
    setXY0(tile, data->paddle_pos[0], data->paddle_pos[1]);
    setWH(tile, PADDLE_WIDTH, PADDLE_HEIGHT);
    sort_prim(tile, 1);
    increment_prim(sizeof(TILE));

    draw_text(10, SCREEN_YRES - 20, 0, "Level: Test");
}
