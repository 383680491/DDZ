//
// File: GamerVisualWnd.h
//
#pragma once

#define GAMER_VISUAL_WND_CLASS_NAME             _T("_GAMER_VISUAL_WND_")

#define GVW_TEXT_CLR                            RGB(255,255,255)

#define GVW_USER_VISUAL_WIDTH                   75
#define GVW_USER_VISUAL_HEIGHT                  125

#define GVW_USER_VISUAL_WND_WIDTH               GVW_USER_VISUAL_WIDTH
#define GVW_USER_VISUAL_WND_HEIGHT              (GVW_USER_VISUAL_HEIGHT + 20)


class GamerVisualWnd
{
    HWND        m_hWnd;
    HWND        m_hWndParent;
    HINSTANCE   m_hInstance;

    int         m_nUserId;

    //
    // ��������ҪĿ������ʾGIF������Ŀǰ��ʹ��PictureEx�࣬���䲻��ʵ��͸��Ч����
    // ���ｫ�����ڱ������������������ݸ�PictureEx��ͬʱ��Ҫ��PictureEx����Ҫ
    // �޸Ĳ���ʵ��GIF͸��Ч�������������޸Ľ��ʺϱ�����������
    //
    HBITMAP     m_hBkBitmap; // ���游���ڱ���λͼ
    BOOL        m_bReloadBkBitmap; // ���ߴ�仯ʱ��ָʾ���»�ȡ�����ڱ���

    CPictureEx* m_PicVisual;

public:
    GamerVisualWnd(void);
    ~GamerVisualWnd(void);

public:
    static ATOM GamerVisualWndRegister(HINSTANCE hInstance);
    static LRESULT CALLBACK GamerVisualWndProc(HWND, UINT, WPARAM, LPARAM);

protected:
    void OnPaint(HDC hdc);
    void OnRButtonDown(WPARAM wParam, LPARAM lParam);
    void SaveParentBackground(void);
    void SaveParentBackground(HDC parentDC);

public:
    HWND Create(int x, int y, int cx, int cy, HWND hWndParent, HINSTANCE hInstance);
    void SetGamerId(int nUserId);
    void SetWindowRect(int x, int y, int cx, int cy);

    void ParentWndSizeChanged(void);

#ifdef PARENT_PAINT_CHILD
    void ParentPaintChild(HDC parentDC);
#endif

};

