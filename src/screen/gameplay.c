#include "render.h"
#include "input.h"
#include "util.h"
#include <stdlib.h>

#include "screen_manager.h"
#include "screen/gameplay.h"

// Ball is 8bpp
// Ball texture: 320x0
// Ball CLUT: 0x480

#define PADDLE_WIDTH ((SCREEN_XRES << 9) >> 12)
#define PADDLE_HEIGHT 10

#define PADDLE_BASE_SPEED   4
#define PADDLE_ACCEL_SPEED  8


#define BALL_RADIUS 5

typedef struct {
    int32_t ball_pos[2];
    int32_t ball_vel[2];
    int16_t paddle_pos[2];
    uint8_t ball_state;
} gameplay_data;

void
screen_gameplay_load()
{
    gameplay_data *data = screen_alloc(sizeof(gameplay_data));
    data->paddle_pos[0] = CENTERX - (PADDLE_WIDTH >> 1);
    data->paddle_pos[1] = SCREEN_YRES - 30 - PADDLE_HEIGHT;

    data->ball_pos[0] = CENTERX << 12;
    data->ball_pos[1] = CENTERY << 12;

    data->ball_vel[0] = 0;
    data->ball_vel[1] = 0;
    data->ball_state = 0;
    
    uint32_t file_length;
    TIM_IMAGE tim;
    uint8_t *file = file_read("\\BALL.TIM;1", &file_length);
    if(file) {
        load_texture(file, &tim);
        free(file);
    }
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

    if(!data->ball_state) {
        data->ball_vel[0] = 0;
        data->ball_vel[1] = 0;
        data->ball_pos[0] = (data->paddle_pos[0] + (PADDLE_WIDTH >> 1)) << 12;
        data->ball_pos[1] = (data->paddle_pos[1] - PADDLE_HEIGHT) << 12;
    }
    
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

    // Draw ball
    int16_t
        ball_x = data->ball_pos[0] >> 12,
        ball_y = data->ball_pos[1] >> 12;
    POLY_FT4 *poly = (POLY_FT4 *)get_next_prim();
    setPolyFT4(poly);
    setRGB0(poly, 0x7f, 0x7f, 0x7f);
    setXY4(poly,
           ball_x - BALL_RADIUS,
           ball_y - BALL_RADIUS,
           ball_x + BALL_RADIUS,
           ball_y - BALL_RADIUS,
           ball_x - BALL_RADIUS,
           ball_y + BALL_RADIUS,
           ball_x + BALL_RADIUS,
           ball_y + BALL_RADIUS);
    setTPage(poly, 1, 0, 320, 0);
    setClut(poly, 0, 480);
    setUV4(poly,
           0, 0,
           10, 0,
           0, 10,
           10, 10);
    sort_prim(poly, 2);
    increment_prim(sizeof(POLY_FT4));

    draw_text(10, SCREEN_YRES - 20, 0, "Level: Test");
}
