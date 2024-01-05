#ifndef MOUSE_C_
#define MOUSE_C_

#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include "macro.h"
POINT mouse_pos;
CONSOLE_SCREEN_BUFFER_INFO csbi;
CONSOLE_FONT_INFO cfi;
DWORD prevMode;
HANDLE hOutput;
HANDLE hInput;

int last_mouse_state, cur_mouse_state;
int mouse_x, mouse_y, mouse_action;
int middle_clicked; // 一堆存储鼠标状态的变量
int force_flush;
char buffa[1 << 15], buffb[1 << 15]; // 双缓冲，防屏闪
void switchBuffer();
int checkMouseAction();

void setCursorPosition(int x, int y) {
    printf(CSI "%dG" CSI "%dd", x + 1, y + 1);
};

void setForeColor(int R, int G, int B){
    printf(CSI "38;2;%d;%d;%dm", R, G, B);
}

void setBackColor(int R, int G, int B){
    printf(CSI "48;2;%d;%d;%dm", R, G, B);
}
void resetTextFormat(){
    printf(CSI "0m");
}
void clearConsoleScreen() {
    COORD coordScreen = {0, 0};
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    GetConsoleScreenBufferInfo(hOutput, &csbi);
    dwConSize = csbi.dwMaximumWindowSize.X * csbi.dwMaximumWindowSize.Y;
    setCursorPosition(0, 0);
    printf(CSI "0m");
    for (int i = 0; i < dwConSize; i++) {
        putchar(' ');
    }
    setCursorPosition(0, 0);
}

void editConsoleMode() {
    GetConsoleMode(hInput, &prevMode);
    SetConsoleMode(hInput,
                   (prevMode & ~ENABLE_QUICK_EDIT_MODE) |
                       ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_MOUSE_INPUT);
}
void setScreenSize(int x, int y) {
    COORD size = {x, y};
    SMALL_RECT rect = {0, 0, size.X - 1, size.Y - 1};
    SetConsoleWindowInfo(hOutput, 1, &rect);
    SetConsoleScreenBufferSize(hOutput, size);
}

void initHandlers() {
    switchBuffer();
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    hInput = GetStdHandle(STD_INPUT_HANDLE);
}

void updateMouseKeyState(int new_state) {
    if (cur_mouse_state == 0) {
        middle_clicked = 0;
    }
    mouse_action = -1;
    last_mouse_state = cur_mouse_state;
    cur_mouse_state = new_state;
    cur_mouse_state &= 7;
}
/*基于事件的鼠标侦测*/
#if GET_MOUSE_BY_EVENT
void awaitMouseEvent() {
    DWORD numEvents;
    INPUT_RECORD inp;
    // printf("Still Waiting\n");
    GetNumberOfConsoleInputEvents(hInput, &numEvents);
    if (numEvents == 0) {
        Sleep(20);
        updateMouseKeyState(cur_mouse_state);
        checkMouseAction();
    } else {
        ReadConsoleInput(hInput, &inp, 1, &numEvents);
        if (inp.EventType == MOUSE_EVENT) {
            // printf("Mouse event get");
            // fflush(stdout);
            mouse_x = inp.Event.MouseEvent.dwMousePosition.X;
            mouse_y = inp.Event.MouseEvent.dwMousePosition.Y;
            // if (cur_mouse_state == 0) {
            //     last_mouse_state = 0;
            // } else {
            //     last_mouse_state |= cur_mouse_state;
            // }
            updateMouseKeyState(inp.Event.MouseEvent.dwButtonState);
            checkMouseAction();
        } else {
            awaitMouseEvent();
        }
    }
}
#else
// 废弃代码
void awaitMouseEvent() {
    Sleep(10);
    int tmp_state = 0;
    if (GetAsyncKeyState(VK_LBUTTON)) {
        tmp_state |= 1;
    }
    if (GetAsyncKeyState(VK_RBUTTON)) {
        tmp_state |= 2;
    }
    if (GetAsyncKeyState(VK_MBUTTON)) {
        tmp_state |= 4;
    }
    GetConsoleScreenBufferInfo(hOutput, &csbi);
    GetCurrentConsoleFont(hOutput, 0, &cfi);
    GetCursorPos(&mouse_pos);
    // mouse_x = csbi.dwCursorPosition.X;
    // mouse_x = cfi.nFont;
    mouse_x = GetConsoleFontSize(hOutput, cfi.nFont).X;
    mouse_y = csbi.dwCursorPosition.Y;
    // 这里获取到的坐标不是以字符为坐标的，麻烦
    updateMouseKeyState(tmp_state);
    checkMouseAction();
}
#endif
void switchBuffer() {
    static int cur = 0;
    switch (cur) {
        case 0:
            // fflush(stdout);
            setvbuf(stdout, buffa, _IOFBF, 1 << 15);
            cur = 1;
            break;
        case 1:
            // fflush(stdout);
            setvbuf(stdout, buffb, _IOFBF, 1 << 15);
            cur = 0;
            break;
        default:
            break;
    }
}
// 判断鼠标的动作
int checkMouseAction() {
    if (mouse_action < 0) {
        if (last_mouse_state != cur_mouse_state) {
            if (!middle_clicked) {
                if (last_mouse_state == 1 && cur_mouse_state == 0) {
                    return mouse_action = MOUSE_LEFTUP;
                }
                if (last_mouse_state == 0 && cur_mouse_state == 2) {
                    return mouse_action = MOUSE_RIGHTDOWN;
                }
            }
            if (last_mouse_state & 4 && !(cur_mouse_state & 4)) {
                middle_clicked = 1;
                return mouse_action = MOUSE_MIDDLEUP;
            }
            if (last_mouse_state == 3 && cur_mouse_state < 3) {
                middle_clicked = 1;
                return mouse_action = MOUSE_MIDDLEUP;
            }
        }
        return mouse_action = MOUSE_IDLE;
    } else {
        return mouse_action;
    }
}

void SystemCLS() {
    system("cls");
    // 这样清屏后要重新获取handler和设置控制台模式，所以不可以频繁调用
    initHandlers();
    editConsoleMode();
}

// 事件处理流程
// 右键按下瞬间：如果左键没有按下，切换旗子
// 左键按下后：左键抬起时若右键没有按下，翻开当前格子，否则翻开周围格子
// 右键抬起瞬间：如果左键按下，翻开周围格子
// 中键抬起瞬间：翻开周围格子

#endif