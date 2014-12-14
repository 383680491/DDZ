//
// MyButton.cpp
//
//  Self-created image button which is not powerful while useful enough.
//
//  ʹ�÷�����
//      1.  �ڳ���ʼ��ע�ᴰ���࣬MyButtonRegister()
//      2.  Create()��SetBitmap()
//      3.  �ڸ���������Ӧ��Ϣ WM_COMMAND
//
//
//  ע�⣺��ťλͼҪ��Ϊ5����ͬ��С����ͬ��ɫ�İ�ť�ų�һ�С�
//

#include "stdafx.h"
#include "MyButton.h"

// ���캯��
MyButton::MyButton(void)
{
    m_hInstance     = NULL;
    m_hWndParent    = NULL;

    m_nID           = 0;
    m_bShow         = TRUE;
    m_bEnable       = TRUE;
    m_bLButtonDown  = FALSE;
    m_bMouseEnter   = FALSE;
    m_hBtnBitmap    = NULL;
    m_clrMask       = RGB(255,255,255);

    m_clrText       = RGB(0,0,0);
    ZeroMemory(&m_szText, sizeof(m_szText));
}

// ��������
MyButton::~MyButton(void)
{
}

// ��ע�ắ��
ATOM MyButton::MyButtonRegister(HINSTANCE hInstance)
{
    WNDCLASSEX  wcex;

    wcex.cbSize             = sizeof(WNDCLASSEX);
    wcex.lpszClassName      = MY_BUTTON_CLASS_NAME;
    wcex.lpfnWndProc	    = MyButtonProc;
    wcex.style			    = CS_HREDRAW | CS_VREDRAW;
    wcex.cbClsExtra		    = 0;
    wcex.cbWndExtra		    = 0;
    wcex.hInstance		    = hInstance;
    wcex.hIcon			    = NULL;
    wcex.hCursor		    = LoadCursor(NULL, IDC_HAND);
    wcex.hbrBackground	    = NULL;
    wcex.lpszMenuName	    = NULL;
    wcex.hIconSm		    = NULL;

    return RegisterClassEx(&wcex);
}

// ���ڴ�����
LRESULT
CALLBACK
MyButton::MyButtonProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    MyButton* btn;
    LPCREATESTRUCT lpcs;
    
    btn = (MyButton*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (nMsg) {
        case WM_CREATE:
            lpcs = (LPCREATESTRUCT)lParam;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(lpcs->lpCreateParams));
            break;

        case WM_DESTROY:
            if (btn->m_hBtnBitmap != NULL) {
                DeleteObject(btn->m_hBtnBitmap);
                btn->m_hBtnBitmap = NULL;
            }
            SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            btn->OnPaint(hdc);
            EndPaint(hWnd, &ps);
            break;

        case WM_RBUTTONDOWN:
            SendMessage(btn->m_hWndParent, nMsg, wParam, lParam);
            break;

        case WM_LBUTTONDOWN:
            btn->OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_LBUTTONUP:
            btn->OnLButtonUp(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MOUSEMOVE:
            btn->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MOUSELEAVE:
            btn->OnMouseLeave(LOWORD(lParam), HIWORD(lParam));
            break;

        default:
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
    }

    return 0;
}

// ������ť
HWND MyButton::Create(DWORD dwStyle, int x, int y, int nWidth, int nHeight,
                      HWND hWndParent, UINT id, HINSTANCE hInstance)
{
    m_hWnd = CreateWindowEx(
        0,
        MY_BUTTON_CLASS_NAME,
        _T(""),
        dwStyle,
        x,
        y,
        nWidth,
        nHeight, 
        hWndParent,
        (HMENU)id,
        hInstance,
        this); // �� this ָ�봫�ݸ� WM_CREATE ��������

    m_hWndParent = hWndParent;
    m_hInstance = hInstance;
    m_nID = id;

    return m_hWnd;
}

// ���ð�ť���ڵľ���
BOOL MyButton::SetWindowRect(int x, int y, int cx, int cy)
{
    MoveWindow(m_hWnd, x, y, cx, cy, TRUE);
    return TRUE;
}

// ���ð�ťλͼ������λͼ��Դ��
BOOL MyButton::SetBitmap(UINT nResID, COLORREF clrMask)
{
    return SetBitmap(m_hInstance, MAKEINTRESOURCE(nResID), clrMask);
}

// ���ð�ťλͼ������λͼ��Դ��
BOOL MyButton::SetBitmap(HMODULE hModule, LPCTSTR lpszResName, COLORREF clrMask)
{
    m_clrMask = clrMask;

    if (m_hBtnBitmap != NULL) {
        DeleteObject(m_hBtnBitmap);
        m_hBtnBitmap = NULL;
    }

    m_hBtnBitmap = (HBITMAP)LoadImage(hModule, lpszResName, IMAGE_BITMAP,
        0, 0, LR_DEFAULTCOLOR);

    if (m_bShow == TRUE) {
        InvalidateRect(m_hWnd, NULL, FALSE);
    }

    return TRUE;
}

// ���ð�ť�ı�
BOOL MyButton::SetText(LPCTSTR lpszText, COLORREF clrText /*= RGB(0,0,0)*/)
{
    m_clrText = clrText;
    _tcscpy_s(m_szText, MY_BUTTON_TEXT_LEN, lpszText);
    m_szText[MY_BUTTON_TEXT_LEN - 1] = _T('\0');

    if (m_bShow == TRUE) {
        InvalidateRect(m_hWnd, NULL, FALSE);
    }

    return TRUE;
}

// ʹ�ܰ�ť
BOOL MyButton::Enable(BOOL bEnable /* = TRUE */)
{
    m_bEnable = bEnable;

    if (m_bShow == TRUE) {
        InvalidateRect(m_hWnd, NULL, FALSE);
    }

    EnableWindow(m_hWnd, bEnable);
    return TRUE;
}

// ��ʾ�����ذ�ť
BOOL MyButton::Show(BOOL bShow /* = TRUE */)
{
    if (m_bShow != bShow) {
        m_bShow = bShow;
        ShowWindow(m_hWnd, bShow ? SW_SHOW : SW_HIDE);
    }

    return TRUE;
}

// ��ť�Ƿ�ɼ�
BOOL MyButton::IsVisible(void)
{
    return m_bShow;
}

// ��ͼ
void MyButton::OnPaint(HDC hdc)
{
    if (m_hBtnBitmap == NULL) { return; }

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    int cx  = rect.right - rect.left;
    int cy  = rect.bottom - rect.top;

    // ��ȡ�����λͼ��Դ�ĳߴ�
    BITMAP bi = { 0 };
    GetObject(m_hBtnBitmap, sizeof(BITMAP), &bi);
    int bmpWidth = bi.bmWidth;
    int bmpHeight = bi.bmHeight;

    int w = bmpWidth / 5;   // ������ťλͼ���
    int h = bmpHeight;      // ������ťλͼ�߶�

    int index = 0;

    // ��ťλͼΪ5����ͬ��ɫ�İ�ť��ȷ��ѡ���ĸ���ťλͼ
    if (m_bEnable == FALSE) {
        index = 4;
    } else if ((m_bMouseEnter == TRUE) && (m_bLButtonDown == FALSE)) {
        index = 3;
    } else if (m_bLButtonDown == FALSE) {
        index = 0;
    } else {
        index = 1;
    }

    int startx = w * index;

    // ������ʱDC�����ڽ���ťλͼѡ���DC
    HDC memTempDC = CreateCompatibleDC(hdc);
    assert(memTempDC != NULL);

    // �����ڴ�DC
    HDC memDC = CreateCompatibleDC(hdc);
    assert(memDC != NULL);

    // ��������DC
    HDC memMaskDC = CreateCompatibleDC(hdc);
    assert(memMaskDC != NULL);

    // �������׼��ˢ����Ļ���ڴ�DC
    HDC lastMemDC = CreateCompatibleDC(hdc);
    assert(lastMemDC != NULL);

    // ѡ����ťλͼ
    HBITMAP hOldTempBmp = (HBITMAP)SelectObject(memTempDC, m_hBtnBitmap);

    // �ڴ�DC��λͼ
    HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, bmpWidth, bmpHeight); // 5����ť�ĳߴ�
    assert(hMemBmp != NULL);

    HBITMAP hOldMemBmp = (HBITMAP)SelectObject(memDC, hMemBmp);

    // ����ťλͼ�������ڴ�DC��λͼ
    BitBlt(memDC, 0, 0, bmpWidth, bmpHeight, memTempDC, 0, 0, SRCCOPY);

    // MASKλͼ
    HBITMAP hMaskBmp = CreateBitmap(w, h, 1, 1, NULL); // ������ť�ĳߴ�
    assert(hMaskBmp != NULL);

    HBITMAP hOldMaskBmp = (HBITMAP)SelectObject(memMaskDC, hMaskBmp);

    // ���ˢ����Ļ��λͼ
    HBITMAP hLastMemBmp = CreateCompatibleBitmap(hdc, w, h); // ������ť�ĳߴ�
    assert(hLastMemBmp != NULL);

    HBITMAP hLastOldMemBmp = (HBITMAP)SelectObject(lastMemDC, hLastMemBmp);

    {// �������ڱ������ƹ������Ա�ʵ��Բ�ǰ�ť��Ե����͸��
        HDC parentDC = GetDC(m_hWndParent);
        assert(parentDC != NULL);

        POINT pt = { 0, 0 };
        ClientToScreen(m_hWnd, &pt);
        ScreenToClient(m_hWndParent, &pt);
        BitBlt(lastMemDC, 0, 0, w, h, parentDC, pt.x, pt.y, SRCCOPY);
        ReleaseDC(m_hWndParent, parentDC);
    }

    //{{ Start to draw the button image in memory
    SetBkColor(memDC, m_clrMask);
    BitBlt(memMaskDC, 0, 0, w, h, memDC, startx, 0, SRCCOPY);
    BitBlt(lastMemDC, 0, 0, w, h, memDC, startx, 0, SRCINVERT);
    BitBlt(lastMemDC, 0, 0, w, h, memMaskDC, 0, 0, SRCAND);
    BitBlt(lastMemDC, 0, 0, w, h, memDC, startx, 0, SRCINVERT);
    //}} Finish drawing

    StretchBlt(hdc, 0, 0, cx, cy, lastMemDC, 0, 0, w, h, SRCCOPY);

    // �ı�
    if (_tcslen(m_szText) > 0) {
        RECT rcText = { 0, 0, cx, cy };

        LOGFONT logFont = { 0 };
        logFont.lfCharSet = GB2312_CHARSET;
        logFont.lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        logFont.lfWeight = FW_BOLD;
        _tcscpy_s(logFont.lfFaceName, sizeof(logFont.lfFaceName) / sizeof(logFont.lfFaceName[0]), _T("������"));
        HFONT hFont = CreateFontIndirect(&logFont);

        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
        COLORREF clrOldText = SetTextColor(hdc, m_clrText);
        int mode = SetBkMode(hdc, TRANSPARENT);

        DrawText(hdc, m_szText, _tcslen(m_szText), &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SetBkMode(hdc, mode);
        SetTextColor(hdc, clrOldText);
        SelectObject(hdc, oldFont);

        DeleteObject(hFont);
    }

    SelectObject(lastMemDC, hLastOldMemBmp);
    SelectObject(memMaskDC, hOldMaskBmp);
    SelectObject(memDC, hOldMemBmp);
    SelectObject(memTempDC, hOldTempBmp);

    DeleteObject(hLastMemBmp);
    DeleteObject(hMaskBmp);
    DeleteObject(hMemBmp);

    DeleteDC(lastMemDC);
    DeleteDC(memMaskDC);
    DeleteDC(memDC);
    DeleteDC(memTempDC);
}

// ��갴��
void MyButton::OnLButtonDown(int x, int y)
{
    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);

    if (m_bLButtonDown == FALSE) {
        SetCapture(m_hWnd);

        m_bLButtonDown = TRUE;

        InvalidateRect(m_hWnd, NULL, FALSE);
    }
}

// ��굯��
void MyButton::OnLButtonUp(int x, int y)
{
    if (m_bLButtonDown == TRUE) {
        ReleaseCapture();

        //
        // ��� LMouseUP ʱ������껹ͣ���ڰ�ť���ڵĿͻ�������ʾ��ȷ�����
        //
        POINT pt;
        pt.x = x;
        pt.y = y;

        RECT rect;
        GetClientRect(m_hWnd, &rect);

        if (PtInRect(&rect, pt)) {
            SendMessage(m_hWndParent, WM_COMMAND, m_nID, 0);
        }

        m_bLButtonDown = FALSE;

        InvalidateRect(m_hWnd, NULL, FALSE);
    }
}

// ����ƶ�
void MyButton::OnMouseMove(int x, int y)
{
    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);

    if (m_bMouseEnter == FALSE) {
        m_bMouseEnter = TRUE;

        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.hwndTrack = m_hWnd;
        tme.dwHoverTime = HOVER_DEFAULT;
        tme.dwFlags = TME_LEAVE;
        TrackMouseEvent(&tme);

        InvalidateRect(m_hWnd, NULL, FALSE);
    }
}

// ����뿪
void MyButton::OnMouseLeave(int x, int y)
{
    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);

    m_bMouseEnter = FALSE;

    InvalidateRect(m_hWnd, NULL, FALSE);
}

// �������ڻ���ʱ���ñ������԰�ť���л���
#ifdef PARENT_PAINT_CHILD
void MyButton::ParentPaintChild(HDC parentDC)
{
    if (m_hBtnBitmap == NULL) { return; }

    if (m_bShow == FALSE) { return; }

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    int cx  = rect.right - rect.left;
    int cy  = rect.bottom - rect.top;

    // ��ȡ�����λͼ��Դ�ĳߴ�
    BITMAP bi = { 0 };
    GetObject(m_hBtnBitmap, sizeof(BITMAP), &bi);
    int bmpWidth = bi.bmWidth;
    int bmpHeight = bi.bmHeight;

    int w = bmpWidth / 5;   // ������ťλͼ���
    int h = bmpHeight;      // ������ťλͼ�߶�

    int index = 0;

    // ��ťλͼΪ5����ͬ��ɫ�İ�ť��ȷ��ѡ���ĸ���ťλͼ
    if (m_bEnable == FALSE) {
        index = 4;
    } else if ((m_bMouseEnter == TRUE) && (m_bLButtonDown == FALSE)) {
        index = 3;
    } else if (m_bLButtonDown == FALSE) {
        index = 0;
    } else {
        index = 1;
    }

    int startx = w * index;

    // ������ʱDC�����ڽ���ťλͼѡ���DC
    HDC memTempDC = CreateCompatibleDC(parentDC);
    assert(memTempDC != NULL);

    // �����ڴ�DC
    HDC memDC = CreateCompatibleDC(parentDC);
    assert(memDC != NULL);

    // ��������DC
    HDC memMaskDC = CreateCompatibleDC(parentDC);
    assert(memMaskDC != NULL);

    // �������׼��ˢ����Ļ���ڴ�DC
    HDC lastMemDC = CreateCompatibleDC(parentDC);
    assert(lastMemDC != NULL);

    // ѡ����ťλͼ
    HBITMAP hOldTempBmp = (HBITMAP)SelectObject(memTempDC, m_hBtnBitmap);

    // �ڴ�DC��λͼ
    HBITMAP hMemBmp = CreateCompatibleBitmap(parentDC, bmpWidth, bmpHeight); // 5����ť�ĳߴ�
    assert(hMemBmp != NULL);

    HBITMAP hOldMemBmp = (HBITMAP)SelectObject(memDC, hMemBmp);

    // ����ťλͼ�������ڴ�DC��λͼ
    BitBlt(memDC, 0, 0, bmpWidth, bmpHeight, memTempDC, 0, 0, SRCCOPY);

    // MASKλͼ
    HBITMAP hMaskBmp = CreateBitmap(w, h, 1, 1, NULL); // ������ť�ĳߴ�
    assert(hMaskBmp != NULL);

    HBITMAP hOldMaskBmp = (HBITMAP)SelectObject(memMaskDC, hMaskBmp);

    // ���ˢ����Ļ��λͼ
    HBITMAP hLastMemBmp = CreateCompatibleBitmap(parentDC, w, h); // ������ť�ĳߴ�
    assert(hLastMemBmp != NULL);

    HBITMAP hLastOldMemBmp = (HBITMAP)SelectObject(lastMemDC, hLastMemBmp);

    {// �������ڱ������ƹ������Ա�ʵ��Բ�ǰ�ť��Ե����͸��
        POINT pt = { 0, 0 };
        ClientToScreen(m_hWnd, &pt);
        ScreenToClient(m_hWndParent, &pt);
        BitBlt(lastMemDC, 0, 0, w, h, parentDC, pt.x, pt.y, SRCCOPY);
    }

    //{{ Start to draw the button image in memory
    SetBkColor(memDC, m_clrMask);
    BitBlt(memMaskDC, 0, 0, w, h, memDC, startx, 0, SRCCOPY);
    BitBlt(lastMemDC, 0, 0, w, h, memDC, startx, 0, SRCINVERT);
    BitBlt(lastMemDC, 0, 0, w, h, memMaskDC, 0, 0, SRCAND);
    BitBlt(lastMemDC, 0, 0, w, h, memDC, startx, 0, SRCINVERT);
    //}} Finish drawing

    POINT pt = { 0, 0 };
    ClientToScreen(m_hWnd, &pt);
    ScreenToClient(m_hWndParent, &pt);

    StretchBlt(parentDC, pt.x, pt.y, cx, cy, lastMemDC, 0, 0, w, h, SRCCOPY);

    // �ı�
    if (_tcslen(m_szText) > 0) {
        RECT rcText = { pt.x, pt.y, pt.x + cx, pt.y + cy };

        LOGFONT logFont = { 0 };
        logFont.lfCharSet = GB2312_CHARSET;
        logFont.lfHeight = -MulDiv(10, GetDeviceCaps(parentDC, LOGPIXELSY), 72);
        logFont.lfWeight = FW_BOLD;
        _tcscpy_s(logFont.lfFaceName, sizeof(logFont.lfFaceName) / sizeof(logFont.lfFaceName[0]), _T("������"));
        HFONT hFont = CreateFontIndirect(&logFont);

        HFONT oldFont = (HFONT)SelectObject(parentDC, hFont);
        COLORREF clrOldText = SetTextColor(parentDC, m_clrText);
        int mode = SetBkMode(parentDC, TRANSPARENT);

        DrawText(parentDC, m_szText, _tcslen(m_szText), &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SetBkMode(parentDC, mode);
        SetTextColor(parentDC, clrOldText);
        SelectObject(parentDC, oldFont);

        DeleteObject(hFont);
    }

    SelectObject(lastMemDC, hLastOldMemBmp);
    SelectObject(memMaskDC, hOldMaskBmp);
    SelectObject(memDC, hOldMemBmp);
    SelectObject(memTempDC, hOldTempBmp);

    DeleteObject(hLastMemBmp);
    DeleteObject(hMaskBmp);
    DeleteObject(hMemBmp);

    DeleteDC(lastMemDC);
    DeleteDC(memMaskDC);
    DeleteDC(memDC);
    DeleteDC(memTempDC);

    //
    // Not validate the button client area.
    // The button may expect some mouse messages for painting.
    //
    //ValidateRect(m_hWnd, NULL);
}
#endif

