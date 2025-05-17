#ifndef CDDA_H
#define CDDA_H

#include <psxcd.h>
#include <psxspu.h>

void cdda_init();
void cdda_play_track(uint8_t track);
void cdda_stop();

#endif
