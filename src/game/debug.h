#ifndef DEBUG_H
#define DEBUG_H

#include <PR/ultratypes.h>

enum DebugPage {
    DEBUG_PAGE_OBJECTINFO,       // 0: objectinfo
    DEBUG_PAGE_CHECKSURFACEINFO, // 1: checkinfo/surfaceinfo
    DEBUG_PAGE_MAPINFO,          // 2: mapinfo
    DEBUG_PAGE_STAGEINFO,        // 3: stageinfo
    DEBUG_PAGE_EFFECTINFO,       // 4: effectinfo
    DEBUG_PAGE_ENEMYINFO         // 5: enemyinfo
};

enum {
    DEBUG_TEXT_ANG,
    DEBUG_TEXT_SPD,
    DEBUG_TEXT_STA,
#if 0
    DEBUG_TEXT_MEM,
    DEBUG_TEXT_BUF,
#endif
    DEBUG_TEXT_COUNT,
    DEBUG_TEXT_MAX = DEBUG_TEXT_COUNT - 1
};

#define DEBUG_TEXT_Y(name) ((DEBUG_TEXT_MAX - (DEBUG_TEXT_##name)) * 16 + 20)

s64 get_current_clock(void);
s64 get_clock_difference(UNUSED s64 cycles);
void set_text_array_x_y(s32 xOffset, s32 yOffset);
void print_debug_top_down_objectinfo(const char *str, s32 number);
void print_debug_top_down_mapinfo(const char * str, s32 number);
void print_debug_bottom_up(const char *str, s32 number);
void debug_unknown_level_select_check(void);
void reset_debug_objectinfo(void);
void stub_debug_5(void);
void try_print_debug_mario_object_info(void);
void try_do_mario_debug_object_spawn(void);
void try_print_debug_mario_level_info(void);

#endif // DEBUG_H
