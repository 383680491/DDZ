//
// CurPokerWnd.h
//
//      Current Poker Window
//
#pragma once

#define CUR_POKER_WND_CLASS_NAME    _T("_CUR_POKER_WND_")

#define INVALID_COORD               -1

#define SEL_CARDS_FRAME_WIDTH       4
#define SEL_CARDS_FRAME_CLR         RGB(255,255,0)

class CurPokerWnd
{
private:
    static const int       m_HSPACE = CPW_POKER_H_SPACE; // ��������ʱ�������Ƶļ��
    static const int       m_VSPACE = CPW_POKER_V_SPACE; // ��������ʱ�������Ƶļ��
    static const int       m_POPSPACE = CPW_POKER_POPUP_SPACE; // ѡ�е��Ƶ����ļ��

    static const int       m_CARDHEIGHT = POKER_BMP_UNIT_HEIGHT; // �˿��Ƶĸ߶�
    static const int       m_CARDWIDTH = POKER_BMP_UNIT_WIDTH; // �˿��ƵĿ��

private:
    // ��ǰ�ƣ�Ԫ��Ϊ0��53���˿�����
    int             m_nCurPokerCards[PLAYER_MAX_CARDS_NUM];
    int             m_nCurPokerCardsNum;

    // ָʾ��Щ�Ʊ�ѡ�У�Ԫ��ȡֵ0��20
    BOOL            m_bPokerCardSelected[PLAYER_MAX_CARDS_NUM];

private:
    HINSTANCE       m_hInstance;
    HWND            m_hWndParent;

private:
    UINT            m_nID;
    HWND            m_hWnd;

    // �˿���λͼ����������ڴ���������ʱ���ݲ���������������ǰ��ɾ����λͼ����
    HBITMAP         m_hPokerBMP;

    HBITMAP         m_hBkBitmap; // ���游���ڵı��������ڻ���ʱ������˸��
    BOOL            m_bReloadBkBitmap; // ���ߴ�仯ʱ��ָʾ���»�ȡ�����ڱ���

private:
    BOOL            m_bShowPoker;       // ��ʾ�˿���
    BOOL            m_bHorizontal;      // �˿����Ƿ�Ϊˮƽ����

    BOOL            m_bIsLord;          // ������Ƿ�Ϊ����

    BOOL            m_bAllowPokerSelect; // �˿����Ƿ���Ա�ѡ�񣬣��Թ��߲���ѡ���ƣ�

    BOOL            m_bLButtonDown;
    POINT           m_ptLButtonDown;

    BOOL            m_bLBtnDownAndMouseMove;
    POINT           m_ptMouseMove;

private:
    void OnPaint(HDC hdc);
    void SaveParentBackground(void);
    void SaveParentBackground(HDC parentDC);

    void PaintHorizontally(HDC hdc);
    void PaintVertically(HDC hdc);

    void OnLButtonDown(int x, int y);
    void OnLButtonUp(int x, int y);
    void OnMouseMove(int x, int y);

    void ClickCard(int x, int y);
    void SelectCardRange(int xStart, int xEnd);

    void GetCardRangeSelectRect(LPRECT lpRect);
    void AdjustWindowRect(void);

public:
    CurPokerWnd(void);
    ~CurPokerWnd(void);

public:
    static ATOM CurPokerWndRegister(HINSTANCE hInstance);
    static LRESULT CALLBACK CurPokerWndProc(HWND, UINT, WPARAM, LPARAM);

public:
    HWND Create(DWORD dwStyle,
        int xCenter,
        int yCenter,
        HWND hWndParent,
        UINT nID,
        HINSTANCE hInstance,
        HBITMAP hPokerBMP,
        BOOL bShowPoker,
        BOOL bHorizontal);

    BOOL ShowPoker(BOOL bShow = TRUE);
    BOOL SetCards(int poker[], int num);
    BOOL GetCards(int poker[], int* num);
    BOOL GetSelectedCards(int poker[], int num, int* selnum);
    BOOL RemoveSelectedCards(void);
    BOOL RemoveCards(int poker[], int num);
    BOOL AddCards(int poker[], int num);
    BOOL SelectCardsFromIndex(int index[], int num);
    BOOL UnSelectAllCards(void);
    BOOL AllowPokerSelection(BOOL bAllow = TRUE);
    BOOL SetLord(BOOL bLord = TRUE);

    BOOL SetWindowCenterPos(int xCenter, int yCenter);

public:
    void ParentWndSizeChanged(void);

#ifdef PARENT_PAINT_CHILD
    void ParentPaintChild(HDC parentDC);
#endif
};