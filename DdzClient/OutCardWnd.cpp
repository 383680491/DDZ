//
// OutCardWnd.cpp
//
//  �ô�����������ʾ��ҳ�����
//
#include "stdafx.h"
#include "OutCardWnd.h"


// ���캯��
OutCardWnd::OutCardWnd(void)
{
    for (int i = 0; i < MAX_OUT_CARDS_NUM; i++) {
        m_nPrevOutCards[i] = POKER_BACK_INDEX;
        m_nCurOutCards[i] = POKER_BACK_INDEX;
    }

    m_nCurOutCardsNum = 0;
    m_nPrevOutCardsNum = 0;
    m_nOutCardsNum = 0;

    m_hInstance = NULL;
    m_hWndParent = NULL;
    m_hWnd = NULL;

    m_hPokerBMP = NULL;
    m_hBkBitmap = NULL;
    m_bReloadBkBitmap = FALSE;
    m_nAlignment = CARD_ALIGN_CENTER;

    m_bShowPoker = TRUE;
    m_StatInfo = GSI_NONE;
}

// ��������
OutCardWnd::~OutCardWnd(void)
{
}

// ������ע�ắ��
ATOM OutCardWnd::OutCardWndRegister(HINSTANCE hInstance)
{
    WNDCLASSEX  wcex;

    wcex.cbSize             = sizeof(WNDCLASSEX);
    wcex.lpszClassName      = OUT_CARD_WND_CLASS_NAME;
    wcex.lpfnWndProc	    = OutCardWndProc;
    wcex.style			    = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.cbClsExtra		    = 0;
    wcex.cbWndExtra		    = 0;
    wcex.hInstance		    = hInstance;
    wcex.hIcon			    = NULL;
    wcex.hCursor		    = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	    = NULL;
    wcex.lpszMenuName	    = NULL;
    wcex.hIconSm		    = NULL;

    return RegisterClassEx(&wcex);
}

// ���ڴ�����
LRESULT
CALLBACK
OutCardWnd::OutCardWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    LPCREATESTRUCT lpcs;
    OutCardWnd* lpWnd = (OutCardWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (nMsg) {
        case WM_CREATE:
            lpcs = (LPCREATESTRUCT)lParam;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(lpcs->lpCreateParams));
            break;

        case WM_DESTROY:
            SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
            if (lpWnd->m_hBkBitmap != NULL) {
                DeleteObject(lpWnd->m_hBkBitmap);
                lpWnd->m_hBkBitmap = NULL;
            }
            break;

        case WM_SIZE:
            // �����ڳߴ��λ�÷����仯����Ҫ���»�ȡ��������������ĸ����ڱ���
            lpWnd->m_bReloadBkBitmap = TRUE;
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            lpWnd->OnPaint(hdc);
            EndPaint(hWnd, &ps);
            break;

        case WM_RBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            SendMessage(lpWnd->m_hWndParent, nMsg, wParam, lParam);
            break;

        default:
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
    }

    return 0;
}

//
// ��������
//
// �������ڴ������ƴ���ʱ����������ͻ����Σ����Ǵ���һ��������һ�������־��
// ��������־Ϊ���У�˵���������Ϊ���ڵ����ģ����Ϊ����룬��˵���������
// �ھ��ε�����ߴ�ֱ�м�㣻���Ϊ�Ҷ��룬���ʾ�������Ϊ�������ұߴ�ֱ�м�㡣
//
// �����������ݸ�����㣬����һ������������˿��Ƶľ��δ��ڡ�
//
HWND OutCardWnd::Create(DWORD dwStyle, int xCoord, int yCoord, int nAlign,
                        HWND hWndParent, HINSTANCE hInstance, HBITMAP hPokerBMP)
{
    int x, y;
    int nMaxWidth, nMaxHeight;

    // ���ô��ھ���Ϊ��������˿��Ƶľ���
    nMaxHeight = m_CARDHEIGHT;
    nMaxWidth = (MAX_OUT_CARDS_NUM - 1) * m_HSPACE + m_CARDWIDTH;

    if (nAlign == CARD_ALIGN_LEFT) { // �����
        x = xCoord;
    } else if (nAlign == CARD_ALIGN_RIGHT) { // �Ҷ���
        x = xCoord - nMaxWidth;
    } else { // ���ж���
        x = xCoord - nMaxWidth / 2;
    }

    y = yCoord - nMaxHeight / 2;

    m_hWnd = CreateWindowEx(0,
        OUT_CARD_WND_CLASS_NAME,
        _T(""),
        dwStyle,
        x,
        y,
        nMaxWidth,
        nMaxHeight,
        hWndParent,
        NULL,
        hInstance,
        this);

    m_nAlignment = nAlign;
    m_hInstance = hInstance;
    m_hWndParent = hWndParent;
    m_hPokerBMP = hPokerBMP;

    return m_hWnd;
}

// ������ʾ����ʾ���棨�������Թ�ʱ�����ô˺�����ʾ�˿��Ʊ��棩
BOOL OutCardWnd::ShowPoker(BOOL bShow /*= TRUE*/)
{
    if (m_bShowPoker != bShow) {
        m_bShowPoker = bShow;

        if (m_nCurOutCardsNum > 0) {
#ifdef PARENT_PAINT_CHILD
            CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
            InvalidateRect(m_hWnd, NULL, FALSE);
#endif
        }
    }
    return TRUE;
}

// �����˿����ھ��ζ��뷽ʽ��Ҫ���ϼ�����룬��ǰ��Ҿ��ж��룬�¼��Ҷ��룩
BOOL OutCardWnd::SetCardsAlign(int nAlign /*= 0*/)
{
    if ((nAlign >= 0) && (nAlign < CARD_ALIGN_MAX)) {
        if (m_nAlignment != nAlign) {
            m_nAlignment = nAlign;

#ifdef PARENT_PAINT_CHILD
            CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
            InvalidateRect(m_hWnd, NULL, FALSE);
#endif
        }
    }
    return TRUE;
}

// �����ڵĳߴ��Ѿ��ı䣬���ñ�־��ָʾ���ƴ���ʱ����Ҫ���»�ȡ�����ڱ���
void OutCardWnd::ParentWndSizeChanged(void)
{
    m_bReloadBkBitmap = TRUE;
}

// ��ʾ��һ�ֳ�����
BOOL OutCardWnd::ShowPrevRound(BOOL bShowPrev /*= TRUE*/)
{
    m_bShowPrevious = bShowPrev;
    
#ifdef PARENT_PAINT_CHILD
    CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
    InvalidateRect(m_hWnd, NULL, FALSE);
#endif

    return TRUE;
}

// �����˿������ݣ����ƣ�
BOOL OutCardWnd::SetCards(int poker[], int num)
{
    // ����ԭ��������Ϊ��һ��
    for (int i = 0; i < m_nOutCardsNum; i++) {
        m_nPrevOutCards[i] = m_nCurOutCards[i];
    }

    m_nPrevOutCardsNum = m_nOutCardsNum;

    if ((num == 0) && (m_nCurOutCardsNum == 0)) {
        // ���汾�ֳ��Ƶĸ������´γ���ǰ����ֵ����һ��������
        m_nOutCardsNum = m_nCurOutCardsNum;
        return TRUE;
    }

    // ������������
    for (int i = 0; i < num; i++) {
        m_nCurOutCards[i] = poker[i];
    }

    m_nCurOutCardsNum = num;

    // ���汾�ֳ��Ƶĸ������´γ���ǰ����ֵ����һ��������
    m_nOutCardsNum = m_nCurOutCardsNum;

#ifdef PARENT_PAINT_CHILD
    CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
    InvalidateRect(m_hWnd, NULL, FALSE);
#endif

    return TRUE;
}

// ��մ���
BOOL OutCardWnd::Clear(void)
{
    // ����ԭ��������Ϊ��һ��
    for (int i = 0; i < m_nCurOutCardsNum; i++) {
        m_nPrevOutCards[i] = m_nCurOutCards[i];
    }

    m_nPrevOutCardsNum = m_nCurOutCardsNum;

    m_StatInfo = GSI_NONE;
    m_nCurOutCardsNum = 0;

#ifdef PARENT_PAINT_CHILD
    CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
    InvalidateRect(m_hWnd, NULL, FALSE);
#endif

    return TRUE;
}

// ���游���ڱ����������ڳߴ�仯ʱ��Ҫ���±��档
void OutCardWnd::SaveParentBackground(void)
{
    // ȡ������DC
    HDC parentDC = GetDC(m_hWndParent);
    assert(parentDC != NULL);

    SaveParentBackground(parentDC);

    // �ͷŸ�����DC
    ReleaseDC(m_hWndParent, parentDC);
}

// ���游���ڱ���
void OutCardWnd::SaveParentBackground(HDC parentDC)
{
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    int cx = rect.right - rect.left;
    int cy = rect.bottom - rect.top;
    assert((cx != 0) && (cy != 0));

    POINT pt = { rect.left, rect.top };
    ClientToScreen(m_hWnd, &pt);
    ScreenToClient(m_hWndParent, &pt);

    // ����Ҫ����λͼ
    if (m_hBkBitmap == NULL) { 
        m_hBkBitmap = CreateCompatibleBitmap(parentDC, cx, cy);
        assert(m_hBkBitmap != NULL);
    } else {
        // ������������λͼ�������ڳߴ��Ѿ��仯����Ҫ���´�����λͼ
        BITMAP bi = { 0 };
        GetObject(m_hBkBitmap, sizeof(BITMAP), &bi);

        if ((bi.bmWidth != cx) && (bi.bmHeight != cy)) {
            DeleteObject(m_hBkBitmap);
            m_hBkBitmap = CreateCompatibleBitmap(parentDC, cx, cy);
            assert(m_hBkBitmap != NULL);
        }
    }

    // �����븸���ڼ���DC
    HDC memDC = CreateCompatibleDC(parentDC);
    assert(memDC != NULL);

    // ѡ��Ҫ����λͼ
    HBITMAP hOldBmp = (HBITMAP)SelectObject(memDC, m_hBkBitmap);

    // �������ڵı�������Ҫ�����λͼ
    BitBlt(memDC, 0, 0, cx, cy, parentDC, pt.x, pt.y, SRCCOPY);

    // ѡ��ɵ�λͼ���µ�λͼ�Ѿ������� m_hBkBitmap
    SelectObject(memDC, hOldBmp);

    // ɾ�������ļ���DC
    DeleteDC(memDC);
}

// ���ƴ���
void OutCardWnd::OnPaint(HDC hdc)
{
#ifdef PARENT_PAINT_CHILD
    UNREFERENCED_PARAMETER(hdc);
    // Parent window will paint me. Do nothing...
#else
    if (m_bReloadBkBitmap == TRUE) {
        SaveParentBackground();
        m_bReloadBkBitmap = FALSE;
    }

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    int cx = rect.right - rect.left;
    int cy = rect.bottom - rect.top;

    int nCardsNum = m_nCurOutCardsNum;
    int* pCardsData = m_nCurOutCards;

    if (m_bShowPrevious == TRUE) {
        nCardsNum = m_nPrevOutCardsNum;
        pCardsData = m_nPrevOutCards;
    }

    int nPokersLen = m_HSPACE * (nCardsNum - 1) + m_CARDWIDTH;

    int nStartX = 0;
    int nStartY = 0;

    nStartY = (cy - m_CARDHEIGHT) / 2;
    if (nStartY < 0) { nStartY = 0; };

    if (m_nAlignment == CARD_ALIGN_CENTER) {
        nStartX = (cx - nPokersLen) / 2;
    } else if (m_nAlignment == CARD_ALIGN_LEFT) {
        nStartX = rect.left;
    } else if (m_nAlignment == CARD_ALIGN_RIGHT) {
        nStartX = rect.right - nPokersLen;
    }

    if (nStartX < 0) { nStartX = 0; }

    // �����ڴ滺��DC����λͼ
    HDC memDC = CreateCompatibleDC(hdc);
    assert(memDC != NULL);

    HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, cx, cy);
    assert(hMemBmp != NULL);

    HBITMAP hOldMemBmp = (HBITMAP)SelectObject(memDC, hMemBmp);

    // �Ȼ������ڱ���
    HDC tempDC = CreateCompatibleDC(hdc);
    assert(tempDC != NULL);

    HBITMAP hOldTempBmp = (HBITMAP)SelectObject(tempDC, m_hBkBitmap);

    BitBlt(memDC, 0, 0, cx, cy, tempDC, 0, 0, SRCCOPY);

    // �ٻ��˿���
    HDC pokerDC = CreateCompatibleDC(hdc);
    assert(pokerDC != NULL);

    HBITMAP hOldPokerBmp = (HBITMAP)SelectObject(pokerDC, m_hPokerBMP);

    int nCardBmpIndex;
    for (int i = 0; i < nCardsNum; i++) {
        if (m_bShowPoker == TRUE) {
            nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(pCardsData[i]);
        } else {
            nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(POKER_BACK_INDEX);
        }

        BitBlt(memDC,
            nStartX + i * m_HSPACE,
            nStartY,
            m_CARDWIDTH,
            m_CARDHEIGHT,
            pokerDC,
            nCardBmpIndex % 13 * m_CARDWIDTH,
            nCardBmpIndex / 13 * m_CARDHEIGHT,
            SRCCOPY);
    }

    if (m_bShowPrevious != TRUE) {
        if (m_StatInfo != GSI_NONE) {
            int nImageIndex = (int)(m_StatInfo - GSI_READY);
            int xImageStart = 0;
            int yImageStart = (cy - CY_GAME_INFO_IMAGE) / 2;

            if (m_nAlignment == CARD_ALIGN_CENTER) {
                xImageStart = (cx - CX_GAME_INFO_IMAGE) / 2;
            } else if (m_nAlignment == CARD_ALIGN_LEFT) {
                xImageStart = 0;
            } else if (m_nAlignment == CARD_ALIGN_RIGHT) {
                xImageStart = cx - CX_GAME_INFO_IMAGE;
            }

            ImageList_Draw(g_himlGameInfo, nImageIndex, memDC, xImageStart, yImageStart, ILD_NORMAL);
        }
    }

    BitBlt(hdc, 0, 0, cx, cy, memDC, 0, 0, SRCCOPY);

    // ��ԭ���ͷ���Դ
    SelectObject(tempDC, hOldTempBmp);
    SelectObject(pokerDC, hOldPokerBmp);
    SelectObject(memDC, hOldMemBmp);

    DeleteObject(hMemBmp);

    DeleteDC(tempDC);
    DeleteDC(pokerDC);
    DeleteDC(memDC);
#endif
}

// ������ͨ�����ñ����ڵĿͻ��������ĵ��λ�����ƶ������ڡ�����Ϊ����������ϵ
BOOL OutCardWnd::SetWindowCenterPos(int xCenter, int yCenter)
{
    int x, y;
    int nMaxWidth, nMaxHeight;

    // ���ô��ھ���Ϊ��������˿��Ƶľ���
    nMaxHeight = m_CARDHEIGHT;
    nMaxWidth = (MAX_OUT_CARDS_NUM - 1) * m_HSPACE + m_CARDWIDTH;

    x = xCenter - nMaxWidth / 2;
    y = yCenter - nMaxHeight / 2;

    MoveWindow(m_hWnd, x, y, nMaxWidth, nMaxHeight, TRUE);
    return TRUE;
}

// �ڳ��ƴ�����ʾ��Ϸ״̬��Ϣ��׼�������������зֵ�
BOOL OutCardWnd::SetGameStatInfo(GAME_STAT_INFO info)
{
    if (m_StatInfo != info) {
        m_StatInfo = info;

        if (m_bShowPoker == TRUE) {
#ifdef PARENT_PAINT_CHILD
            CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
            InvalidateRect(m_hWnd, NULL, FALSE);
#endif
        }
    }

    return TRUE;
}

#ifdef PARENT_PAINT_CHILD
void OutCardWnd::ParentPaintChild(HDC parentDC)
{
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    int cx = rect.right - rect.left;
    int cy = rect.bottom - rect.top;

    int nCardsNum = m_nCurOutCardsNum;
    int* pCardsData = m_nCurOutCards;

    if (m_bShowPrevious == TRUE) {
        nCardsNum = m_nPrevOutCardsNum;
        pCardsData = m_nPrevOutCards;
    }

    int nPokersLen = m_HSPACE * (nCardsNum - 1) + m_CARDWIDTH;

    int nStartX = 0;
    int nStartY = 0;

    nStartY = (cy - m_CARDHEIGHT) / 2;
    if (nStartY < 0) { nStartY = 0; };

    if (m_nAlignment == CARD_ALIGN_CENTER) {
        nStartX = (cx - nPokersLen) / 2;
    } else if (m_nAlignment == CARD_ALIGN_LEFT) {
        nStartX = rect.left;
    } else if (m_nAlignment == CARD_ALIGN_RIGHT) {
        nStartX = rect.right - nPokersLen;
    }

    if (nStartX < 0) { nStartX = 0; }

    POINT ptParentStart = { nStartX, nStartY };
    ClientToScreen(m_hWnd, &ptParentStart);
    ScreenToClient(m_hWndParent, &ptParentStart);

    nStartX = ptParentStart.x;
    nStartY = ptParentStart.y;

    // ���˿���
    HDC pokerDC = CreateCompatibleDC(parentDC);
    assert(pokerDC != NULL);

    HBITMAP hOldPokerBmp = (HBITMAP)SelectObject(pokerDC, m_hPokerBMP);

    int nCardBmpIndex;
    for (int i = 0; i < nCardsNum; i++) {
        if (m_bShowPoker == TRUE) {
            nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(pCardsData[i]);
        } else {
            nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(POKER_BACK_INDEX);
        }

        BitBlt(parentDC,
            nStartX + i * m_HSPACE,
            nStartY,
            m_CARDWIDTH,
            m_CARDHEIGHT,
            pokerDC,
            nCardBmpIndex % 13 * m_CARDWIDTH,
            nCardBmpIndex / 13 * m_CARDHEIGHT,
            SRCCOPY);
    }

    if (m_bShowPrevious != TRUE) {
        if (m_StatInfo != GSI_NONE) {
            int nImageIndex = (int)(m_StatInfo - GSI_READY);
            int xImageStart = 0;
            int yImageStart = (cy - CY_GAME_INFO_IMAGE) / 2;

            if (m_nAlignment == CARD_ALIGN_CENTER) {
                xImageStart = (cx - CX_GAME_INFO_IMAGE) / 2;
            } else if (m_nAlignment == CARD_ALIGN_LEFT) {
                xImageStart = 0;
            } else if (m_nAlignment == CARD_ALIGN_RIGHT) {
                xImageStart = cx - CX_GAME_INFO_IMAGE;
            }

            POINT ptParentImageStart = { xImageStart, yImageStart };
            ClientToScreen(m_hWnd, &ptParentImageStart);
            ScreenToClient(m_hWndParent, &ptParentImageStart);

            xImageStart = ptParentImageStart.x;
            yImageStart = ptParentImageStart.y;

            ImageList_Draw(g_himlGameInfo, nImageIndex, parentDC, xImageStart, yImageStart, ILD_NORMAL);
        }
    }

    // ��ԭ���ͷ���Դ
    SelectObject(pokerDC, hOldPokerBmp);
    DeleteDC(pokerDC);

    //
    // The following statement will inform system not redraw this window (WM_PAINT)
    // again. For it's already been painted through this function called by it's
    // parent window.
    //
    // System sends WM_PAINT message to parent window, and then sends it to this
    // child window. There's really no need for us to process it any more. Since
    // this macro (PARENT_PAINT_CHILD) is used, and the following statement presented,
    // the window procedure will not receive WM_PAINT unless you call InvalidateRect/Rgn
    // explicitly to do something else with WM_PAINT.
    //
    //ValidateRect(m_hWnd, NULL);
}
#endif


