#ifndef RENDER_H
#define RENDER_H

#include <psxgpu.h>

#define SCREEN_XRES 320
#define SCREEN_YRES 240
#define SCREEN_Z    320
#define CENTERX     (SCREEN_XRES >> 1)
#define CENTERY     (SCREEN_YRES >> 1)

#define OT_LENGTH      2048
#define BUFFER_LENGTH 24576

typedef struct {
    DISPENV disp_env;
    DRAWENV draw_env;
    uint32_t ot[OT_LENGTH];
    uint8_t  buffer[BUFFER_LENGTH];
} RenderBuffer;

typedef struct {
    RenderBuffer buffers[2];
    uint8_t      *next_packet;
    int          active_buffer;
} RenderContext;

void render_init();
void set_clear_color(uint8_t r, uint8_t g, uint8_t b);
void force_clear();
void swap_buffers();
void *get_next_prim();
void increment_prim(uint32_t size);
void sort_prim(void *prim, uint32_t otz);

uint32_t *get_ot_at(uint32_t otz);

void draw_text(int x, int y, int z, const char *text);

#endif
