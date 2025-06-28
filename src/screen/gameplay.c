#include "render.h"
#include "input.h"
#include "util.h"
#include "timer.h"
#include <stdlib.h>
#include <stdio.h>
#include "cdda.h"
#include "strings.h"
#include "collision.h"

#include "screen_manager.h"
#include "screen/gameplay.h"

// Ball is 8bpp
// Ball texture: 320x0
// Ball CLUT: 0x480

// Paddle is 8bpp
// Paddle texture: 448x0
// Paddle CLUT 0x481

// Score:
// 10 digits
// Block base score: 10
// Level beaten score: 500
#define SCORE_BLOCK            10
#define SCORE_LEVEL_BEATEN    500
#define SCORE_MAX_MULTIPLER  0x10

#define PLAYER_MAX_LIVES 5

#define PADDLE_WIDTH  50
#define PADDLE_HEIGHT 5
#define PADDLE_MIN_WIDTH 30
#define PADDLE_MAX_WIDTH 200

#define PADDLE_BASE_SPEED   4
#define PADDLE_ACCEL_SPEED  8

#define BALL_RADIUS 3
#define BALL_TEXTURE_RADIUS      5
#define BALL_SPEED          0x3800

#define PADDLE_REBOUND_MIN_ANGLE    0x155 // approx. 30°
#define PADDLE_REBOUND_RANGE_ANGLE  0x555 // approx. 120°
// PADDLE_REBOUND_MIN_ANGLE + (P * PADDLE_REBOUND_RANGE_ANGLE)

#define BLOCK_WIDTH   16
#define BLOCK_HEIGHT   8

#define MAX_BLOCKS_WIDTH  20
#define MAX_BLOCKS_HEIGHT 18
#define MAX_BLOCKS (MAX_BLOCKS_WIDTH * MAX_BLOCKS_HEIGHT)

// Max level name size: 12
uint8_t level_layout[MAX_BLOCKS] = {
    0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,
    0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,
    1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,
    1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,
    1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,
    0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,
    0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,
    1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,
    1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,
    0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,
    0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t state;
} block_state;


typedef struct {
    int32_t ball_pos[2];    // fixpoint
    int32_t ball_vel[2];    // fixpoint
    int32_t paddle_pos[2];
    int32_t ball_init_angle; // fixpoint
    int16_t paddle_width;
    uint8_t ball_state;
    uint8_t player_lives;
    uint32_t player_score;
    uint8_t  player_score_multiplier;

    block_state blocks[MAX_BLOCKS];

    char txt[255];
} gameplay_data;


void
_respawn_ball(gameplay_data *data)
{
    // 0.125 a 0.375
    data->ball_init_angle = (rand() % 0x400) + 0x200;
    data->ball_vel[0] = 0;
    data->ball_vel[1] = 0;
    data->ball_state = 0;
    data->ball_pos[0] = (data->paddle_pos[0] + (data->paddle_width >> 1)) << 12;
    data->ball_pos[1] = (data->paddle_pos[1] - PADDLE_HEIGHT) << 12;
}

void
_reset_level(block_state *blocks)
{
    bzero(blocks, sizeof(uint8_t) * MAX_BLOCKS);
    for(uint16_t i = 0; i < MAX_BLOCKS; i++) {
        /* blocks[i].state = rand() % 2; */
        blocks[i].state = level_layout[i];
        blocks[i].r = 0x80 + (rand() % 0x80);
        blocks[i].g = 0x80 + (rand() % 0x80);
        blocks[i].b = 0x80 + (rand() % 0x80);
    }
}

void
screen_gameplay_load()
{
    srand(get_global_frames());
    gameplay_data *data = screen_alloc(sizeof(gameplay_data));
    data->paddle_pos[0] = CENTERX - (PADDLE_WIDTH >> 1);
    data->paddle_pos[1] = SCREEN_YRES - 25 - PADDLE_HEIGHT;
    data->paddle_width = PADDLE_WIDTH;
    data->player_score = 0;
    data->player_score_multiplier = 0;

    _reset_level(data->blocks);
    _respawn_ball(data);
    
    uint32_t file_length;
    TIM_IMAGE tim;
    uint8_t *file = file_read("\\BALL.TIM;1", &file_length);
    if(file) {
        load_texture(file, &tim);
        free(file);
    }

    file = file_read("\\OBJECTS.TIM;1", &file_length);
    if(file) {
        load_texture(file, &tim);
        free(file);
    }

    data->player_lives = PLAYER_MAX_LIVES;

    /* cdda_play_track(1); */
}

void
screen_gameplay_unload(void *d)
{
    gameplay_data *data = (gameplay_data *)d;
    (void)(data);
    screen_free();
    cdda_stop();
}

void
screen_gameplay_update(void *d)
{
    gameplay_data *data = (gameplay_data *)d;

    // Paddle movement
    int16_t paddle_speed =
        pad_pressing(PAD_SQUARE)
        ? PADDLE_ACCEL_SPEED
        : PADDLE_BASE_SPEED;

    if(pad_pressing(PAD_LEFT)) data->paddle_pos[0] -= paddle_speed;
    else if(pad_pressing(PAD_RIGHT)) data->paddle_pos[0] += paddle_speed;

    if(data->paddle_pos[0] < 0)
        data->paddle_pos[0] = 0;
    else if(data->paddle_pos[0] > SCREEN_XRES - data->paddle_width)
        data->paddle_pos[0] = SCREEN_XRES - data->paddle_width;

    // Ball movement
    if(!data->ball_state) {
        data->ball_vel[0] = 0;
        data->ball_vel[1] = 0;
        data->ball_pos[0] = (data->paddle_pos[0] + (data->paddle_width >> 1)) << 12;
        data->ball_pos[1] = (data->paddle_pos[1] - PADDLE_HEIGHT) << 12;

        if(pad_pressed(PAD_CROSS)) {
            data->ball_vel[0] = (BALL_SPEED * rcos(data->ball_init_angle)) >> 12;
            data->ball_vel[1] = -((BALL_SPEED * rsin(data->ball_init_angle)) >> 12);
            data->ball_state = 1;
        }
    } else {
        data->ball_pos[0] += data->ball_vel[0];
        data->ball_pos[1] += data->ball_vel[1];

        // Debug. TODO: remove
        if(pad_pressed(PAD_SELECT)) {
            _respawn_ball(data);
            _reset_level(data->blocks);
            data->paddle_width = PADDLE_WIDTH;
        }

        // Boundary collision
        int32_t ball_top = data->ball_pos[1] - (BALL_RADIUS << 12);
        int32_t ball_bottom = data->ball_pos[1] + (BALL_RADIUS << 12);
        if((ball_top < 0) && (data->ball_vel[1] < 0)) {
            data->ball_vel[1] *= -1;
            data->ball_pos[1] = BALL_RADIUS << 12;
        } else if(ball_top > (SCREEN_YRES << 12)) {
            // Respawn ball, and lose a life
            _respawn_ball(data);
            if(data->player_lives > 0) data->player_lives--;
            data->player_score_multiplier = 0;
        }

        int32_t ball_left = data->ball_pos[0] - (BALL_RADIUS << 12);
        int32_t ball_right = data->ball_pos[0] + (BALL_RADIUS << 12);
        if((ball_left < 0) && (data->ball_vel[0] < 0)) {
            data->ball_vel[0] *= -1;
            data->ball_pos[0] = BALL_RADIUS << 12;
        } else if((ball_right > (SCREEN_XRES << 12)) && (data->ball_vel[0] > 0)) {
            data->ball_vel[0] *= -1;
            data->ball_pos[0] = (SCREEN_XRES << 12) - (BALL_RADIUS << 12);
        }


        // Paddle collision
        if((ball_top < (data->paddle_pos[1] << 12))
           && (ball_bottom >= (data->paddle_pos[1] << 12))
           && (data->ball_vel[1] > 0)) {

            int32_t ball_hradius = (BALL_RADIUS << 11);
            
            // Check for X collision
            int32_t paddle_col_left =
                (data->paddle_pos[0] << 12) - ball_hradius;
            int32_t paddle_col_right =
                (data->paddle_pos[0] << 12) + (data->paddle_width << 12) + ball_hradius;

            if((data->ball_pos[0] >= paddle_col_left) && (data->ball_pos[0] <= paddle_col_right)) {
                data->ball_pos[1] = (data->paddle_pos[1] << 12) - (BALL_RADIUS << 12);

                // calculate paddle position factor
                uint32_t paddle_col_lradius = paddle_col_left + ball_hradius;
                uint32_t paddle_col_rradius = paddle_col_right - ball_hradius;
                uint32_t ball_paddle_range = paddle_col_rradius - paddle_col_lradius;
                uint32_t ball_relative_pos = data->ball_pos[0] - paddle_col_left;
                
                int32_t p = ONE - ((ball_relative_pos << 12) / ball_paddle_range);
                p = p < 0 ? 0 : (p > ONE ? ONE : p);

                uint32_t rebound_angle =
                    PADDLE_REBOUND_MIN_ANGLE + ((p * PADDLE_REBOUND_RANGE_ANGLE) >> 12);

                data->ball_vel[0] = (BALL_SPEED * rcos(rebound_angle)) >> 12;
                data->ball_vel[1] = -((BALL_SPEED * rsin(rebound_angle)) >> 12);

                data->player_score_multiplier = 0;
            }
        }

        // Block collision
        RECT box = {
            .x = 0,
            .y = 0,
            .w = BLOCK_WIDTH,
            .h = BLOCK_HEIGHT
        };
        vec2 ballpos = {
            .vx = data->ball_pos[0] >> 12,
            .vy = data->ball_pos[1] >> 12,
        };
        for(int16_t i = 0; i < MAX_BLOCKS; i++) {
            block_state *s = &data->blocks[i];
            if(s->state == 0) continue;

            int16_t y = i / MAX_BLOCKS_WIDTH;
            int16_t x = i - (y * MAX_BLOCKS_WIDTH);
            box.x = x << 4;
            box.y = y << 3;

            vec2 r_mov, r_pos;
            r_mov = r_pos = (vec2){ .vx = 0, .vy = 0 };
            if(collision_ball_box(&ballpos, BALL_RADIUS, &box,
                                  &r_mov, &r_pos)) {
                s->state = 0;

                // Horizontal collision
                if((r_mov.vx != 0) && (SIGNUM(data->ball_vel[0]) != SIGNUM(r_mov.vx))) {
                    data->ball_vel[0] *= -1;
                    data->ball_pos[0] = r_pos.vx << 12;
                }

                // Vertical collision
                if((r_mov.vy != 0) && (SIGNUM(data->ball_vel[1]) != SIGNUM(r_mov.vy))) {
                    data->ball_vel[1] *= -1;
                    data->ball_pos[1] = r_pos.vy << 12;
                }

                // Calculate score with multiplier
                if(data->player_score_multiplier > 1) {
                    data->player_score += SCORE_BLOCK * data->player_score_multiplier;
                } else data->player_score += SCORE_BLOCK;

                if(data->player_score_multiplier < SCORE_MAX_MULTIPLER)
                    data->player_score_multiplier =
                        (!data->player_score_multiplier)
                        ? 1
                        : (data->player_score_multiplier << 1);
                break;
            }
        }
    }

    // Changle paddle size
    if(pad_pressed(PAD_L1)) {
        data->paddle_width -= 10;
    }
    else if(pad_pressed(PAD_R1)) {
        data->paddle_width += 10;
    }
    data->paddle_width =
        (data->paddle_width < PADDLE_MIN_WIDTH)
        ? PADDLE_MIN_WIDTH
        : ((data->paddle_width > PADDLE_MAX_WIDTH)
           ? PADDLE_MAX_WIDTH
           : data->paddle_width);
}

void
_draw_ball(int16_t ball_x, int16_t ball_y)
{
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
           0,                        0,
           BALL_TEXTURE_RADIUS << 1, 0,
           0,                        BALL_TEXTURE_RADIUS << 1,
           BALL_TEXTURE_RADIUS << 1, BALL_TEXTURE_RADIUS << 1);
    sort_prim(poly, 2);
    increment_prim(sizeof(POLY_FT4));
}

#define darken(v, qtd) \
    ((v < qtd) ? 1 : (v - qtd))

void
_draw_block(block_state *s, int16_t vx, int16_t vy)
{
    POLY_F3 *poly;

    int16_t m_vx = vx + (BLOCK_WIDTH >> 1);
    int16_t m_vy = vy + (BLOCK_HEIGHT >> 1);

    // Mid color
    uint8_t
        mr = darken(s->r, 0x55),
        mg = darken(s->g, 0x55),
        mb = darken(s->b, 0x55),

        dr = darken(mr, 0x2a),
        dg = darken(mg, 0x2a),
        db = darken(mb, 0x2a);

    poly = (POLY_F3 *)get_next_prim();
    increment_prim(sizeof(POLY_F3));
    setPolyF3(poly);
    setRGB0(poly, s->r, s->g, s->b);
    setXY3(poly,
           vx, vy,
           vx + BLOCK_WIDTH, vy,
           m_vx, m_vy);
    sort_prim(poly, 2);

    poly = (POLY_F3 *)get_next_prim();
    increment_prim(sizeof(POLY_F3));
    setPolyF3(poly);
    setRGB0(poly, mr, mg, mb);
    setXY3(poly,
           vx, vy,
           m_vx, m_vy,
           vx, vy + BLOCK_HEIGHT);
    sort_prim(poly, 2);

    poly = (POLY_F3 *)get_next_prim();
    increment_prim(sizeof(POLY_F3));
    setPolyF3(poly);
    setRGB0(poly, mr, mg, mb);
    setXY3(poly,
           vx + BLOCK_WIDTH, vy,
           m_vx, m_vy,
           vx + BLOCK_WIDTH, vy + BLOCK_HEIGHT);
    sort_prim(poly, 2);

    poly = (POLY_F3 *)get_next_prim();
    increment_prim(sizeof(POLY_F3));
    setPolyF3(poly);
    setRGB0(poly, dr, dg, db);
    setXY3(poly,
           m_vx, m_vy,
           vx + BLOCK_WIDTH, vy + BLOCK_HEIGHT,
           vx, vy + BLOCK_HEIGHT);
    sort_prim(poly, 2);
}

void
_draw_paddle_part(int16_t vx, int16_t vy, uint8_t type, uint8_t flipx)
{
    POLY_FT4 *poly = (POLY_FT4 *)get_next_prim();
    increment_prim(sizeof(POLY_FT4));
    setPolyFT4(poly);
    setXYWH(poly, vx, vy, 10, 5);
    setRGB0(poly, 128, 128, 128);
    if(!flipx) {
        setUVWH(poly, type ? 10 : 0, 0, 10, 5);
    } else {
        uint8_t u0 = type ? 10 : 0;
        uint8_t v0 = 0;
        setUV4(poly,
               u0 + 9, v0,
               u0,      v0,
               u0 + 9, v0 + 5,
               u0,      v0 + 5);
    }
    setTPage(poly, 1, 0, 448, 0);
    setClut(poly, 0, 481);
    sort_prim(poly, 1);
}

void
_draw_paddle(int16_t vx, int16_t vy, int16_t width)
{
    // Determine the amount of parts for the paddle
    int16_t num_parts = (width / 10) - 2;

    // Draw borders
    _draw_paddle_part(vx, vy, 1, 0);              // Left border
    _draw_paddle_part(vx + width - 10, vy, 1, 1); // Right border
    for(int16_t i = 0; i < num_parts; i++) {
        int16_t xpos = (vx + 10) + (10 * i);
        _draw_paddle_part(xpos, vy, 0, 0);
    }
}

void
screen_gameplay_draw(void *d)
{
    gameplay_data *data = (gameplay_data *)d;

    // Draw paddle
    _draw_paddle(data->paddle_pos[0], data->paddle_pos[1], data->paddle_width);

    // Draw ball
    _draw_ball(data->ball_pos[0] >> 12, data->ball_pos[1] >> 12);

    for(int16_t i = 0; i < MAX_BLOCKS; i++) {
        block_state *s = &data->blocks[i];
        if(!s->state) continue;
        int16_t y = i / MAX_BLOCKS_WIDTH;
        int16_t x = i - (y * MAX_BLOCKS_WIDTH);
        _draw_block(s, x << 4, y << 3);
    }

    draw_text(10, SCREEN_YRES - 18, 0, "AAAAAAAAAAAA");

    snprintf(data->txt, 255, "%010u", data->player_score);
    draw_text(CENTERX - 40, SCREEN_YRES - 18, 0, data->txt);

    int16_t lives_x = SCREEN_XRES - BALL_RADIUS - 5;

    if(data->player_score_multiplier > 1) {
        snprintf(data->txt, 255, "x%d", data->player_score_multiplier);
        draw_text(
            (lives_x - ((BALL_RADIUS << 1) + 2) * PLAYER_MAX_LIVES) - (strlen(data->txt) * 8),
            SCREEN_YRES - 18,
            0,
            data->txt);
    }

    // Draw lives
    for(uint8_t i = 0; i < data->player_lives; i++) {
        _draw_ball(lives_x, SCREEN_YRES - 15);
        lives_x -= (BALL_RADIUS << 1) + 2;
    }
}

