#include "util.h"
#include <inline_c.h>
#include <psxcd.h>
#include <psxgpu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int
RotAverageNclip3(
    SVECTOR *a, SVECTOR *b, SVECTOR *c,
    uint32_t *xy0, uint32_t *xy1, uint32_t *xy2,
    int *otz)
{
    int nclip = 0;

    gte_ldv0(a);
    gte_ldv1(b);
    gte_ldv2(c);
    gte_rtpt();
    gte_nclip();
    gte_stopz(&nclip);
    if(nclip <= 0) goto exit;
    gte_stsxy3(xy0, xy1, xy2);
    gte_avsz3();
    gte_stotz(otz);
exit:
    return nclip;
}

int
RotAverageNclip4(
    SVECTOR *a, SVECTOR *b, SVECTOR *c, SVECTOR *d,
    uint32_t *xy0, uint32_t *xy1, uint32_t *xy2, uint32_t *xy3,
    int *otz)
{
    int nclip = 0;

    gte_ldv0(a);
    gte_ldv1(b);
    gte_ldv2(c);
    gte_rtpt();
    gte_nclip();
    gte_stopz(&nclip);
    if(nclip <= 0) goto exit;
    gte_stsxy0(xy0);
    gte_ldv0(d);
    gte_rtps();
    gte_stsxy3(xy1, xy2, xy3);
    gte_avsz4();
    gte_stotz(otz);
exit:
    return nclip;
}

int
RotTransPers(SVECTOR *v, uint32_t *xy0)
{
    int otz = 0;
    gte_ldv0(v);
    gte_rtps();
    gte_stsxy0(xy0);
    gte_stotz(&otz);
    return otz;
}

void
CrossProduct0(VECTOR *v0, VECTOR *v1, VECTOR *out)
{
    gte_ldopv1(v0);
    gte_ldopv2(v1);
    gte_op0();
    gte_stlvnl(out);
}

void
CrossProduct12(VECTOR *v0, VECTOR *v1, VECTOR *out)
{
    gte_ldopv1(v0);
    gte_ldopv2(v1);
    gte_op12();
    gte_stlvnl(out);
}

uint8_t *
file_read(const char *filename, uint32_t *length)
{
    CdlFILE filepos;
    int numsectors;
    uint8_t *buffer;
    buffer = NULL;

    if(CdSearchFile(&filepos, filename) == NULL) {
        printf("File %s not found!\n", filename);
        return NULL;
    }

    numsectors = (filepos.size + 2047) / 2048;
    buffer = (uint8_t *) malloc(2048 * numsectors);
    if(!buffer) {
        printf("Error allocating %d sectors.\n", numsectors);
        return NULL;
    }

    CdControl(CdlSetloc, (uint8_t *) &filepos.pos, 0);
    CdRead(numsectors, (uint32_t *) buffer, CdlModeSpeed);
    CdReadSync(0, 0);

    *length = filepos.size;
    return buffer;
}

void
load_texture(uint8_t *data, TIM_IMAGE *tim)
{
    GetTimInfo((const uint32_t *)data, tim);
    LoadImage(tim->prect, tim->paddr);
    DrawSync(0);
    if(tim->mode & 0x8) {
        LoadImage(tim->crect, tim->caddr);
        DrawSync(0);
    }
}


uint8_t
get_byte(uint8_t *bytes, uint32_t *b)
{
    return (uint8_t) bytes[(*b)++];
}

uint16_t
get_short_be(uint8_t *bytes, uint32_t *b)
{
    uint16_t value = 0;
    value |= bytes[(*b)++] << 8;
    value |= bytes[(*b)++];
    return value;
}

uint16_t
get_short_le(uint8_t *bytes, uint32_t *b)
{
    uint16_t value = 0;
    value |= bytes[(*b)++];
    value |= bytes[(*b)++] << 8;
    return value;
}

uint32_t
get_long_be(uint8_t *bytes, uint32_t *b)
{
    uint32_t value = 0;
    value |= bytes[(*b)++] << 24;
    value |= bytes[(*b)++] << 16;
    value |= bytes[(*b)++] << 8;
    value |= bytes[(*b)++];
    return value;
}


uint32_t
adler32(const char *s)
{
    uint32_t a = 0x0001, b = 0x0000;
    const char *i = s;
    while(*i != '\0') {
        a = (a + *i) % 0xfff1;
        b = (a + b) % 0xfff1;
        i++;
    }
    return (b << 16) | a;
}

int32_t
div12(int32_t a, int32_t b)
{
    return ((a << 12) / b) & ~(uint32_t)0xfff;
}

int32_t
floor12(int32_t a)
{
    return a & ~(uint32_t)0xfff;
}
