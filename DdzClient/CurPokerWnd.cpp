//
// CurPokerWnd.cpp
//
//  ʹ��ע�⣺
//      1. �˿�����Դλͼ�����������У������Ƴߴ籣�浽m_CARDWIDTH��m_CARDHEIGHT
//
//          ��Ƭ��A  2  3  4  5  6  7  8  9  0  J  Q  K
//          ÷����A  2  3  4  5  6  7  8  9  0  J  Q  K
//          ���ң�A  2  3  4  5  6  7  8  9  0  J  Q  K
//          ���ң�A  2  3  4  5  6  7  8  9  0  J  Q  K
//          ���ƣ�С �� ��
//
//      2. ʹ�ñ��˿˴�������Ҫ�ȵ����ຯ�� CurPokerWndRegister() ���д�����ע�ᣬ
//         ��Σ�������Ӧ�����Ե��� Create() ���д������ڣ����֮�󣬼���ʹ������
//         ���ýӿڣ��������˿������ݣ���������ʾ�˿˻���ֻ��ʾ�˿˱��棨��ֹ�Թۣ���
//
//      3. ��������Ҫ���� WM_SIZE ��Ϣ�����У������˿��ƴ��ڵĺ���
//         ParentWndSizeChanged()����ȷ�������ڵı�������ȷ���¡�
//
//  �ر�˵����
//      ���ǵ���������Ϸ�ͻ���������Ʊ���ͼʱ��������˸���ܶര��ʹ��NULL_BRUSH��
//      ���䴰����ע�ắ�������������ػ�ʱ��������������ͼ������ҳ��ƺ�
//      CurPokerWnd ���ػ棬����ƵĲ������Ȼ����䴰����������ĸ����ڱ�������
//      ����ʣ���˿���λͼ�����Ƹ����ڱ���ʱ����ֱ��ȡ�����ڱ��������ڸ�����
//      û�в����䱳�������ܵõ�����Ч������������ǣ��������ڵ�һ�λ���ʱ������
//      �������ڸ����ڶ�Ӧ����ı���ͼ��ÿ�α�������Ҫ����ʱ���Ȼ��Ƹñ���ı���
//      ͼ���ٻ��Ʊ����ڵ����ݡ���ˣ��������ڳߴ緢���仯ʱ����������Ҫ֪ͨ�����ڣ�
//      ������������Ҫ���»�ȡ���������Ӧ����������ı���ͼ��
//      
#include "stdafx.h"
#include "CurPokerWnd.h"


// ���캯��
CurPokerWnd::CurPokerWnd(void)
{
    for (int i = 0; i < PLAYER_MAX_CARDS_NUM; i++) {
        m_nCurPokerCards[i] = POKER_BACK_INDEX;
        m_bPokerCardSelected[i] = FALSE;
    }

    m_nCurPokerCardsNum = 0;

    m_hInstance = NULL;
    m_hWndParent = NULL;

    m_hWnd = NULL;
    m_hPokerBMP = NULL;
    m_hBkBitmap = NULL;
    m_bReloadBkBitmap = FALSE;

    m_bLButtonDown = FALSE;
    m_ptLButtonDown.x = INVALID_COORD;
    m_ptLButtonDown.y = INVALID_COORD;

    m_bLBtnDownAndMouseMove = FALSE;
    m_ptMouseMove.x = INVALID_COORD;
    m_ptMouseMove.y = INVALID_COORD;

    m_bShowPoker = TRUE;
    m_bHorizontal = TRUE;

    m_bIsLord = FALSE;
    m_bAllowPokerSelect = TRUE;
}

// ��������
CurPokerWnd::~CurPokerWnd(void)
{
}

// ע�ᴰ����
ATOM CurPokerWnd::CurPokerWndRegister(HINSTANCE hInstance)
{
    WNDCLASSEX  wcex;

    wcex.cbSize             = sizeof(WNDCLASSEX);
    wcex.lpszClassName      = CUR_POKER_WND_CLASS_NAME;
    wcex.lpfnWndProc	    = CurPokerWndProc;
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
CurPokerWnd::CurPokerWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    LPCREATESTRUCT lpcs;
    CurPokerWnd* lpWnd = (CurPokerWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

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

        case WM_LBUTTONDOWN:
            lpWnd->OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_LBUTTONUP:
            lpWnd->OnLButtonUp(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MOUSEMOVE:
            lpWnd->OnMouseMove((short)(LOWORD(lParam)), (short)(HIWORD(lParam)));
            break;

        default:
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
    }

    return 0;
}

// �����˿��ƴ���
//
// �������ڴ����˿˴���ʱ����������ͻ����Σ����Ǵ�����������ĵ����ꡣ
// ���������������ĵ㣬����һ������������˿��Ƶľ��δ��ڡ�
//
HWND CurPokerWnd::Create(DWORD dwStyle, int xCenter, int yCenter,
                         HWND hWndParent, UINT nID, HINSTANCE hInstance,
                         HBITMAP hPokerBMP, BOOL bShowPoker, BOOL bHorizontal)
{
    if ((hWndParent == NULL) || (hInstance == NULL) || (hPokerBMP == NULL)) {
        return NULL;
    }

    int x, y;
    int nMaxWidth, nMaxHeight;

    // ���ô��ھ���Ϊ��������˿��Ƶľ���
    if (bHorizontal == TRUE) {
        nMaxHeight = m_CARDHEIGHT + m_POPSPACE;
        nMaxWidth = (PLAYER_MAX_CARDS_NUM - 1) * m_HSPACE + m_CARDWIDTH;
    } else {
        nMaxHeight = (PLAYER_MAX_CARDS_NUM - 1) * m_VSPACE + m_CARDHEIGHT;
        nMaxWidth = m_CARDWIDTH;
    }

    x = xCenter - nMaxWidth / 2;
    y = yCenter - nMaxHeight / 2;

    m_hWnd = CreateWindowEx(0,
        CUR_POKER_WND_CLASS_NAME,
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

    m_nID = nID;
    m_hWndParent = hWndParent;
    m_hInstance = hInstance;
    m_hPokerBMP = hPokerBMP;
    m_bShowPoker = bShowPoker;
    m_bHorizontal = bHorizontal;

    return m_hWnd;
}

//// �������ڵ�λ����ߴ硣����������ʱָ���ľ��β�һ����������ľ��γߴ磩
//void CurPokerWnd::AdjustWindowRect(void)
//{
//    RECT rect;
//    GetClientRect(m_hWnd, &rect);
//
//    int x = rect.left;
//    int y = rect.top;
//    int cx = rect.right - rect.left;
//    int cy = rect.bottom - rect.top;
//
//    int nMinHeight;
//    int nMinWidth;
//
//    if (m_bHorizontal == TRUE) {
//        // ��ֱ���н����ھ��ε����������˿˵���С����
//        nMinHeight = m_CARDHEIGHT + m_POPSPACE;
//        nMinWidth = (m_nCurPokerCardsNum - 1) * m_HSPACE + m_CARDWIDTH;
//    } else {
//        // ˮƽ���н����ھ��ε����������˿˵���С����
//        nMinWidth = m_CARDWIDTH;
//        nMinHeight = (m_nCurPokerCardsNum - 1) * m_VSPACE + m_CARDHEIGHT;
//    }
//
//    x += (cx - nMinWidth) / 2;
//    y += (cy - nMinHeight) / 2;
//    cx = nMinWidth;
//    cy = nMinHeight;
//
//    POINT pt = { x, y };
//    ClientToScreen(m_hWnd, &pt);
//    ScreenToClient(m_hWndParent, &pt);
//    SetWindowPos(m_hWnd, NULL, pt.x, pt.y, cx, cy, SWP_SHOWWINDOW);
//}

// �����˿����ݸ���ǰ�˿��ƴ���
BOOL CurPokerWnd::SetCards(int poker[], int num)
{
    // ����NUM����Ϊ0������ձ����ڵ�����
    if ((num == 0) && (m_nCurPokerCardsNum == 0)) {
        return TRUE;
    }

    m_nCurPokerCardsNum = num;
    for (int i = 0; i < num; i++) {
        m_nCurPokerCards[i] = poker[i];
    }

    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        m_bPokerCardSelected[i] = FALSE;
    }

#ifdef PARENT_PAINT_CHILD
    CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
    InvalidateRect(m_hWnd, NULL, FALSE);
#endif

    return TRUE;
}

// ��ȡ��ǰ�˿��ơ����� poker[] �����㹻���Ƽ�ʹ��ֵ PLAYER_MAX_CARDS_NUM
BOOL CurPokerWnd::GetCards(int poker[], int* num)
{
    assert((poker != NULL) && (num != NULL));

    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        poker[i] = m_nCurPokerCards[i];
    }

    *num = m_nCurPokerCardsNum;
    return TRUE;
}

// ��ȡѡ����˿�����������
BOOL CurPokerWnd::GetSelectedCards(int poker[], int num, int* selnum)
{
    UNREFERENCED_PARAMETER(num);
    assert(num >= m_nCurPokerCardsNum);

    int count = 0;

    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        if (m_bPokerCardSelected[i] == TRUE) {
            poker[count++] = m_nCurPokerCards[i];
        }
    }

    *selnum = count;
    return TRUE;
}

// ɾ��ѡ����˿��ƣ����ƣ�
BOOL CurPokerWnd::RemoveSelectedCards(void)
{
    int nRemain = 0;
    int pokers[PLAYER_MAX_CARDS_NUM] = { 0 };

    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        if (m_bPokerCardSelected[i] == FALSE) {
            pokers[nRemain++] = m_nCurPokerCards[i];
        }
    }

    if (nRemain == m_nCurPokerCardsNum) { // no selected pokers
        return TRUE;
    }

    for (int i = 0; i < nRemain; i++) {
        m_nCurPokerCards[i] = pokers[i];
    }

    m_nCurPokerCardsNum = nRemain;

    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        m_bPokerCardSelected[i] = FALSE;
    }

#ifdef PARENT_PAINT_CHILD
    CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
    InvalidateRect(m_hWnd, NULL, FALSE);
#endif

    return TRUE;
}

// ������ָʾѡ����Щ�ƣ��ýӿ���Ҫ������ʾ���ܷ��ص�������ע��ú���������������
BOOL CurPokerWnd::SelectCardsFromIndex(int index[], int num)
{
    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        m_bPokerCardSelected[i] = FALSE;
    }

    for (int i = 0; i < num; i++) {
        assert((index[i] >= 0) && (index[i] < m_nCurPokerCardsNum));

        m_bPokerCardSelected[index[i]] = TRUE;
    }

#ifdef PARENT_PAINT_CHILD
    CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
    InvalidateRect(m_hWnd, NULL, FALSE);
#endif

    return TRUE;
}

// ɾ��ָ�����˿������У��˿���Ϊ����ģ�
BOOL CurPokerWnd::RemoveCards(int poker[], int num)
{
    int nRemCount = 0;
    int nRemCards[PLAYER_MAX_CARDS_NUM] = { 0 };
    BOOL bRemoved[PLAYER_MAX_CARDS_NUM] = { 0 };

    if (num <= 0) {
        return TRUE;
    }

    // �Ƚ�Ҫɾ�������ñ�־
    int j = 0;
    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        if (m_nCurPokerCards[i] == poker[j]) {
            bRemoved[i] = TRUE;
            if (++j >= num) {
                break;
            }
        }
    }

    // ��ûɾ�����ƿ�����ȥ
    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        if (bRemoved[i] != TRUE) {
            nRemCards[nRemCount++] = m_nCurPokerCards[i];
        }
    }

    // ��������ȥ�ģ�û��ɾ���ģ����ٿ�������
    for (int i = 0; i < nRemCount; i++) {
        m_nCurPokerCards[i] = nRemCards[i];
    }

    m_nCurPokerCardsNum = nRemCount;

    // ���ƺ󣬸�λ����ѡ���������
    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        m_bPokerCardSelected[i] = FALSE;
    }

#ifdef PARENT_PAINT_CHILD
    CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
    InvalidateRect(m_hWnd, NULL, FALSE);
#endif

    return TRUE;
}

// �����˿��Ƶ���ǰ�˿˴��ڣ��е���֮�󣬽����Ʋ��������ǰ���У�
BOOL CurPokerWnd::AddCards(int poker[], int num)
{
    assert(m_nCurPokerCardsNum + num <= PLAYER_MAX_CARDS_NUM);

    for (int i = 0; i < num; i++) {
        m_nCurPokerCards[m_nCurPokerCardsNum + i] = poker[i];
    }

    m_nCurPokerCardsNum += num;

    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        m_bPokerCardSelected[i] = FALSE;
    }

    CommonUtil::QuickSort(m_nCurPokerCards, 0, m_nCurPokerCardsNum - 1);

#ifdef PARENT_PAINT_CHILD
    CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
    InvalidateRect(m_hWnd, NULL, FALSE);
#endif

    return TRUE;
}

// ���游���ڱ����������ڳߴ�仯ʱ��Ҫ���±��档
void CurPokerWnd::SaveParentBackground(void)
{
    // ȡ������DC
    HDC parentDC = GetDC(m_hWndParent);
    assert(parentDC != NULL);

    SaveParentBackground(parentDC);

    // �ͷŸ�����DC
    ReleaseDC(m_hWndParent, parentDC);
}

// ���游���ڱ���
void CurPokerWnd::SaveParentBackground(HDC parentDC)
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

// ��ͼ
void CurPokerWnd::OnPaint(HDC hdc)
{
#ifdef PARENT_PAINT_CHILD
    UNREFERENCED_PARAMETER(hdc);
    // Parent window will paint me. Do nothing...
#else
    if (m_bReloadBkBitmap == TRUE) {
        SaveParentBackground();
        m_bReloadBkBitmap = FALSE;
    }

    if (m_bHorizontal == TRUE) {
        PaintHorizontally(hdc);
    } else {
        PaintVertically(hdc);
    }
#endif
}

// ����ˮƽ���е��˿�����
void CurPokerWnd::PaintHorizontally(HDC hdc)
{
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    int cx = rect.right - rect.left;
    int cy = rect.bottom - rect.top;

    int nPokersLen = m_HSPACE * (m_nCurPokerCardsNum - 1) + m_CARDWIDTH;

    // ����ͻ����ڵ�����ƫ�ƣ�ȷ���ڿͻ����м���ʾ
    int nStartY = m_POPSPACE;
    int nStartX = (cx - nPokersLen) / 2;
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

    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        if (m_bShowPoker == TRUE) {
            nStartY = m_bPokerCardSelected[i] ? 0 : m_POPSPACE;
            nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(m_nCurPokerCards[i]);
        } else {
            nStartY = m_POPSPACE;
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

    BitBlt(hdc, 0, 0, cx, cy, memDC, 0, 0, SRCCOPY);

    // ��ԭ���ͷ���Դ
    SelectObject(tempDC, hOldTempBmp);
    SelectObject(pokerDC, hOldPokerBmp);
    SelectObject(memDC, hOldMemBmp);

    DeleteObject(hMemBmp);

    DeleteDC(tempDC);
    DeleteDC(pokerDC);
    DeleteDC(memDC);
}

// ������ֱ���е��˿�����
void CurPokerWnd::PaintVertically(HDC hdc)
{
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    int cx = rect.right - rect.left;
    int cy = rect.bottom - rect.top;

    // ����ͻ����ڵ�����ƫ�ƣ�ȷ���ڿͻ����м���ʾ
    int nStartX = (cx - m_CARDWIDTH) / 2;
    if (nStartX < 0) { nStartX = 0; }

    int nPokersHt = m_VSPACE * (m_nCurPokerCardsNum - 1) + m_CARDHEIGHT;
    int nStartY = (cy - nPokersHt) / 2;
    if (nStartY < 0) { nStartY = 0; }

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

    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        if (m_bShowPoker == TRUE) {
            nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(m_nCurPokerCards[i]);
        } else {
            nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(POKER_BACK_INDEX);
        }

        BitBlt(memDC,
            nStartX,
            nStartY + i * m_VSPACE,
            m_CARDWIDTH,
            m_CARDHEIGHT,
            pokerDC,
            nCardBmpIndex % 13 * m_CARDWIDTH,
            nCardBmpIndex / 13 * m_CARDHEIGHT,
            SRCCOPY);
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
}


// ���
void CurPokerWnd::ClickCard(int x, int y)
{
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    // �����ε�������ǰ�����˿������ڵľ���
    int nClientWidth = rect.right - rect.left;
    int nPokerCardsLen = (m_nCurPokerCardsNum - 1) * m_HSPACE + m_CARDWIDTH;

    if (nClientWidth > nPokerCardsLen) {
        rect.left = (nClientWidth - nPokerCardsLen) / 2;
        rect.right = (nClientWidth + nPokerCardsLen - 1) / 2;
    }

    rect.top = 0;

    if (rect.bottom > m_POPSPACE + m_CARDHEIGHT) {
        rect.bottom = m_POPSPACE + m_CARDHEIGHT;
    }

    // ��������˿��ƾ����ϡ��жϵ����������
    POINT pt = {x, y};
    if (PtInRect(&rect, pt)) {
        int xCoord = pt.x - rect.left;
        int yCoord = pt.y;
        int index;

        if (xCoord > (m_nCurPokerCardsNum - 1) * m_HSPACE) {
            // �������ұߣ�����˵�� Z-TopMost �������ơ��ɿ�����ȫ�����ݵ�������
            index = m_nCurPokerCardsNum - 1;
        } else {
            index = xCoord / m_HSPACE;
        }

        if (m_bPokerCardSelected[index] == TRUE) { // �Ѿ��ǵ���״̬
            if ((yCoord >= 0) && (yCoord <= m_CARDHEIGHT)) {
                m_bPokerCardSelected[index] = FALSE;

#ifdef PARENT_PAINT_CHILD
                CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
                InvalidateRect(m_hWnd, NULL, FALSE);
#endif
            } else {
                //
                // ���������֧��˵����ҵ���˵����˿��ƺ�����µ�С����
                // ��Ϊ�˿����ǲ�����õģ����ԣ���ǰ������ĵ������λ
                // ������δ��������У�Ӧ������ǰ�������Ž����жϡ�
                //
                int num;
                int remaider = xCoord % m_HSPACE;
                int multiple = m_CARDWIDTH / m_HSPACE;

                // ������� num ��
                if (remaider + multiple * m_HSPACE > m_CARDWIDTH) {
                    num = multiple;
                } else {
                    num = multiple + 1;
                }

                // ��ǰ����num���ƣ��ҵ���һ�ż��ɡ�ע�������±��underflow
                for (int i = 1; i <= num; i++) {
                    int idx = index - i;

                    if (idx < 0) { return; }

                    if (m_bPokerCardSelected[idx] == FALSE) {
                        //
                        // �����ҵ��ĵ�һ�ţ���һ��ȷ�ϵ���Ƿ����˿������棬
                        // ������ǣ������������ң�����ǣ�����ѡ������½��档
                        //
                        if (idx * m_HSPACE + m_CARDWIDTH > xCoord) {
                            m_bPokerCardSelected[idx] = TRUE;

#ifdef PARENT_PAINT_CHILD
                            CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
                            InvalidateRect(m_hWnd, NULL, FALSE);
#endif
                        }

                        return;
                    }
                }
            }
        } else { // �˿��Ʋ��ǵ���״̬
            if ((yCoord >= m_POPSPACE) && (yCoord <= m_POPSPACE + m_CARDHEIGHT)) {
                m_bPokerCardSelected[index] = TRUE;

#ifdef PARENT_PAINT_CHILD
                CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
                InvalidateRect(m_hWnd, NULL, FALSE);
#endif
            } else {
                int num;
                int remaider = xCoord % m_HSPACE;
                int multiple = m_CARDWIDTH / m_HSPACE;

                // ������� num ��
                if (remaider + multiple * m_HSPACE > m_CARDWIDTH) {
                    num = multiple;
                } else {
                    num = multiple + 1;
                }

                // ��ǰ����num���ƣ��ҵ���һ�ż��ɡ�ע�������±��underflow
                for (int i = 1; i <= num; i++) {
                    int idx = index - i;

                    if (idx < 0) { return; }

                    if (m_bPokerCardSelected[idx] == TRUE) {
                        if (idx * m_HSPACE + m_CARDWIDTH > xCoord) {
                            m_bPokerCardSelected[idx] = FALSE;

#ifdef PARENT_PAINT_CHILD
                            CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
                            InvalidateRect(m_hWnd, NULL, FALSE);
#endif
                        }

                        return;
                    }
                }
            }
        }
    }
}

// ����������
void CurPokerWnd::OnLButtonDown(int x, int y)
{
    if (m_bHorizontal == FALSE) { // ֻ��ˮƽ�͵ģ���ǰ��ҵ��ƣ����Ա�ѡ��
        return;
    }

    if (m_nCurPokerCardsNum <= 0) {
        return;
    }

    if (m_bAllowPokerSelect == FALSE) { // �Ƿ�����ѡ���˿��ƣ���Ҫ�����Թ����ӽ�
        return;
    }

    POINT pt;
    pt.x = x;
    pt.y = y;

    RECT rect;
    GetCardRangeSelectRect(&rect);

    m_ptLButtonDown.x = x;
    m_ptLButtonDown.y = y;

    m_bLButtonDown = TRUE;
    SetCapture(m_hWnd);
}

// ����������
void CurPokerWnd::OnLButtonUp(int x, int y)
{
    RECT rect;
    RECT rectRangeSel;

    if (m_bLButtonDown == TRUE) {
        m_bLButtonDown = FALSE;
        ReleaseCapture();

        if (m_bLBtnDownAndMouseMove == TRUE) {
            m_bLBtnDownAndMouseMove = FALSE;

            HDC hdc = GetDC(m_hWnd);
            HPEN hpen = CreatePen(PS_SOLID|PS_INSIDEFRAME, SEL_CARDS_FRAME_WIDTH, SEL_CARDS_FRAME_CLR);
            HBRUSH hbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HPEN oldpen = (HPEN)SelectObject(hdc, hpen);
            HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, hbrush);
            int mode = SetROP2(hdc, R2_XORPEN);

            GetCardRangeSelectRect(&rectRangeSel);

            rect.left = max(0, min(m_ptMouseMove.x, m_ptLButtonDown.x));
            rect.right = max(m_ptMouseMove.x, m_ptLButtonDown.x);
            rect.top = max(0, min(m_ptMouseMove.y, m_ptLButtonDown.y));
            rect.bottom = max(m_ptMouseMove.y, m_ptLButtonDown.y);
            IntersectRect(&rect, &rect, &rectRangeSel);
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

            SetROP2(hdc, mode);
            SelectObject(hdc, oldbrush);
            SelectObject(hdc, oldpen);

            DeleteObject(hpen);
            ReleaseDC(m_hWnd, hdc);

            SelectCardRange(min(m_ptMouseMove.x, m_ptLButtonDown.x),
                max(m_ptMouseMove.x, m_ptLButtonDown.x));

            m_ptMouseMove.x = INVALID_COORD;
            m_ptMouseMove.y = INVALID_COORD;
        } else {
            if (m_ptLButtonDown.x == x) {
                if (m_ptLButtonDown.y == y) {
                    ClickCard(x, y); // �������ѡ��һ����
                }
            }
        }
    }
}

// ѡ������˿���
void CurPokerWnd::SelectCardRange(int xStart, int xEnd)
{
    RECT rectRangeSel;
    GetCardRangeSelectRect(&rectRangeSel);

    // xStart�� xEnd ����λ�� rectRangeSel ������
    int xCoordStart = xStart - rectRangeSel.left;
    int xCoordEnd = xEnd - rectRangeSel.left;

    int iStart = xCoordStart / m_HSPACE;
    int iEnd = xCoordEnd / m_HSPACE;

    if (iStart < 0) { iStart = 0; }
    if (iStart > m_nCurPokerCardsNum - 1) { iStart = m_nCurPokerCardsNum - 1; }
    if (iEnd > m_nCurPokerCardsNum - 1) { iEnd = m_nCurPokerCardsNum - 1; }

    for (int i = iStart; i <= iEnd; i++) {
        m_bPokerCardSelected[i] = !m_bPokerCardSelected[i];
    }

#ifdef PARENT_PAINT_CHILD
    CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
    InvalidateRect(m_hWnd, NULL, FALSE);
#endif
}

// ��ȡ��������˿���ѡ��ľ��Σ���������ˮƽ�͵��˿��ƴ��ڣ�
void CurPokerWnd::GetCardRangeSelectRect(LPRECT lpRect)
{
    GetClientRect(m_hWnd, lpRect);

    int nClientWidth = lpRect->right - lpRect->left;
    int nPokerCardsLen = (m_nCurPokerCardsNum - 1) * m_HSPACE + m_CARDWIDTH;

    if (nClientWidth > nPokerCardsLen) {
        lpRect->left = (nClientWidth - nPokerCardsLen) / 2;
        lpRect->right = (nClientWidth + nPokerCardsLen - 1) / 2;
    }

    lpRect->top += m_POPSPACE;
    lpRect->bottom -= m_POPSPACE;
}

// �ƶ���꣨��������˿����ϰ��²��ƶ�ʱ��XOR��ʽ��һ���Σ���ʾ���ѡ���˿�����
void CurPokerWnd::OnMouseMove(int x, int y)
{
    RECT rect;
    RECT rectRangeSel;

    if (m_bLButtonDown != TRUE) {
        return;
    }

    GetCardRangeSelectRect(&rectRangeSel);

    if (PtInRect(&rectRangeSel, m_ptLButtonDown)) {
        m_bLBtnDownAndMouseMove = TRUE;

        HDC hdc = GetDC(m_hWnd);
        HPEN hpen = CreatePen(PS_SOLID|PS_INSIDEFRAME, SEL_CARDS_FRAME_WIDTH, SEL_CARDS_FRAME_CLR);
        HBRUSH hbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HPEN oldpen = (HPEN)SelectObject(hdc, hpen);
        HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, hbrush);
        int mode = SetROP2(hdc, R2_XORPEN);

        if (m_ptMouseMove.x != INVALID_COORD) {
            rect.left = max(0, min(m_ptMouseMove.x, m_ptLButtonDown.x));
            rect.right = max(m_ptMouseMove.x, m_ptLButtonDown.x);
            rect.top = max(0, min(m_ptMouseMove.y, m_ptLButtonDown.y));
            rect.bottom = max(m_ptMouseMove.y, m_ptLButtonDown.y);
            IntersectRect(&rect, &rect, &rectRangeSel);
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
        }

        m_ptMouseMove.x = max(0, x);
        m_ptMouseMove.y = max(0, y);

        rect.left = max(0, min(m_ptMouseMove.x, m_ptLButtonDown.x));
        rect.right = max(m_ptMouseMove.x, m_ptLButtonDown.x);
        rect.top = max(0, min(m_ptMouseMove.y, m_ptLButtonDown.y));
        rect.bottom = max(m_ptMouseMove.y, m_ptLButtonDown.y);
        IntersectRect(&rect, &rect, &rectRangeSel);
        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

        SetROP2(hdc, mode);
        SelectObject(hdc, oldbrush);
        SelectObject(hdc, oldpen);

        DeleteObject(hpen);

        ReleaseDC(m_hWnd, hdc);
    }
}

// �����ڵĳߴ��Ѿ��ı䣬���ñ�־��ָʾ���ƴ���ʱ����Ҫ���»�ȡ�����ڱ���
void CurPokerWnd::ParentWndSizeChanged(void)
{
    m_bReloadBkBitmap = TRUE;
}

// ��λ����ѡ�е��˿���
BOOL CurPokerWnd::UnSelectAllCards(void)
{
    BOOL bNeedPaint = FALSE;

    for (int i = 0; i < m_nCurPokerCardsNum; i++) {
        if (m_bPokerCardSelected[i] == TRUE) {
            m_bPokerCardSelected[i] = FALSE;
            bNeedPaint = TRUE;
        }
    }

    if (bNeedPaint == TRUE) {
#ifdef PARENT_PAINT_CHILD
        CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
        InvalidateRect(m_hWnd, NULL, FALSE);
#endif
    }

    return TRUE;
}

// ��ʾ�˿���
BOOL CurPokerWnd::ShowPoker(BOOL bShow /*= TRUE*/)
{
    if (m_bShowPoker != bShow) {
        m_bShowPoker = bShow;

        if (m_nCurPokerCardsNum > 0) {
#ifdef PARENT_PAINT_CHILD
            CommonUtil::ParentPaintChild(m_hWndParent, m_hWnd, NULL, FALSE);
#else
            InvalidateRect(m_hWnd, NULL, FALSE);
#endif
        }
    }

    return TRUE;
}

// ������ͨ�����ñ����ڵĿͻ��������ĵ��λ�����ƶ������ڡ�����Ϊ����������ϵ
BOOL CurPokerWnd::SetWindowCenterPos(int xCenter, int yCenter)
{
    int x, y;
    int nMaxWidth, nMaxHeight;

    // ���ô��ھ���Ϊ��������˿��Ƶľ���
    if (m_bHorizontal == TRUE) {
        nMaxHeight = m_CARDHEIGHT + m_POPSPACE;
        nMaxWidth = (PLAYER_MAX_CARDS_NUM - 1) * m_HSPACE + m_CARDWIDTH;
    } else {
        nMaxHeight = (PLAYER_MAX_CARDS_NUM - 1) * m_VSPACE + m_CARDHEIGHT;
        nMaxWidth = m_CARDWIDTH;
    }

    x = xCenter - nMaxWidth / 2;
    y = yCenter - nMaxHeight / 2;

    MoveWindow(m_hWnd, x, y, nMaxWidth, nMaxHeight, TRUE);
    return TRUE;
}

#ifdef PARENT_PAINT_CHILD
void CurPokerWnd::ParentPaintChild(HDC parentDC)
{
    if (m_nCurPokerCardsNum <= 0) { return; }

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    POINT ptParentStart; // �ڸ���������ϵ�еĻ��˿��Ƶ���ʼ��

    int cx = rect.right - rect.left;
    int cy = rect.bottom - rect.top;

    // ���˿���λͼѡ���ڴ�DC
    HDC pokerDC = CreateCompatibleDC(parentDC);
    assert(pokerDC != NULL);
    HBITMAP hOldPokerBmp = (HBITMAP)SelectObject(pokerDC, m_hPokerBMP);

    if (m_bHorizontal == TRUE) { // ˮƽ�͵ĵ�ǰ�˿��ƴ���
        int nPokersLen = m_HSPACE * (m_nCurPokerCardsNum - 1) + m_CARDWIDTH;

        // ����ͻ����ڵ�����ƫ�ƣ�ȷ���ڿͻ����м���ʾ
        int nStartY = m_POPSPACE;
        int nStartX = (cx - nPokersLen) / 2;
        if (nStartX < 0) { nStartX = 0; }

        // ������ת��Ϊ����������ϵ
        ptParentStart.x = nStartX;
        ptParentStart.y = nStartY;
        ClientToScreen(m_hWnd, &ptParentStart);
        ScreenToClient(m_hWndParent, &ptParentStart);
        nStartX = ptParentStart.x;
        nStartY = ptParentStart.y;

        // ���˿���
        int nCardBmpIndex;

        for (int i = 0; i < m_nCurPokerCardsNum; i++) {
            if (m_bShowPoker == TRUE) {
                nStartY = m_bPokerCardSelected[i] ? (ptParentStart.y - m_POPSPACE) : ptParentStart.y;

                nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(m_nCurPokerCards[i]);
            } else {
                nStartY = ptParentStart.y;

                if (m_bIsLord == TRUE) {
                    nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(LORD_POKER_BACK_INDEX);
                } else {
                    nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(POKER_BACK_INDEX);
                }
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
    } else { // ��ֱ�͵ĵ�ǰ�˿��ƴ���
        // ����ͻ����ڵ�����ƫ�ƣ�ȷ���ڿͻ����м���ʾ
        int nStartX = (cx - m_CARDWIDTH) / 2;
        if (nStartX < 0) { nStartX = 0; }

        int nPokersHt = m_VSPACE * (m_nCurPokerCardsNum - 1) + m_CARDHEIGHT;
        int nStartY = (cy - nPokersHt) / 2;
        if (nStartY < 0) { nStartY = 0; }

        // ������ת��Ϊ����������ϵ
        ptParentStart.x = nStartX;
        ptParentStart.y = nStartY;
        ClientToScreen(m_hWnd, &ptParentStart);
        ScreenToClient(m_hWndParent, &ptParentStart);
        nStartX = ptParentStart.x;
        nStartY = ptParentStart.y;

        int nCardBmpIndex;

        for (int i = 0; i < m_nCurPokerCardsNum; i++) {
            if (m_bShowPoker == TRUE) {
                nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(m_nCurPokerCards[i]);
            } else {
                if (m_bIsLord == TRUE) {
                    nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(LORD_POKER_BACK_INDEX);
                } else {
                    nCardBmpIndex = CommonUtil::PokerIndexToBmpIndex(POKER_BACK_INDEX);
                }
            }

            BitBlt(parentDC,
                nStartX,
                nStartY + i * m_VSPACE,
                m_CARDWIDTH,
                m_CARDHEIGHT,
                pokerDC,
                nCardBmpIndex % 13 * m_CARDWIDTH,
                nCardBmpIndex / 13 * m_CARDHEIGHT,
                SRCCOPY);
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

// �����Ƿ�����ǰ�˿��ƴ��ڵ��ƿ��Ա�ѡ��Ĭ��Ϊ����ѡ�����������Թ��߲���ѡ���ơ�
BOOL CurPokerWnd::AllowPokerSelection(BOOL bAllow /*= TRUE*/)
{
    if (bAllow == FALSE) {
        UnSelectAllCards();
    }

    m_bAllowPokerSelect = bAllow;
    return TRUE;
}

// ���ø����Ϊ���������ñ����������Զ�ˢ�½��档
BOOL CurPokerWnd::SetLord(BOOL bLord /*= TRUE*/)
{
    m_bIsLord = bLord;
    return TRUE;
}
