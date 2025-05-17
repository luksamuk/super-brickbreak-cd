#include "cdda.h"
#include <stdio.h>

static int    num_toc;
static CdlLOC toc_loc[20];

static uint8_t current_track = 0xff;

void
_cdda_loop_callback()
{
    if(current_track > num_toc) {
        CdControlF(CdlPause, 0);
        return;
    }
    CdControlF(CdlSetloc, &toc_loc[current_track]);
    CdControlF(CdlPlay, 0);
}

void
cdda_init()
{
    CdAutoPauseCallback(_cdda_loop_callback);

    SpuSetCommonMasterVolume(0x3fff, 0x3fff);
    SpuSetCommonCDVolume(0x3fff, 0x3fff);

    while((num_toc = CdGetToc(toc_loc)) == 0) {
        printf("CD TOC not found, please insert a CD-DA disc.\n");
    }

    printf("Found %u entries on CD TOC. Aligning locations...\n", num_toc);
    for(uint8_t i = 0; i < num_toc; i++) {
        CdIntToPos(CdPosToInt(&toc_loc[i]) - 74, &toc_loc[i]);
        printf("Entry #%u: Track=%d, Minute=%d, Second=%d, Sector=%d\n",
               i, toc_loc[i].track, toc_loc[i].minute, toc_loc[i].second,
               toc_loc[i].sector);
    }
}

void
cdda_play_track(uint8_t track)
{
    if(track > num_toc) {
        printf("Error setting track: Unknown track! Max value: %u\n", num_toc);
        return;
    }

    // Set mode to report playback data, play CDDA and enable auto-pause callback
    uint8_t mode = CdlModeRept | CdlModeDA | CdlModeAP;

    current_track = track;
    CdSync(0, 0);
    CdControl(CdlSetmode, &mode, 0);
    CdControl(CdlSetloc, &toc_loc[track], 0);
    CdControl(CdlPlay, 0, 0);
}

void
cdda_stop()
{
    CdControl(CdlPause, 0, 0);
    CdSync(0, 0);
    current_track = 0xff;
}
