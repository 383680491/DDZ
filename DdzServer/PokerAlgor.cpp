//
// PokerAlgor.cpp
//
//  ��������Ϸ�ļ���ϴ���㷨
//

#include "stdafx.h"
#include "PokerAlgor.h"

#define POKER_ITEM_COUNT            54

// ��λ�ķ���ϴ��
static void get_remainder(int poker[], int num, int times)
{
    int index;
    int remainder;
    int old_poker[POKER_ITEM_COUNT];

    if (num != POKER_ITEM_COUNT) {
        return;
    }

#if 0
    // ���ó�ʼ�˿�������Ϊ 0��53
    for (int i = 0; i < num; i++) {
        poker[i] = i;
    }
#else
    get_random(poker, num, 1);
#endif

    srand((unsigned)time(NULL));

    while (times--) { // �ظ� times �δ�λϴ��
        
        if (rand() % 2 == 0) {
            for (int i = 0; i < num; i++) { // ��ͷ��β���汣���ϴ�õ���
                old_poker[i] = poker[i];
            }
        } else {
            for (int i = 0; i < num; i++) { // ��β��ͷ���汣���ϴ�õ���
                old_poker[i] = poker[num - i - 1];
            }
        }

        remainder = 3 + rand() % 6; // ȡһ�� 3��8 ֮�������Ϊ��λֵ

        index = 0;

        for (int i = 0; i < remainder; i++) { // ��λϴ��
            for (int j = 0; j < num; j++) {
                if (j % remainder == i) {
                    poker[index++] = old_poker[j];
                }
            }
        }
    }
}

// ����ķ���ϴ��
static void get_random(int poker[], int num, int times)
{
    int random;
    int temp;

    // ���ó�ʼ�˿�������Ϊ 0��53
    for (int i = 0; i < num; i++) {
        poker[i] = i;
    }

    srand((unsigned)time(NULL));

    while (times--) { // �������˳��Ĵ���
        for (int i = 0; i < num; i++) { // �������˳��
            if (i == num - 1) {
                random = 0;
            } else {
                random = rand() % (num - i);
            }

            // ��ÿ������õ������滻��ĩβ
            temp = poker[num - i - 1];
            poker[num - i - 1] = poker[random];
            poker[random] = temp;
        }
    }
}

// ���ϴ��1��
void PA_Randomize1(int poker[], int num)
{
    int rd;
    int unused_cnt;
    int used[POKER_ITEM_COUNT] = { 0 };

    if (num != POKER_ITEM_COUNT) {
        return;
    }

    srand((unsigned)time(NULL));

    for (int i = 0; i < POKER_ITEM_COUNT; i++) {
        if (i == POKER_ITEM_COUNT - 1) {
            rd = 0;
        } else {
            rd = rand() % (POKER_ITEM_COUNT - i);
        }

        unused_cnt = 0;
        for (int index = 0; index < POKER_ITEM_COUNT; index++) {
            if (used[index] != 0) {
                continue;
            } else {
                if (unused_cnt == rd) {
                    used[index] = 1;
                    poker[i] = index;
                    break;
                }

                unused_cnt++;
            }
        }
    }
}

// ����ķ���ϴ��
void PA_Randomize10(int poker[], int num)
{
    srand((unsigned)time(NULL));
    int times = 6 + rand() % 5;

    // ���ϴ��6��10��
    get_random(poker, num, times);
}

void PA_RandRemainder(int poker[], int num)
{
    srand((unsigned)time(NULL));
    int times = 6 + rand() % 5;

    // �����λϴ�ƣ�ϴ��6��10��
    get_remainder(poker, num, times);
}

// 
void PA_Test1(int poker[], int num)
{
    // ��ʼ���˿�������Ϊ 0��53
    for (int i = 0; i < num; i++) {
        poker[i] = i;
    }
}

void PA_Test2(int poker[], int num)
{
    // ��ʼ���˿�������Ϊ 0��53
    for (int i = 0; i < num; i++) {
        poker[i] = i;
    }
}

void PA_Test3(int poker[], int num)
{
    // ��ʼ���˿�������Ϊ 0��53
    for (int i = 0; i < num; i++) {
        poker[i] = i;
    }
}
