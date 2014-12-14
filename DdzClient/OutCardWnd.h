//
// OutCardWnd.h
//
//  �ô�����������ʾ��ҳ�����
//
#pragma once

#define MAX_OUT_CARDS_NUM           20

#define OUT_CARD_WND_CLASS_NAME    _T("_OUT_CARD_WND_")

// �˿����ڳ��ƴ����е�λ�ã�ʼ���Ǵ�ֱ���С�����ֻ�ṩˮƽ��ʾ�˿ˡ�
#define CARD_ALIGN_CENTER           0
#define CARD_ALIGN_LEFT             1
#define CARD_ALIGN_RIGHT            2
#define CARD_ALIGN_MAX              3

// �ڳ��ƴ�����ʾ�й���Ϸ��״̬��Ϣ
typedef enum {
    GSI_NONE = 0,
    GSI_READY,
    GSI_SCORE0,
    GSI_SCORE1,
    GSI_SCORE2,
    GSI_SCORE3,
    GSI_PASS
} GAME_STAT_INFO;


class OutCardWnd
{
private:
    static const int       m_HSPACE = OCW_POKER_H_SPACE; // ��������ʱ�������Ƶļ��
    static const int       m_CARDHEIGHT = POKER_BMP_UNIT_HEIGHT; // �˿��Ƶĸ߶�
    static const int       m_CARDWIDTH = POKER_BMP_UNIT_WIDTH; // �˿��ƵĿ��

private:
    BOOL            m_bShowPrevious;

    // ���浱ǰ������
    int             m_nPrevOutCards[MAX_OUT_CARDS_NUM];
    int             m_nPrevOutCardsNum;

    // ������һ�ֳ�����
    int             m_nCurOutCards[MAX_OUT_CARDS_NUM];
    int             m_nCurOutCardsNum;

    int             m_nOutCardsNum;

    int             m_nAlignment;
    BOOL            m_bShowPoker; // ָʾ�Ƿ���ʾ�˿�

    GAME_STAT_INFO  m_StatInfo;

private:
    HINSTANCE       m_hInstance;
    HWND            m_hWndParent;
    HWND            m_hWnd;
    
    // �˿���λͼ����������ڴ���������ʱ���ݲ���������������ǰ��ɾ����λͼ����
    HBITMAP         m_hPokerBMP;

    HBITMAP         m_hBkBitmap; // ���游���ڵı��������ڻ���ʱ������˸��
    BOOL            m_bReloadBkBitmap; // ���ߴ�仯ʱ��ָʾ���»�ȡ�����ڱ���

protected:
    void OnPaint(HDC hdc);
    void SaveParentBackground(void);
    void SaveParentBackground(HDC parentDC);

public:
    OutCardWnd(void);
    ~OutCardWnd(void);

public:
    static ATOM OutCardWndRegister(HINSTANCE hInstance);
    static LRESULT CALLBACK OutCardWndProc(HWND, UINT, WPARAM, LPARAM);

public:
    HWND Create(DWORD dwStyle,
        int xCoord,
        int yCoord,
        int nAlign,
        HWND hWndParent,
        HINSTANCE hInstance,
        HBITMAP hPokerBMP);

    BOOL SetWindowCenterPos(int xCenter, int yCenter);
    BOOL ShowPoker(BOOL bShow = TRUE);
    BOOL SetCards(int poker[], int num);
    BOOL Clear(void);
    BOOL SetCardsAlign(int nAlign = 0);
    BOOL ShowPrevRound(BOOL bShowPrev = TRUE);
    BOOL SetGameStatInfo(GAME_STAT_INFO info);

public:
    void ParentWndSizeChanged(void);

#ifdef PARENT_PAINT_CHILD
    void ParentPaintChild(HDC parentDC);
#endif
};