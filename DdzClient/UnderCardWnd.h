//
// UnderCardWnd.h
//

#pragma once

#define UNDER_CARDS_NUM                3
#define UNDER_CARD_WND_CLASS_NAME      _T("_UNDER_CARD_WND_")


class UnderCardWnd
{
private:
    static const int m_CARDWIDTH = POKER_BMP_UNIT_WIDTH;
    static const int m_CARDHEIGHT = POKER_BMP_UNIT_HEIGHT;
    static const int m_HSPACE = UCW_POKER_H_SPACE;

private:
    HWND            m_hWndParent;
    HINSTANCE       m_hInstance;

    HWND            m_hWnd;

    // �˿���λͼ����������ڴ���������ʱ���ݲ���������������ǰ��ɾ����λͼ����
    HBITMAP         m_hPokerBMP;

    int             m_nUnderCards[UNDER_CARDS_NUM];
    int             m_nUnderCardsNum;
    BOOL            m_bShowPoker;

    HBITMAP         m_hBkBitmap; // ���游���ڱ���λͼ
    BOOL            m_bReloadBkBitmap; // ���ߴ�仯ʱ��ָʾ���»�ȡ�����ڱ���

protected:
    void OnPaint(HDC hdc);
    void SaveParentBackground(void);
    void SaveParentBackground(HDC parentDC);

public:
    UnderCardWnd(void);
    ~UnderCardWnd(void);

public:
    static ATOM UnderCardWndRegister(HINSTANCE hInstance);
    static LRESULT CALLBACK UnderCardWndProc(HWND, UINT, WPARAM, LPARAM);

public:
    HWND Create(DWORD dwStyle,
        int xCenter,
        int yCenter,
        HWND hWndParent,
        HINSTANCE hInstance,
        HBITMAP hPokerBMP);

    BOOL ShowPoker(BOOL bShow = TRUE);
    BOOL SetCards(int poker[], int num);
    BOOL GetCards(int poker[], int* num);

    BOOL SetWindowCenterPos(int xCenter, int yCenter);

public:
    void ParentWndSizeChanged(void);

#ifdef PARENT_PAINT_CHILD
    void ParentPaintChild(HDC parentDC);
#endif
};