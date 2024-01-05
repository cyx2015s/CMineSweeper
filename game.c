#ifndef GAME_C_
#define GAME_C_

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "macro.h"
int** map;

/*��ÿһ��Ԫ��ʹ��λ�洢��Ϣ
��һλ���Ƿ����ף�1�У�0��
�ڶ�������λ��0δ������1������2���죬3�����ַ���������
����������λ��Ԥ�������Χ������
*/
int width, height, mines;
// ��ͼ���ȸ߶�
int *temp_x, *temp_y;
int tot = 0;
// ����������
int first_click;
int win, lose;
int mask_id = 0;
int dx[50][50];//dx[0] = {-1,-1,0,1,1,1,0,-1};
int dy[50][50];//dy[0] = {0,1,1,1,0,-1,-1,-1};
int dcard[50];
int totmask;
extern int use_hanzi;
extern void add_eq(int, int);
extern void ai_decide(int, int);
int get_attr(int, int);
extern void add_total_restrict_eq();
extern void setForeColor(int, int, int);
extern void setBackColor(int, int, int);
extern void resetTextFormat();
/*
ʹ��x��ʾ������ʹ��o��ʾ�������ͳ�Ƹ���ʱ���ھӣ��ո��ʾ������
���磺"ooo\\noxo\\nooo" ��ʾͨ���İ���ͨ��Ϸģʽ��
*/
void fast_add_mask(char* str) {
    int x = 0, y = 0, cx = -1, cy = -1;
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == '\n') {
            x = 0;
            y++;
            continue;
        }
        if (str[i] == 'x') {
            cx = x, cy = y;
            break;
        }
        x++;
    }
    if (cx != -1 && cy != -1) {
        dcard[totmask] = 0;
        x = 0, y = 0;
        for (int i = 0; i < len; i++) {
            if (str[i] == '\n') {
                x = 0;
                y++;
                continue;
            }
            if (str[i] == 'o') {
                dx[totmask][dcard[totmask]] = x - cx;
                dy[totmask][dcard[totmask]] = y - cy;
                dcard[totmask]++;
            }
            x++;
        }
        totmask++;
    }
}

// ����ͱ�Ż�ת����
int pos2ind(int x, int y) {
    return y * width + x;
}
// ����ͱ�Ż�ת����
void ind2pos(int ind, int* x, int* y) {
    *x = ind % width;
    *y = ind / width;
}
// �ж��Ƿ�Խ��
int inbound(int x, int y) {
    return x >= 0 && x < width && y >= 0 && y < height;
}
// �����ڴ�
void malloc_map(int w, int h, int m) {
    width = w;
    height = h;
    mines = m;
    win = lose = 0;
    map = (int**)malloc(sizeof(int*) * w);
    for (int i = 0; i < w; i++) {
        map[i] = (int*)malloc(sizeof(int) * h);
        for (int j = 0; j < h; j++) {
            map[i][j] = 0;
        }
    }

    first_click = FIRST_CLICK;
}
// �ͷ��ڴ�
void free_map() {
    for (int i = 0; i < width; i++) {
        free(map[i]);
    }
    free(map);
    width = 0;
    height = 0;
    mines = 0;
}

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
// ���ɵ�ͼ�������һ�ε���ĸ��ӵ�����
void gen_map(int x, int y) {
    temp_x = (int*)malloc(sizeof(int) * (width * height));
    temp_y = (int*)malloc(sizeof(int) * (width * height));
    tot = 0;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (i == x && j == y) {
                continue;
            }
            int ok = 1;
            FOR_NBR(_dx, _dy) {
                if (i == _dx + x && j == _dy + y) {
                    ok = 0;
                    break;
                }
            }
            if (ok) {
                temp_x[tot] = i;
                temp_y[tot] = j;
                tot++;
            }
        }
    }
    mines = min(tot, mines);
    int rand_i;
    for (int i = 0; i < mines; i++) {
        rand_i = rand() % tot;
        swap(&temp_x[i], &temp_x[rand_i]);
        swap(&temp_y[i], &temp_y[rand_i]);
        // ϴ���㷨
    }
    for (int i = 0; i < mines; i++) {
        map[temp_x[i]][temp_y[i]] |= 1;
        FOR_NBR(_dx, _dy) {
            int new_x = temp_x[i] - _dx;
            int new_y = temp_y[i] - _dy;
            if (inbound(new_x, new_y)) {
                map[new_x][new_y] += 8;
                // ��ʾ�����ִӵ�3λ��ʼ�洢���պ�+8
            }
        }
    }
    add_total_restrict_eq();
    free(temp_x);
    free(temp_y);
}
// �û�����ĳ������ 
void open(int x, int y) {
    if (first_click) {
        // �ڵ�һ�ε��ʱ�������׷ֲ�����֤��һ�����Է����϶����
        first_click = !first_click;
        gen_map(x, y);
    }
    if (!(get_attr(map[x][y], GET_OPEN))) {
        map[x][y] |= 2;
        map[x][y] &= ~4;
        if (!get_attr(map[x][y], GET_MINE) &&
            get_attr(map[x][y], GET_COUNT) == 0) {
            FOR_NBR(_dx, _dy) {
                if (inbound(x + _dx, y + _dy)) {
                    open(x + _dx, y + _dy);
                }
            }
        }

        if (!get_attr(map[x][y], GET_MINE)) {
            add_eq(x, y);  // debuuging
        } else {
            ai_decide(pos2ind(x, y), 1);
            lose++;
        }
    }
}
// �ӱ���õ������л�ȡĳ������
int get_attr(int val, int mode) {
    switch (mode) {
        case GET_MINE:
            return val & 1;
            break;
        case GET_OPEN:
            return (val >> 1) & 1;
            break;
        case GET_FLAG:
            return (val >> 2) & 1;
            break;
        case GET_COUNT:
            return val >> 3;
            break;
        default:
            return val;
            break;
    }
}
// ͼ�λ���ӡ
void print() {
    printf(CSI "1m");
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (get_attr(map[x][y], GET_OPEN)) {
                if (get_attr(map[x][y], GET_MINE)) {
                    printf(CSI "47m");
                    setForeColor(192, 0, 0);
                    printf(TEXT_MINE);
                } else {
                    printf(CSI "47m");
                    int tmp = get_attr(map[x][y], GET_COUNT);
                    setForeColor(color_num[tmp % 8][0], color_num[tmp % 8][1],
                                 color_num[tmp % 8][2]);
                    if (use_hanzi) {
                        printf("%2s", text_num[tmp]);
                    } else {
                        if (tmp == 0) {
                            printf("  ");
                        } else {
                            printf("%2d", tmp);
                        }
                    }
                }
            } else {
                int tmp = (x + y) % 2;
                setBackColor(0 + tmp * 16, 32 + tmp * 32, 128 + 32 * tmp);
                if (get_attr(map[x][y], GET_FLAG) == 1) {
                    setForeColor(192, 0, 0);
                    printf(TEXT_FLAG);
                } else {
                    printf("  ");
                }
            }
        }
        printf("\n");
    }
    resetTextFormat();
}
// ��ӡ��������
void debug_print() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("(%d,%d,%d,%d)", get_attr(map[x][y], GET_COUNT),
                   get_attr(map[x][y], GET_OPEN), get_attr(map[x][y], GET_FLAG),
                   get_attr(map[x][y], GET_MINE));
        }
        printf("\n");
    }
}
// ��ǰ����ʱ���
unsigned long long getCurrentTime() {
    SYSTEMTIME st;
    GetSystemTime(&st);

    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);

    unsigned long long nanoseconds =
        ((unsigned long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    unsigned long long milliseconds = nanoseconds / 10000ULL;
    return milliseconds;
}
// Ӯ����
int check_win() {
    if (win) {
        return win;
    }
    int open_count = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (get_attr(map[x][y], GET_OPEN) == 1 &&
                !get_attr(map[x][y], GET_MINE)) {
                open_count++;
            }
        }
    }
    if (open_count == width * height - mines) {
        win = 1;
    }
    return win;
}
#endif