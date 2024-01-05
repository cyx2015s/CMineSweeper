#ifndef MACRO_H_
#define MACRO_H_

#define PAGE_TITLE 0
#define PAGE_SETTING 1
#define PAGE_GAME_ENTER 2
#define PAGE_GAME 3
#define PAGE_ABOUT 4
#define PAGE_EXIT 5

#define GET_MINE 0
#define GET_OPEN 1
#define GET_FLAG 2
#define GET_COUNT 3

#define FIRST_CLICK 1
#define PLAYING 1

#define CSI "\033["
#define CLR_SCR "2J"

#define MOUSE_IDLE 0
#define MOUSE_LEFTUP 1
#define MOUSE_RIGHTDOWN 2
#define MOUSE_MIDDLEUP 3

#define TEXT_FLAG "旗"
#define TEXT_MINE "雷"
#define TEXT_COVER "▉▉"
#define TEXT_CENTER "<>"

#define GET_MOUSE_BY_EVENT 1

char text_num[][4] = {"",   "一", "二", "三", "四", "五", "六", "七", "八",
                      "九", "十", "11", "12", "13", "14", "15", "16", "17",
                      "18", "19", "20", "21", "22", "23", "24", "25", "26",
                      "27", "28", "29", "30", "31", "32", "33", "34", "35",
                      "36", "37", "38", "39", "40", "41", "42", "43", "44",
                      "45", "46", "47", "48", "49", "50"};

int color_num[][3] = {{32, 128, 192}, {0, 0, 128},   {0, 128, 0},
                      {128, 0, 0},    {128, 0, 128}, {128, 128, 0},
                      {0, 128, 128},  {32, 32, 32}};
char yesno[][4] = {"否", "是"};

extern int mouse_x, mouse_y, page, mouse_action;

#define BUTTON(TEXT, X, Y, FUNC)                                          \
    do {                                                                  \
        printf(CSI "0m" CSI "7m" CSI "%dG" CSI "%dd%s" CSI "0m", (X) + 1, \
               (Y) + 1, TEXT);                                            \
        if (mouse_action == MOUSE_LEFTUP && mouse_x >= X &&               \
            mouse_x < X + strlen(TEXT) && mouse_y == Y) {                 \
            force_flush = 1;                                              \
            do                                                            \
                FUNC while (0);                                           \
        }                                                                 \
    } while (0)

#define FOR_NBR(DX, DY)                                              \
    for (int DX##_##DY##_##i = 0, DX = dx[mask_id][DX##_##DY##_##i], \
             DY = dy[mask_id][DX##_##DY##_##i];                      \
         DX##_##DY##_##i < dcard[mask_id];                           \
         DX##_##DY##_##i++, DX = dx[mask_id][DX##_##DY##_##i],       \
             DY = dy[mask_id][DX##_##DY##_##i])

#define FOR_MAP(X, Y)                \
    for (int Y = 0; Y < height; Y++) \
        for (int X = 0; X < width; X++)

#endif
