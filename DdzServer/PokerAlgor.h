//
// PokerAlgor.h
//
// ������ϴ���㷨
//
#pragma once

//
// ϴ�ƺ�������
//
// ������
//      poker   - [in, out] �˿������� 0��53
//      num     - [in] ����poker�ĳ��ȣ�����Ϊ54
//  
typedef void (*LPPOKERALGORITHM)(int poker[], int num);

typedef struct tagPokerAlogrithm {
    LPPOKERALGORITHM    algorithm;
    LPTSTR              name;
} POKER_ALOGRITHM, *LPPOKER_ALGORITHM;


void PA_Randomize1(int poker[], int num);
void PA_Randomize10(int poker[], int num);
void PA_RandRemainder(int poker[], int num);
void PA_Test1(int poker[], int num);
void PA_Test2(int poker[], int num);
void PA_Test3(int poker[], int num);


static void get_remainder(int poker[], int num, int times);
static void get_random(int poker[], int num, int times);

