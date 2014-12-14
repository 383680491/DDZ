//
//
// Utility.cpp
//
//
#include "stdafx.h"
#include "Utility.h"

//
// Used for saving debug info to print out.
//
#ifdef _DEBUG
TCHAR dbg_TraceTextBuf [DEBUG_TRACE_LEN];
#endif


//
// �ײ��˿��Ƶ�������������Ƶģ�
//
// ֵ��ͬ���˿�������Ϊ����Ƭ ÷�� ���� ����
// �˿ˣ�3 3 3 3 4 4 4 4 5 ... ... JOKER0 JOKER1
// ������0 1 2 3 4 5 6 7 8 ... ... 52     53
//
//
// Ŀǰʹ�õ��˿�λͼ�Ǵ������ҵ��ģ������Լ����ģ����������£�
//
// ��Ƭ��A  2  3  4  5  6  7  8  9  0  J  Q  K
// ÷����A  2  3  4  5  6  7  8  9  0  J  Q  K
// ���ң�A  2  3  4  5  6  7  8  9  0  J  Q  K
// ���ң�A  2  3  4  5  6  7  8  9  0  J  Q  K
//       С �� ��
//
//
// Ϊ���ڻ�ͼ�˿��ƣ����������ת����ʹ�˿���������ȷ��Ӧ����Ӧ��λͼ��
//
// ˵����
//  �˿��Ƶ�λͼ����5�У�ÿ��13����λͼ�����У�ÿ5��ֻ��3��λͼ������
//  ��λͼ�ߴ�Ϊ96x71���ء���������Ϊ�����ң����ϵ��£�����Ϊ0��54��
//
//  ��ͼ��������˿��������λͼ�����󣬳���13�õ�λͼ���ڵ��У�ȡģ13
//  �õ�λͼ���ڵ��У��ٸ��ݵ����Ƶĳߴ磬���ɷ�����Ƴ��˿��ơ�
//
//  ���������˿�����Ϊ-1����ʾȡ�˿��Ʊ���λͼ��
//
//
//  ���˿�������ת���������������˿���λͼ����
int CommonUtil::PokerIndexToBmpIndex(int nPokerIndex)
{
    int value; // �˿��Ƶ�ֵ
    int color; // �˿��ƵĻ�ɫ

    if (nPokerIndex == LORD_POKER_BACK_INDEX) return 55; // �����˿��Ʊ���

    if ((nPokerIndex <= POKER_BACK_INDEX) || (nPokerIndex >= 54)) return 54; // ����
    if (nPokerIndex == 52) return 52; // С��
    if (nPokerIndex == 53) return 53; // ����

    color = nPokerIndex % 4;
    value = nPokerIndex / 4;

    // ����λͼ�����У������޸�
#if 0
    switch (value) {
        case 0:     value = 2;      break;
        case 1:     value = 3;      break;
        case 2:     value = 4;      break;
        case 3:     value = 5;      break;
        case 4:     value = 6;      break;
        case 5:     value = 7;      break;
        case 6:     value = 8;      break;
        case 7:     value = 9;      break;
        case 8:     value = 10;     break;
        case 9:     value = 11;     break;
        case 10:    value = 12;     break;
        case 11:    value = 0;      break;
        case 12:    value = 1;      break;
    }
#else
    value = (value + 2) % 13;
#endif

    return value + color * 13;
}

// ���������ļ�
void CommonUtil::PlaySound(UINT nSndType)
{
    switch (nSndType) {
        case SND_GAME_START:
            sndPlaySound(_T("start.wav"), SND_ASYNC);
            break;

        case SND_GAME_OUTPUT_CARD:
            sndPlaySound(_T("outcard.wav"), SND_ASYNC);
            break;

        case SND_GAME_WIN:
            sndPlaySound(_T("win.wav"), SND_ASYNC);
            break;

        case SND_GAME_BOMB:
            sndPlaySound(_T("bomb.wav"), SND_ASYNC);
            break;

        case SND_GAME_SCORE0_F:
            sndPlaySound(_T("0_f.wav"), SND_ASYNC);
            break;

        case SND_GAME_RUNAWAY:
            sndPlaySound(_T("runaway.wav"), SND_ASYNC);
            break;

        case SND_GAME_TIMEOUT:
            sndPlaySound(_T("timeout.wav"), SND_ASYNC);
            break;

        case SND_GAME_SCORE1_F:
            sndPlaySound(_T("1_f.wav"), SND_ASYNC);
            break;

        case SND_GAME_SCORE2_F:
            sndPlaySound(_T("2_f.wav"), SND_ASYNC);
            break;

        case SND_GAME_SCORE3_F:
            sndPlaySound(_T("3_f.wav"), SND_ASYNC);
            break;

        case SND_GAME_SCORE0_M:
            sndPlaySound(_T("0_m.wav"), SND_ASYNC);
            break;

        case SND_GAME_SCORE1_M:
            sndPlaySound(_T("1_m.wav"), SND_ASYNC);
            break;

        case SND_GAME_SCORE2_M:
            sndPlaySound(_T("2_m.wav"), SND_ASYNC);
            break;

        case SND_GAME_SCORE3_M:
            sndPlaySound(_T("3_m.wav"), SND_ASYNC);
            break;
    }
}

#ifdef _USE_CRC16
// ���� 16 λ CRC ��
unsigned long CommonUtil::Table_CRC16[256] = { 0 };
BOOL CommonUtil::bCrc16TableBuilded = FALSE;

void CommonUtil::BuildTable16(unsigned short aPoly)
{
    unsigned short i, j;
    unsigned short nData;
    unsigned short nAccum;

    for (i = 0; i < 256; i++) {
        nData = (unsigned short)(i << 8);
        nAccum = 0;
        for (j = 0; j < 8; j++) {
            if ((nData ^ nAccum) & 0x8000) {
                nAccum = (nAccum << 1) ^ aPoly;
            } else {
                nAccum <<= 1;
            }
            nData <<= 1;
        }
        Table_CRC16[i] = (unsigned long)nAccum;
    }
}

// ���� 16 λ CRC ֵ��CRC-16 �� CRC-CCITT
unsigned short CommonUtil::CRC16(unsigned char * aData, unsigned long aSize)
{
    unsigned long i;
    unsigned short nAccum = 0;

    if (bCrc16TableBuilded == FALSE) {
        BuildTable16(cnCRC_16); // or cnCRC_CCITT
        bCrc16TableBuilded = TRUE;
    }

    for (i = 0; i < aSize; i++) {
        nAccum = (nAccum << 8) ^ (unsigned short)Table_CRC16[(nAccum >> 8) ^ *aData++];
    }

    return nAccum;
}
#endif

#ifdef _USE_CRC32
// ���� 32 λ CRC ��
unsigned long CommonUtil::Table_CRC32[256] = { 0 };
BOOL CommonUtil::bCrc32TableBuilded = FALSE;

void CommonUtil::BuildTable32(unsigned long aPoly)
{ 
    unsigned long i, j;
    unsigned long nData;
    unsigned long nAccum;

    for (i = 0; i < 256; i++) {
        nData = (unsigned long)(i << 24);
        nAccum = 0;
        for (j = 0; j < 8; j++) {
            if ((nData ^ nAccum) & 0x80000000) {
                nAccum = (nAccum << 1) ^ aPoly;
            } else {
                nAccum <<= 1;
            }
            nData <<= 1;
        }
        Table_CRC32[i] = nAccum;
    }
}

// ���� 32 λ CRC-32 ֵ 
unsigned long CommonUtil::CRC32(unsigned char * aData, unsigned long aSize)
{
    unsigned long i;
    unsigned long nAccum = 0;

    if (bCrc32TableBuilded == FALSE) {
        BuildTable32(cnCRC_32);
        bCrc32TableBuilded = TRUE;
    }

    for (i = 0; i < aSize; i++) {
        nAccum = (nAccum << 8) ^ Table_CRC32[(nAccum >> 24) ^ *aData++];
    }

    return nAccum; 
}
#endif

#ifdef PARENT_PAINT_CHILD
void CommonUtil::ParentPaintChild(HWND hWndParent, HWND hWndChild, LPRECT lpRectOfChild, BOOL bErase)
{
    RECT rect;

    if (lpRectOfChild == NULL) {
        GetClientRect(hWndChild, &rect);
    } else {
        CopyRect(&rect, lpRectOfChild);
    }

    ClientRectToScreen(hWndChild, &rect);
    ScreenRectToClient(hWndParent, &rect);

    InvalidateRect(hWndParent, &rect, bErase);
}
#endif

void CommonUtil::ClientRectToScreen(HWND hWnd, LPRECT lpRect)
{
    assert(lpRect != NULL);

    POINT ptLT = { lpRect->left, lpRect->top };
    POINT ptRB = { lpRect->right, lpRect->bottom };

    ClientToScreen(hWnd, &ptLT);
    ClientToScreen(hWnd, &ptRB);

    SetRect(lpRect, ptLT.x, ptLT.y, ptRB.x, ptRB.y);
}

void CommonUtil::ScreenRectToClient(HWND hWnd, LPRECT lpRect)
{
    assert(lpRect != NULL);

    POINT ptLT = { lpRect->left, lpRect->top };
    POINT ptRB = { lpRect->right, lpRect->bottom };

    ScreenToClient(hWnd, &ptLT);
    ScreenToClient(hWnd, &ptRB);

    SetRect(lpRect, ptLT.x, ptLT.y, ptRB.x, ptRB.y);
}

int CommonUtil::QuickSortPartition(int a[], int low, int high)
{
    int i, j;
    int compare;

    i = low;
    j = high;
    compare = a[low];

    while (i < j) { 
        while ((i < j) && (compare <= a[j])) {
            j--;
        }

        if (i < j) {
            a[i] = a[j];
            i++;
        }

        while ((i < j) && (compare > a[i])) {
            i++;
        }

        if (i < j) {
            a[j] = a[i];
            j--;
        }
    }

    a[i] = compare;

    return i;
}

void CommonUtil::QuickSort(int a[], int low, int high)
{
    int pos;

    if (low < high) {
        pos = QuickSortPartition(a, low, high);
        QuickSort(a, low, pos - 1);
        QuickSort(a, pos + 1, high);
    }
}
