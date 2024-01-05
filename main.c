#include <conio.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "game.c"
#include "macro.h"
#include "mouse.c"
#include "solve.c"

int t_start, t_end;
int page = PAGE_TITLE;
unsigned long long start_time, end_time, delta_time;
UINT prev_CP;
int user_w = 15, user_h = 12, user_m = 50, user_assist = 0, use_hanzi = 1,
    nbr_hint = 1;
int _x, _y;
int mlups, mrdns, mmups, t_1, t_2, frames;
// 统计鼠标事件，时间，帧数
int init_all() {
    // 获取代码页，设置为GBK2312
    prev_CP = GetConsoleOutputCP();
    SetConsoleOutputCP(936);
    initHandlers();
    // 打开转义字符功能
    editConsoleMode();
    // 隐藏光标
    printf(CSI "?25l");  
    // 初始化一堆玩法
    fast_add_mask("ooo\noxo\nooo");
    fast_add_mask("oo\noxo\n oo");
    fast_add_mask("  o\n ooo\nooxoo\n ooo\n  o");
    fast_add_mask(" o o\no   o\n  x\no   o\n o o");
    fast_add_mask("ooooo\no   o\no x o\no   o\nooooo");
    fast_add_mask("ooooo\nooooo\nooxoo\nooooo\nooooo");
    fast_add_mask(" o o\nooooo\n oxo\nooooo\n o o");
    fast_add_mask(" o\noxo\n o");
    fast_add_mask("oooo\noooo\noxoo\noooo");
    fast_add_mask(" ooo\n ooo\n xoo\n");
}

int init_game(int w, int h, int m) {
    free_eq();
    // 推理模块释放时需要惨开malloc_map设置的数据，不可交换
    free_map();
    malloc_map(w, h, m);
    // 同理分配也有顺序
    malloc_eq(w, h);
}
// 处理鼠标中键的事件（比较复杂专门写成函数）
void middle_action(int _x, int _y) {
    if (user_assist) {
        if (get_attr(map[_x][_y], GET_OPEN)) {
            t_1 = getCurrentTime();
            FOR_NBR(_dx, _dy) {
                if (inbound(_x + _dx, _y + _dy)) {
                    switch (ai_res[_x + _dx][_y + _dy]) {
                        case 0:
                            open(_x + _dx, _y + _dy);
                            break;
                        case 1:
                            map[_x + _dx][_y + _dy] |= 4;
                            break;
                        default:
                            break;
                    }
                }
            }
            t_2 = getCurrentTime();
        } else {
            switch (ai_res[_x][_y]) {
                case 0:
                    open(_x, _y);
                    break;
                case 1:
                    map[_x][_y] |= 4;
                    break;
                default:
                    break;
            }
        }
        if (get_attr(map[_x][_y], GET_OPEN)) {
            int cnt_flag = 0, cnt_cover = 0;
            FOR_NBR(_dx, _dy) {
                if (inbound(_x + _dx, _y + _dy)) {
                    cnt_cover++;
                    if (!get_attr(map[_x + _dx][_y + _dy], GET_OPEN)) {
                        if (ai_res[_x + _dx][_y + _dy]) {
                            cnt_flag++;
                        }
                    } else {
                        cnt_cover--;
                        if (get_attr(map[_x + _dx][_y + _dy], GET_MINE)) {
                            cnt_flag++;
                        }
                    }
                }
            }
            if (cnt_flag == get_attr(map[_x][_y], GET_COUNT)) {
                FOR_NBR(_dx, _dy) {
                    if (inbound(_x + _dx, _y + _dy) &&
                        !ai_res[_x + _dx][_y + _dy] &&
                        !get_attr(map[_x + _dx][_y + _dy], GET_OPEN)) {
                        open(_x + _dx, _y + _dy);
                    }
                }
            } else if (cnt_cover == get_attr(map[_x][_y], GET_COUNT)) {
                FOR_NBR(_dx, _dy) {
                    if (inbound(_x + _dx, _y + _dy) &&
                        !get_attr(map[_x + _dx][_y + _dy], GET_OPEN)) {
                        map[_x + _dx][_y + _dy] |= 4;
                    }
                }
            }
        }
    } else {
        if (get_attr(map[_x][_y], GET_OPEN)) {
            int cnt_flag = 0, cnt_cover = 0;
            FOR_NBR(_dx, _dy) {
                if (inbound(_x + _dx, _y + _dy)) {
                    cnt_cover++;
                    if (!get_attr(map[_x + _dx][_y + _dy], GET_OPEN)) {
                        if (get_attr(map[_x + _dx][_y + _dy], GET_FLAG)) {
                            cnt_flag++;
                        }
                    } else {
                        cnt_cover--;
                        if (get_attr(map[_x + _dx][_y + _dy], GET_MINE)) {
                            cnt_flag++;
                        }
                    }
                }
            }
            if (cnt_flag == get_attr(map[_x][_y], GET_COUNT)) {
                FOR_NBR(_dx, _dy) {
                    if (inbound(_x + _dx, _y + _dy) &&
                        !get_attr(map[_x + _dx][_y + _dy], GET_FLAG) &&
                        !get_attr(map[_x + _dx][_y + _dy], GET_OPEN)) {
                        open(_x + _dx, _y + _dy);
                    }
                }
            } else if (cnt_cover == get_attr(map[_x][_y], GET_COUNT)) {
                FOR_NBR(_dx, _dy) {
                    if (inbound(_x + _dx, _y + _dy) &&
                        !get_attr(map[_x + _dx][_y + _dy], GET_OPEN)) {
                        map[_x + _dx][_y + _dy] |= 4;
                    }
                }
            }
        }
    }
}
// 邻居高光时刻！
void highlight_nbr(int _cx, int _cy) {
    printf(CSI "1m");
    FOR_NBR(_dx, _dy) {
        int _x = _cx + _dx, _y = _cy + _dy;
        if (!inbound(_x, _y)) {
            continue;
        }
        setCursorPosition(_x * 2, _y);
        if (get_attr(map[_x][_y], GET_OPEN)) {
            if (get_attr(map[_x][_y], GET_MINE)) {
                printf(CSI "46m");
                setForeColor(192, 0, 0);
                printf(TEXT_MINE);
            } else {
                printf(CSI "46m");
                int tmp = get_attr(map[_x][_y], GET_COUNT);
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
            int tmp = (_x + _y) % 2;
            setBackColor(224 + tmp * 16, 192 + tmp * 16, 32 + 32 * tmp);
            if (get_attr(map[_x][_y], GET_FLAG) == 1) {
                setForeColor(192, 0, 0);
                printf(TEXT_FLAG);
            } else {
                printf("  ");
            }
        }
    }
    resetTextFormat();
}
int main() {
    init_all();
    start_time = getCurrentTime();
    srand(start_time);
    printf("加载中……");
    fflush(stdout);
    Sleep(100);
    force_flush = 1;
    while (page != PAGE_EXIT) {
        if (force_flush > 0) {
            force_flush--;
            updateMouseKeyState(cur_mouse_state);
        } else {
            awaitMouseEvent();
        }
        if (page != PAGE_GAME) {
            clearConsoleScreen();
        } else {
            printf(CSI "0G" CSI "0d");
        }
        frames++;
        switch (mouse_action) {
            case MOUSE_LEFTUP:
                mlups++;
                break;
            case MOUSE_RIGHTDOWN:
                mrdns++;
                break;
            case MOUSE_MIDDLEUP:
                mmups++;
                break;
            default:
                break;
        }
        switch (page) {
            case PAGE_TITLE:
                setCursorPosition(6, 3);
                printf("控  制  台  扫  雷");
                setCursorPosition(16, 5);
                printf("切向量2-2-3 @ 哔哩哔哩");
                setCursorPosition(16, 6);
                printf("东方震E4    @ 哔哩哔哩");
                BUTTON("随机游戏", 20, 8, {
                    page = PAGE_GAME_ENTER;
                    user_h = rand() % 10 + 10;
                    user_w = rand() % 10 + 10;
                    user_m = rand() % 5 + user_h * user_w / 6;
                    mask_id = rand() % totmask;
                });
                BUTTON("开始游戏", 10, 8, { page = PAGE_SETTING; });

                BUTTON("使用说明", 10, 10, { page = PAGE_ABOUT; });
                BUTTON("退    出", 20, 10, { page = PAGE_EXIT; });
                break;
            case PAGE_SETTING:
                setCursorPosition(10, 2);
                printf("宽度: %3d", user_w);
                if (user_w < 30) {
                    BUTTON("增加", 20, 2, { user_w++; });
                }
                if (user_w > 8) {
                    BUTTON("减少", 4, 2, { user_w--; });
                }
                setCursorPosition(10, 3);
                printf("高度: %3d", user_h);
                if (user_h < 24) {
                    BUTTON("增加", 20, 3, { user_h++; });
                }
                if (user_h > 8) {
                    BUTTON("减少", 4, 3, { user_h--; });
                }
                setCursorPosition(10, 5);
                printf("雷数: %3d", user_m);
                if (user_m < (user_h - 1) * (user_w - 1)) {
                    BUTTON("加一", 20, 5, { user_m++; });
                    BUTTON("加十", 20, 6, { user_m += 10; });
                }
                if (user_m > 10) {
                    BUTTON("减一", 4, 5, { user_m--; });
                    BUTTON("减十", 4, 6, { user_m -= 10; });
                }
                if (user_m < 10) {
                    user_m = 10;
                }
                if (user_m > (user_h - 1) * (user_w - 1)) {
                    user_m = (user_h - 1) * (user_w - 1);
                }
                setCursorPosition(26, 2);
                printf("邻居规则: ", mask_id + 1);
                setCursorPosition(26, 3);
                printf("%2d / %2d", mask_id + 1, totmask);
                for (int i = 0; i < dcard[mask_id]; i++) {
                    setCursorPosition(44 + dx[mask_id][i] * 2,
                                      4 + dy[mask_id][i]);
                    printf(TEXT_COVER);
                }
                setCursorPosition(44, 4);
                printf(TEXT_CENTER);
                BUTTON("切换", 30, 4, { mask_id = (mask_id + 1) % totmask; });
                setCursorPosition(2, 8);
                printf("高级选项");
                setCursorPosition(10, 12);
                printf("中键辅助模式: ");
                switch (user_assist) {
                    case 0:
                        printf("默认");
                        break;
                    case 1:
                        printf("高级");
                        setCursorPosition(10, 13);
                        printf("程序会尽可能地进行推理, 并无视玩家的标记。");
                        break;
                    default:
                        break;
                }

                BUTTON("切换", 4, 12, { user_assist ^= 1; });
                setCursorPosition(10, 10);
                printf("使用汉字数字: %s", yesno[use_hanzi]);
                BUTTON("切换", 4, 10, { use_hanzi ^= 1; });

                setCursorPosition(10, 11);
                printf("高亮邻居格子: %s", yesno[nbr_hint]);
                BUTTON("切换", 4, 11, { nbr_hint ^= 1; });

                BUTTON("开    始", 28, 8, { page = PAGE_GAME_ENTER; });
                BUTTON("返回标题", 0, 0, { page = PAGE_TITLE; });
                break;
            case PAGE_GAME_ENTER:
                init_game(user_w, user_h, user_m);
                page = PAGE_GAME;
                force_flush = 1;
                break;
            case PAGE_GAME:
                _x = mouse_x / 2, _y = mouse_y;
                if (inbound(_x, _y) && !win) {
                    switch (mouse_action) {
                        case MOUSE_LEFTUP:
                            if (!get_attr(map[_x][_y], GET_FLAG)) {
                                open(_x, _y);
                            }
                            break;
                        case MOUSE_RIGHTDOWN:
                            if (!get_attr(map[_x][_y], GET_OPEN)) {
                                map[_x][_y] ^= 4;
                            }
                            break;
                        case MOUSE_MIDDLEUP:
                            middle_action(_x, _y);
                        default:
                            break;
                    }

                    elim_eq();  
                }
                setCursorPosition(0, 0);
                check_win();
                print();
                if (nbr_hint && inbound(_x, _y)) {
                    highlight_nbr(_x, _y);
                }
                BUTTON("返回标题", 0, 1 * height, {
                    SystemCLS();
                    page = PAGE_TITLE;
                });
                printf("\n宽度=%3d, 高度=%3d, 总雷数=%3d\n", width, height,
                       mines);
                if (!first_click && !win) {
                    end_time = getCurrentTime();
                    delta_time = end_time - start_time;
                    printf("总时间: %02d:%02d:%02d.%03d",
                           delta_time / 3600 / 1000,
                           delta_time / 60 / 1000 % 60, delta_time / 1000 % 60,
                           delta_time % 1000);
                } else if (first_click) {
                    start_time = getCurrentTime();
                }
                if (win && !lose) {
                    printf("\n你赢了! \n");
                } else if (win) {
                    printf("\n完成了! 失误次数: %d\n", lose);
                }
                break;
            case PAGE_ABOUT:
                setCursorPosition(10, 0);
                printf("使用说明");
                setCursorPosition(0, 2);
                printf(
                    "左键: 翻开地格\n右键: "
                    "插上/删除旗子\n\n中键或左右键同时点击:\n  1. "
                    "如果周围旗子数量等于显示的数字, \n     翻开剩下的地格。\n"
                    "  2. 如果周围未翻开格子数量等于显示的数字, "
                    "\n     "
                    "在未翻开的地格上插上旗子。\n\n- "
                    "游戏的配色取决于控制台设置。"
                    "\n- 由于不同版本控制台不同, "
                    "\n  请开始游戏后调节窗口使得画面能够完整出现在控制台内。\n"
                    "- 点到雷不会导致游戏结束, "
                    "可以继续游玩测试。\n- 反色文字均为可以点击的按钮。\n- "
                    "如果发现控制台有残余图像, "
                    "点击「强制重置画面」。\n  点击时确保按钮处于画面第一行。");
                BUTTON("返回标题", 0, 0, {
                    SystemCLS();
                    page = PAGE_TITLE;
                });
                BUTTON("强制重置画面", 20, 0, { SystemCLS(); });
            default:
                break;
        }

        fflush(stdout);
    }
    SetConsoleMode(hInput, prevMode);
    SetConsoleCP(prev_CP);
    resetTextFormat();
    init_game(0, 0, 0);
    return 0;
}
