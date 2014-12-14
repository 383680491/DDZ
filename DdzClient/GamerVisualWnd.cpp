//
// File: GamerVisualWnd.cpp
//
//  ��ʾ��Ϸ�������ͼ
//
#include "stdafx.h"
#include "GamerVisualWnd.h"

GamerVisualWnd::GamerVisualWnd(void)
{
    m_PicVisual = NULL;
    m_nUserId = INVALID_USER_ID;

    m_hWnd = NULL;
    m_hWndParent = NULL;
    m_hInstance = NULL;

    m_hBkBitmap = NULL;
    m_bReloadBkBitmap = FALSE;
}

GamerVisualWnd::~GamerVisualWnd(void)
{
}

ATOM GamerVisualWnd::GamerVisualWndRegister(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpszClassName  = GAMER_VISUAL_WND_CLASS_NAME;
    wcex.lpfnWndProc    = GamerVisualWndProc;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hInstance      = hInstance;
    wcex.hbrBackground  = NULL;//(HBRUSH)(COLOR_WINDOW + 1);

    return RegisterClassEx(&wcex);
}

LRESULT CALLBACK GamerVisualWnd::GamerVisualWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    LPCREATESTRUCT lpcs;
    GamerVisualWnd* lpWnd = (GamerVisualWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (nMsg) {
        case WM_CREATE:
            lpcs = (LPCREATESTRUCT)lParam;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpcs->lpCreateParams);
            break;

        case WM_DESTROY:
            //lpWnd->m_PicVisual->OnDestroy();
            delete lpWnd->m_PicVisual;

            if (lpWnd->m_hBkBitmap != NULL) {
                DeleteObject(lpWnd->m_hBkBitmap);
                lpWnd->m_hBkBitmap = NULL;
            }

            SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
            break;

        case WM_PAINT:
            hdc =BeginPaint(hWnd, &ps);
            lpWnd->OnPaint(hdc);
            EndPaint(hWnd, &ps);
            break;

        case WM_SIZE:
            // �����ڳߴ��λ�÷����仯����Ҫ���»�ȡ��������������ĸ����ڱ���
            lpWnd->m_bReloadBkBitmap = TRUE;
            break;

        case WM_LBUTTONDBLCLK:
            SendMessage(lpWnd->m_hWndParent, WM_LBUTTONDBLCLK, 0, 0);
            break;

        case WM_LBUTTONDOWN:
            break;

        case WM_RBUTTONDOWN:
            lpWnd->OnRButtonDown(wParam, lParam);
            break;

        default:
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
    }

    return 0;
}

void GamerVisualWnd::OnPaint(HDC hdc)
{
#ifdef PARENT_PAINT_CHILD
    //
    // �������Ѿ������걾�����������ݣ�ʣ�µ�GIFͼ�����Լ�����PictureEx��ɻ���
    //
    if (m_nUserId != INVALID_USER_ID) {
        m_PicVisual->OnPaint();
    } else { // ������û����ң�������뿪������Ʊ���ĸ����ڱ���ͼ
        RECT rect;
        GetClientRect(m_hWnd, &rect);
        HDC memdc = CreateCompatibleDC(hdc);
        HBITMAP hbmpOld = (HBITMAP)SelectObject(memdc, m_hBkBitmap);
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, memdc, 0, 0, SRCCOPY);
        SelectObject(memdc, hbmpOld);
        DeleteDC(memdc);
    }
#else

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    if (m_bReloadBkBitmap == TRUE) {
        m_bReloadBkBitmap = FALSE;

        SaveParentBackground();
        m_PicVisual->SetBkBitmap(m_hBkBitmap);
    }

    // ������û����ң�������뿪���򻭱���ĸ����ڱ���ͼ
    if (m_nUserId == INVALID_USER_ID) {
        HDC memdc = CreateCompatibleDC(hdc);
        HBITMAP hbmpOld = (HBITMAP)SelectObject(memdc, m_hBkBitmap);
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, memdc, 0, 0, SRCCOPY);
        SelectObject(memdc, hbmpOld);
        DeleteDC(memdc);
        return;
    }

    // PictureEx ����GIFͼƬ
    m_PicVisual->OnPaint();

    // ��������ǳ�
    LPCTSTR lpszName = GetUserNameStr(m_nUserId);

    HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HFONT oldfont = (HFONT)SelectObject(hdc, hfont);
    int mode = SetBkMode(hdc, TRANSPARENT);
    COLORREF oldtextclr = SetTextColor(hdc, GVW_TEXT_CLR);

    DrawText(hdc, lpszName, _tcslen(lpszName), &rect, DT_BOTTOM | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    SetTextColor(hdc, oldtextclr);
    SetBkMode(hdc, mode);
    SelectObject(hdc, oldfont);
#endif
}

#ifdef PARENT_PAINT_CHILD
void GamerVisualWnd::ParentPaintChild(HDC parentDC)
{
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    if (m_bReloadBkBitmap == TRUE) {
        m_bReloadBkBitmap = FALSE;

        SaveParentBackground(parentDC);
        m_PicVisual->SetBkBitmap(m_hBkBitmap);
    }

    // ������û�����
    if (m_nUserId == INVALID_USER_ID) {
        return;
    }

    // ת��Ϊ����������ϵ
    CommonUtil::ClientRectToScreen(m_hWnd, &rect);
    CommonUtil::ScreenRectToClient(m_hWndParent, &rect);

    // CPictureEx ����GIFͼƬ
    m_PicVisual->OnPaint(parentDC, &rect);

    // ��������ǳ�
    LPCTSTR lpszName = GetUserNameStr(m_nUserId);

    HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HFONT oldfont = (HFONT)SelectObject(parentDC, hfont);
    int mode = SetBkMode(parentDC, TRANSPARENT);
    COLORREF oldtextclr = SetTextColor(parentDC, GVW_TEXT_CLR);

    DrawText(parentDC, lpszName, _tcslen(lpszName), &rect, DT_BOTTOM | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    SetTextColor(parentDC, oldtextclr);
    SetBkMode(parentDC, mode);
    SelectObject(parentDC, oldfont);
}
#endif

HWND GamerVisualWnd::Create(int x, int y, int cx, int cy, HWND hWndParent, HINSTANCE hInstance)
{
    m_hWndParent = hWndParent;
    m_hInstance = hInstance;
    m_hWnd = CreateWindowEx(0,
        GAMER_VISUAL_WND_CLASS_NAME,
        _T(""),
        WS_CHILD | WS_VISIBLE,
        x, y, cx, cy,
        hWndParent,
        NULL,
        hInstance,
        this);
    assert(m_hWnd != NULL);

    m_PicVisual = new CPictureEx(m_hWnd);
    assert(m_PicVisual != NULL);

    return m_hWnd;
}

// �����ڵĳߴ��Ѿ��ı䣬���ñ�־��ָʾ���ƴ���ʱ����Ҫ���»�ȡ�����ڱ���
void GamerVisualWnd::ParentWndSizeChanged(void)
{
    m_bReloadBkBitmap = TRUE;
}

// ���游���ڱ����������ڳߴ�仯ʱ��Ҫ���±��档
void GamerVisualWnd::SaveParentBackground(void)
{
    // ȡ������DC
    HDC parentDC = GetDC(m_hWndParent);
    assert(parentDC != NULL);
    
    SaveParentBackground(parentDC);

    // �ͷŸ�����DC
    ReleaseDC(m_hWndParent, parentDC);
}

// ���游���ڱ���
void GamerVisualWnd::SaveParentBackground(HDC parentDC)
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

void GamerVisualWnd::SetGamerId(int nUserId)
{
    m_nUserId = nUserId;

    if (nUserId == INVALID_USER_ID) { // ����û�����ͼ
        m_PicVisual->UnLoad();

        InvalidateRect(m_hWnd, NULL, FALSE);
    } else {
        //
        // Ŀǰ����Դ�н�׼����Ů������GIF����ͼ
        //
        int remaider = nUserId % 3;
        PLAYER_GENDER gender = GetUserGender(nUserId);
        HMODULE hModule = LoadLibrary(RES_IMAGE_DLL_NAME);

        if (hModule == NULL) {
            return;
        }

        if (gender == MALE) {
            switch (remaider) {
            case 0:
                m_PicVisual->Load(hModule, MAKEINTRESOURCE(IDR_GAMER_VISUAL_M0), _T("GIF"));
                break;
            case 1:
                m_PicVisual->Load(hModule, MAKEINTRESOURCE(IDR_GAMER_VISUAL_M1), _T("GIF"));
                break;
            case 2:
                m_PicVisual->Load(hModule, MAKEINTRESOURCE(IDR_GAMER_VISUAL_M2), _T("GIF"));
                break;
            }
        } else {
            switch (remaider) {
            case 0:
                m_PicVisual->Load(hModule, MAKEINTRESOURCE(IDR_GAMER_VISUAL_F0), _T("GIF"));
                break;
            case 1:
                m_PicVisual->Load(hModule, MAKEINTRESOURCE(IDR_GAMER_VISUAL_F1), _T("GIF"));
                break;
            case 2:
                m_PicVisual->Load(hModule, MAKEINTRESOURCE(IDR_GAMER_VISUAL_F2), _T("GIF"));
                break;
            }
        }

        FreeLibrary(hModule);

        m_PicVisual->Draw();
    }
}

void GamerVisualWnd::SetWindowRect(int x, int y, int cx, int cy)
{
    MoveWindow(m_hWnd, x, y, cx, cy, TRUE);

    if (m_PicVisual != NULL) {
        RECT rect;
        SetRect(&rect, 0, 0,
            GVW_USER_VISUAL_WIDTH < cx ? GVW_USER_VISUAL_WIDTH : cx,
            GVW_USER_VISUAL_HEIGHT < cy ? GVW_USER_VISUAL_HEIGHT : cy);
        m_PicVisual->SetPaintRect(&rect);
    }
}

void GamerVisualWnd::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    POINT pt;
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    //POINTSTOPOINT(pt, MAKEPOINTS(lParam));

    ClientToScreen(m_hWnd, &pt);
    
    if (!IS_INVALID_USER_ID(m_nUserId)) {
        HMENU hMenu = CreatePopupMenu();
        if (hMenu != NULL) {
            TCHAR szInfo[80] = { 0 };

            _stprintf_s(szInfo, sizeof(szInfo) / sizeof(szInfo[0]),
                _T("���: [ %s ]"), GetUserNameStr(m_nUserId));
            AppendMenu(hMenu, MF_STRING | MF_DISABLED, 1100, (LPCTSTR)szInfo);
            AppendMenu(hMenu, MF_SEPARATOR, 0, 0);

             _stprintf_s(szInfo, sizeof(szInfo) / sizeof(szInfo[0]),
                _T("����: %s\t����: %d\n"), GetUserLevelStr(m_nUserId), GetUserScore(m_nUserId));
             AppendMenu(hMenu, MF_STRING | MF_DISABLED, 1100, (LPCTSTR)szInfo);

            _stprintf_s(szInfo, sizeof(szInfo) / sizeof(szInfo[0]),
                _T("ʤ: %d  ��: %d\t����: %d\n"),
                GetUserWinGames(m_nUserId),
                GetUserTotalGames(m_nUserId) - GetUserWinGames(m_nUserId),
                GetUserTotalGames(m_nUserId));
            AppendMenu(hMenu, MF_STRING | MF_DISABLED, 1100, (LPCTSTR)szInfo);

            int nWinRate = 0;
            int nRunawayRate = 0;
            if (GetUserTotalGames(m_nUserId) != 0) {
                nWinRate = (int)((double)(GetUserWinGames(m_nUserId)) / (double)(GetUserTotalGames(m_nUserId)) * 100);
                nRunawayRate = (int)((double)(GetUserRunawayTimes(m_nUserId)) / (double)(GetUserTotalGames(m_nUserId)) * 100);
            }
            _stprintf_s(szInfo, sizeof(szInfo) / sizeof(szInfo[0]),
                _T("ʤ��: %d%%\t����: %d%%\n"), nWinRate, nRunawayRate);
            AppendMenu(hMenu, MF_STRING | MF_DISABLED, 1100, (LPCTSTR)szInfo);

            AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
            AppendMenu(hMenu, MF_STRING | MF_DISABLED, 1101, _T("�����뿪"));
            AppendMenu(hMenu, MF_STRING | MF_DISABLED, 1102, _T("��Ϊ����"));
            AppendMenu(hMenu, MF_SEPARATOR, 0, 0);

            HMENU hMenuPop = CreatePopupMenu();
            if (hMenuPop != NULL) {
                AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hMenuPop, _T("MORE..."));

                AppendMenu(hMenuPop, MF_STRING, 1103, _T("send E-mail"));
                AppendMenu(hMenuPop, MF_STRING, 1104, _T("send files"));
                AppendMenu(hMenuPop, MF_STRING, 1105, _T("voice chat"));
                AppendMenu(hMenuPop, MF_STRING, 1106, _T("video chat"));

                TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, NULL);

                DestroyMenu(hMenuPop);
            }

            DestroyMenu(hMenu);
        }
    }
}