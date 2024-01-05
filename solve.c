#ifndef SOLVE_C_
#define SOLVE_C_
#include <stdio.h>
#include "macro.h"

/* ������֮ǰ��Զ�� Python
 ���򣬷��ֵ�ʱΪ�����Ԫ������������Ƿ�Ψһ���һ�д��һ����̬�滮����������д�˴����Դ��룬���ֲ���Ҫ
 dp�����������Ľ�պ�����Щ�����໥������γɵ����ֵ����Сֵ��Ҳ���ǣ������̳�����Ϊ����Ԫ����������ϵ����ֵ/��Сֵ����ô����Ψһ�⡣
*/

extern int** map;
extern int width;
extern int height;
extern int mines;

extern int pos2ind(int, int);
extern void ind2pos(int, int*, int*);
extern int dx[][50];
extern int dy[][50];
extern int dcard[];
extern int mask_id;
extern int inbound(int, int);
extern int get_attr(int, int);

int** eq;
int* pivot;
int maxeq = -1;
int toteq;
int** ai_res;  // �Ƿ�Ӧ�ñ�ע����
// �����ڴ�
void malloc_eq() {
    maxeq = width * height + 1;

    eq = (int**)(malloc(sizeof(int*) * (maxeq)));
    pivot = (int*)(malloc(sizeof(int) * (maxeq)));
    for (int i = 0; i < maxeq; i++) {
        eq[i] = (int*)(malloc(sizeof(int) * (maxeq)));
        for (int j = 0; j < maxeq; j++) {
            eq[i][j] = 0;
        }
        pivot[i] = 0;
    }
    ai_res = (int**)(malloc(sizeof(int*) * width));

    for (int i = 0; i < width; i++) {
        ai_res[i] = (int*)(malloc(sizeof(int) * height));
        for (int j = 0; j < height; j++) {
            ai_res[i][j] = -1;
        }
    }
    toteq = 0;
    maxeq--;
}
// �ͷ��ڴ�
void free_eq() {
    for (int i = 0; i <= maxeq; i++) {
        free(eq[i]);
    }
    free(eq);
    free(pivot);
    for (int i = 0; i < width; i++) {
        free(ai_res[i]);
    }
    free(ai_res);
    maxeq = -1;
    toteq = 0;
}
// �����ã���ӡ������
void print_eq() {
    int tx, ty;
    printf("\nCurrent: %d Equation(s)\n", toteq);
    for (int i = 0; i < toteq; i++) {
        for (int j = 0; j < maxeq; j++) {
            if (eq[i][j] != 0) {
                ind2pos(j, &tx, &ty);
                printf("+%d(%d,%d)", eq[i][j], tx, ty);
            }
        }
        printf("=%d", eq[i][maxeq]);
        ind2pos(pivot[i], &tx, &ty);
        printf("{pivot=(%d,%d)}\n", tx, ty);
    }
}
/* ���� j �е� mul ���ӵ��� i ���ϡ�
  op �� operation ����д��
  ���г����б任��
 */
void row_op(int i, int j, int mul) {
    for (int k = 0; k <= maxeq; k++) {
        eq[i][k] += eq[j][k] * mul;
    }
}
// �Գ�һ������
void row_imul(int i, int mul) {
    for (int k = 0; k <= maxeq; k++) {
        eq[i][k] *= mul;
    }
}
// ȷ����Ԫ��
void find_pivot(int ind) {
    if (eq[ind][pivot[ind]] == 0 || pivot[ind] == maxeq) {
        pivot[ind] = maxeq;
        for (int i = 0; i < maxeq; i++) {
            if (eq[ind][i] != 0) {
                pivot[ind] = i;
                return;
            }
        }
    }
}
/*
�� pivot[j] Ϊ�ο��������õ� j ������ɾ���� i �������еĶ�Ӧ����
*/
void cancel_row(int i, int j) {
    if (eq[j][pivot[j]] == 0 || eq[i][pivot[j]] == 0) {
        return;
    }
    row_imul(i, eq[j][pivot[j]]);
    row_op(i, j, -eq[i][pivot[j]] / eq[j][pivot[j]]);
}

void ai_decide(int var, int val) {
    int x, y;
    ind2pos(var, &x, &y);
    ai_res[x][y] = val;
    for (int i = 0; i < toteq; i++) {
        eq[i][maxeq] -= val * eq[i][var];
        eq[i][var] = 0;
    }
}
// ���и��Ӽ�������������
void add_total_restrict_eq() {
    for (int i = 0; i < maxeq; i++) {
        eq[toteq][i] = 1;
    }
    eq[toteq][maxeq] = mines;
    toteq++;
}

// ����ĳ������ȷ���������ھӵ�һ�����̲�����
void add_eq(int x, int y) {
    ai_decide(pos2ind(x, y), 0);
    if (get_attr(map[x][y], GET_COUNT) == 0) {
        return;
    }
    eq[toteq][maxeq] = 0;
    row_imul(toteq, 0);
    FOR_NBR(_dx, _dy) {
        int new_x = x + _dx;
        int new_y = y + _dy;

        if (inbound(new_x, new_y)) {
            if (ai_res[new_x][new_y] == -1) {
                eq[toteq][pos2ind(new_x, new_y)] = 1;
            } else {
                eq[toteq][maxeq] -= ai_res[new_x][new_y];
            }
        }
    }
    eq[toteq][maxeq] += get_attr(map[x][y], GET_COUNT);
    toteq++;
}
// �����з�����Ԫ
void elim_eq() {
    for (int i = 0; i < toteq; i++) {
        find_pivot(i);
        if (pivot[i] == maxeq) {
            row_imul(i, 0);
            row_op(i, toteq - 1, 1);
            toteq--;
            i--;
            continue;
        }
        for (int j = 0; j < toteq; j++) {
            if (j != i) {
                cancel_row(j, i);
            }
        }
    }
    int updated = 0;
    int maxself, minself;
    for (int i = 0; i < toteq; i++) {
        maxself = 0;
        minself = 0;
        // ��������һ��ע��
        for (int j = 0; j < maxeq; j++) {
            if (eq[i][j] > 0) {
                maxself += eq[i][j];
            }
            if (eq[i][j] < 0) {
                minself += eq[i][j];
            }
        }
        if (maxself == eq[i][maxeq]) {
            updated = 1;
            for (int j = 0; j < maxeq; j++) {
                if (eq[i][j] > 0) {
                    ai_decide(j, 1);
                } else if (eq[i][j] < 0) {
                    ai_decide(j, 0);
                }
            }
        } else if (minself == eq[i][maxeq]) {
            updated = 1;
            for (int j = 0; j < maxeq; j++) {
                if (eq[i][j] > 0) {
                    ai_decide(j, 0);
                } else if (eq[i][j] < 0) {
                    ai_decide(j, 1);
                }
            }
        }
    }
    if (updated) {
        elim_eq();// ��Ԫ����û�б仯Ϊֹ
    }
    return;
}
// ��ӡ������
void print_ai_res() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            switch (ai_res[x][y]) {
                case -1:
                    printf(" ?");
                    break;
                case 0:
                    printf("__");
                    break;
                case 1:
                    printf(" !");
                    break;
            }
        }
        printf("\n");
    }
}
#endif