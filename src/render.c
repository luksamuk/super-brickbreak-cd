#include "render.h"
#include <assert.h>
#include <psxgte.h>
#include <inline_c.h>

RenderContext ctx;

void
render_init()
{
    ResetGraph(0);
    
    FntLoad(960, 0);
    FntOpen(4, 12, 312, 16, 2, 256);

    SetDefDrawEnv(&ctx.buffers[0].draw_env, 0, 0,           SCREEN_XRES, SCREEN_YRES);
    SetDefDispEnv(&ctx.buffers[0].disp_env, 0, 0,           SCREEN_XRES, SCREEN_YRES);
    SetDefDrawEnv(&ctx.buffers[1].draw_env, 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);
    SetDefDispEnv(&ctx.buffers[1].disp_env, 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);

    set_clear_color(0, 0, 0);
    ctx.buffers[0].draw_env.isbg = 1;
    ctx.buffers[1].draw_env.isbg = 1;
    ctx.buffers[0].draw_env.dtd  = 1;
    ctx.buffers[1].draw_env.dtd  = 1;
    ctx.buffers[0].draw_env.dfe  = 1;
    ctx.buffers[1].draw_env.dfe  = 1;

    ctx.active_buffer = 0;
    ctx.next_packet   = ctx.buffers[0].buffer;
    ClearOTagR(ctx.buffers[0].ot, OT_LENGTH);

    InitGeom();
    gte_SetGeomOffset(CENTERX, CENTERY);
    gte_SetGeomScreen(SCREEN_Z);

    SetDispMask(1);

    force_clear();
}


void
force_clear()
{
    for(int i = 0; i < 2; i++) {
        TILE *tile = (TILE *)get_next_prim();
        setTile(tile);
        setRGB0(tile, 0, 0, 0);
        setXY0(tile, 0, 0);
        setWH(tile, SCREEN_XRES, SCREEN_YRES);
        sort_prim(tile, 0);
        increment_prim(sizeof(TILE));

        DrawOTagEnv(&ctx.buffers[ctx.active_buffer].ot[OT_LENGTH - 1],
                    &ctx.buffers[ctx.active_buffer].draw_env);
        PutDispEnv(&ctx.buffers[ctx.active_buffer].disp_env);

        DrawSync(0);
        VSync(0);

        ctx.active_buffer ^= 1;
        ctx.next_packet   = ctx.buffers[ctx.active_buffer].buffer;
        ClearOTagR(ctx.buffers[ctx.active_buffer].ot, OT_LENGTH);
    }
}

void
set_clear_color(uint8_t r, uint8_t g, uint8_t b)
{
    for(int i = 0; i < 2; i++) {
        setRGB0(&(ctx.buffers[i].draw_env), r, g, b);
    }
}

void
swap_buffers()
{
    DrawSync(0);
    VSync(0);
    RenderBuffer *draw_buffer = &ctx.buffers[ctx.active_buffer];
    RenderBuffer *disp_buffer = &ctx.buffers[ctx.active_buffer ^ 1];

    PutDispEnv(&disp_buffer->disp_env);
    DrawOTagEnv(&draw_buffer->ot[OT_LENGTH - 1], &draw_buffer->draw_env);

    ctx.active_buffer ^= 1;
    ctx.next_packet   = disp_buffer->buffer;

    ClearOTagR(disp_buffer->ot, OT_LENGTH);
}

void *
get_next_prim()
{
    return (void *)ctx.next_packet;
}

void
increment_prim(uint32_t size)
{
    ctx.next_packet += size;
}

void
sort_prim(void *prim, uint32_t otz)
{
    addPrim(get_ot_at(otz), (uint8_t *)prim);
    assert(ctx.next_packet <= &ctx.buffers[ctx.active_buffer].buffer[BUFFER_LENGTH]);
}

uint32_t *
get_ot_at(uint32_t otz)
{
    RenderBuffer *buffer = &ctx.buffers[ctx.active_buffer];
    return &buffer->ot[otz];
}

void
draw_text(int x, int y, int z, const char *text)
{
    ctx.next_packet = FntSort(get_ot_at(z), get_next_prim(), x, y, text);
    assert(ctx.next_packet <= &ctx.buffers[ctx.active_buffer].buffer[BUFFER_LENGTH]);
}

