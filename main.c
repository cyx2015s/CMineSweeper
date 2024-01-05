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
// ͳ������¼���ʱ�䣬֡��
int init_all() {
    // ��ȡ����ҳ������ΪGBK2312
    prev_CP = GetConsoleOutputCP();
    SetConsoleOutputCP(936);
    initHandlers();
    // ��ת���ַ�����
    editConsoleMode();
    // ���ع��
    printf(CSI "?25l");  
    // ��ʼ��һ���淨
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
    // ����ģ���ͷ�ʱ��Ҫ�ҿ�malloc_map���õ����ݣ����ɽ���
    free_map();
    malloc_map(w, h, m);
    // ͬ�����Ҳ��˳��
    malloc_eq(w, h);
}
// ��������м����¼����Ƚϸ���ר��д�ɺ�����
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
// �ھӸ߹�ʱ�̣�
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
    printf("�����С���");
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
                printf("��  ��  ̨  ɨ  ��");
                setCursorPosition(16, 5);
                printf("������2-2-3 @ ��������");
                setCursorPosition(16, 6);
                printf("������E4    @ ��������");
                BUTTON("�����Ϸ", 20, 8, {
                    page = PAGE_GAME_ENTER;
                    user_h = rand() % 10 + 10;
                    user_w = rand() % 10 + 10;
                    user_m = rand() % 5 + user_h * user_w / 6;
                    mask_id = rand() % totmask;
                });
                BUTTON("��ʼ��Ϸ", 10, 8, { page = PAGE_SETTING; });

                BUTTON("ʹ��˵��", 10, 10, { page = PAGE_ABOUT; });
                BUTTON("��    ��", 20, 10, { page = PAGE_EXIT; });
                break;
            case PAGE_SETTING:
                setCursorPosition(10, 2);
                printf("���: %3d", user_w);
                if (user_w < 30) {
                    BUTTON("����", 20, 2, { user_w++; });
                }
                if (user_w > 8) {
                    BUTTON("����", 4, 2, { user_w--; });
                }
                setCursorPosition(10, 3);
                printf("�߶�: %3d", user_h);
                if (user_h < 24) {
                    BUTTON("����", 20, 3, { user_h++; });
                }
                if (user_h > 8) {
                    BUTTON("����", 4, 3, { user_h--; });
                }
                setCursorPosition(10, 5);
                printf("����: %3d", user_m);
                if (user_m < (user_h - 1) * (user_w - 1)) {
                    BUTTON("��һ", 20, 5, { user_m++; });
                    BUTTON("��ʮ", 20, 6, { user_m += 10; });
                }
                if (user_m > 10) {
                    BUTTON("��һ", 4, 5, { user_m--; });
                    BUTTON("��ʮ", 4, 6, { user_m -= 10; });
                }
                if (user_m < 10) {
                    user_m = 10;
                }
                if (user_m > (user_h - 1) * (user_w - 1)) {
                    user_m = (user_h - 1) * (user_w - 1);
                }
                setCursorPosition(26, 2);
                printf("�ھӹ���: ", mask_id + 1);
                setCursorPosition(26, 3);
                printf("%2d / %2d", mask_id + 1, totmask);
                for (int i = 0; i < dcard[mask_id]; i++) {
                    setCursorPosition(44 + dx[mask_id][i] * 2,
                                      4 + dy[mask_id][i]);
                    printf(TEXT_COVER);
                }
                setCursorPosition(44, 4);
                printf(TEXT_CENTER);
                BUTTON("�л�", 30, 4, { mask_id = (mask_id + 1) % totmask; });
                setCursorPosition(2, 8);
                printf("�߼�ѡ��");
                setCursorPosition(10, 12);
                printf("�м�����ģʽ: ");
                switch (user_assist) {
                    case 0:
                        printf("Ĭ��");
                        break;
                    case 1:
                        printf("�߼�");
                        setCursorPosition(10, 13);
                        printf("����ᾡ���ܵؽ�������, ��������ҵı�ǡ�");
                        break;
                    default:
                        break;
                }

                BUTTON("�л�", 4, 12, { user_assist ^= 1; });
                setCursorPosition(10, 10);
                printf("ʹ�ú�������: %s", yesno[use_hanzi]);
                BUTTON("�л�", 4, 10, { use_hanzi ^= 1; });

                setCursorPosition(10, 11);
                printf("�����ھӸ���: %s", yesno[nbr_hint]);
                BUTTON("�л�", 4, 11, { nbr_hint ^= 1; });

                BUTTON("��    ʼ", 28, 8, { page = PAGE_GAME_ENTER; });
                BUTTON("���ر���", 0, 0, { page = PAGE_TITLE; });
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
                BUTTON("���ر���", 0, 1 * height, {
                    SystemCLS();
                    page = PAGE_TITLE;
                });
                printf("\n���=%3d, �߶�=%3d, ������=%3d\n", width, height,
                       mines);
                if (!first_click && !win) {
                    end_time = getCurrentTime();
                    delta_time = end_time - start_time;
                    printf("��ʱ��: %02d:%02d:%02d.%03d",
                           delta_time / 3600 / 1000,
                           delta_time / 60 / 1000 % 60, delta_time / 1000 % 60,
                           delta_time % 1000);
                } else if (first_click) {
                    start_time = getCurrentTime();
                }
                if (win && !lose) {
                    printf("\n��Ӯ��! \n");
                } else if (win) {
                    printf("\n�����! ʧ�����: %d\n", lose);
                }
                break;
            case PAGE_ABOUT:
                setCursorPosition(10, 0);
                printf("ʹ��˵��");
                setCursorPosition(0, 2);
                printf(
                    "���: �����ظ�\n�Ҽ�: "
                    "����/ɾ������\n\n�м������Ҽ�ͬʱ���:\n  1. "
                    "�����Χ��������������ʾ������, \n     ����ʣ�µĵظ�\n"
                    "  2. �����Χδ������������������ʾ������, "
                    "\n     "
                    "��δ�����ĵظ��ϲ������ӡ�\n\n- "
                    "��Ϸ����ɫȡ���ڿ���̨���á�"
                    "\n- ���ڲ�ͬ�汾����̨��ͬ, "
                    "\n  �뿪ʼ��Ϸ����ڴ���ʹ�û����ܹ����������ڿ���̨�ڡ�\n"
                    "- �㵽�ײ��ᵼ����Ϸ����, "
                    "���Լ���������ԡ�\n- ��ɫ���־�Ϊ���Ե���İ�ť��\n- "
                    "������ֿ���̨�в���ͼ��, "
                    "�����ǿ�����û��桹��\n  ���ʱȷ����ť���ڻ����һ�С�");
                BUTTON("���ر���", 0, 0, {
                    SystemCLS();
                    page = PAGE_TITLE;
                });
                BUTTON("ǿ�����û���", 20, 0, { SystemCLS(); });
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
