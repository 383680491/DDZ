//
// File: ServerListWnd.cpp
//
// Contain server tree list and Add, Edit, Remove button.
//
#include "stdafx.h"
#include "DdzClient.h"
#include "ServerListWnd.h"


ServerListWnd::ServerListWnd(void)
{
    m_bShow = TRUE;
    m_hInstance = NULL;
    m_hWndParent = NULL;
    m_hWnd = NULL;

    m_hServerListTree = NULL;
    m_hButtonAdd = NULL;
    m_hButtonEdit = NULL;
    m_hButtonRemove = NULL;

    ZeroMemory(m_ServerInfo, sizeof(SERVER_INFO) * MAX_SERVER_NUM_SUPPORTED);
    ZeroMemory(m_szConfigFile, sizeof(m_szConfigFile));
}

ServerListWnd::~ServerListWnd(void)
{
}

// ע���������Ϣ��崰����
ATOM ServerListWnd::ServerListWndRegister(HINSTANCE hInstance)
{
    WNDCLASSEX  wcex;

    wcex.cbSize             = sizeof(WNDCLASSEX);
    wcex.lpszClassName      = SERVER_LIST_WND_CLASS_NAME;
    wcex.lpfnWndProc	    = ServerListWndProc;
    wcex.style			    = CS_HREDRAW | CS_VREDRAW;
    wcex.cbClsExtra		    = 0;
    wcex.cbWndExtra		    = 0;
    wcex.hInstance		    = hInstance;
    wcex.hIcon			    = NULL;
    wcex.hCursor		    = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	    = NULL;//(HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName	    = NULL;
    wcex.hIconSm		    = NULL;

    return RegisterClassEx(&wcex);
}

// �����������б�����
HWND ServerListWnd::Create(int x, int y, int cx, int cy, HWND hWndParent, HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    m_hWndParent = hWndParent;

    m_hWnd = CreateWindowEx(0,
        SERVER_LIST_WND_CLASS_NAME,
        _T(""),
        WS_CHILD | WS_VISIBLE,
        x,
        y,
        cx,
        cy,
        hWndParent,
        NULL,
        hInstance,
        this);

    CreateChildWindows();

    return m_hWnd;
}

// �������б��ڴ�����
LRESULT
CALLBACK
ServerListWnd::ServerListWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    LPCREATESTRUCT lpcs;
    ServerListWnd* lpWnd = (ServerListWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (nMsg) {
        case WM_CREATE:
            lpcs = (LPCREATESTRUCT)lParam;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(lpcs->lpCreateParams));
            break;

        case WM_DESTROY:
            SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            lpWnd->OnPaint(hdc);
            EndPaint(hWnd, &ps);
            break;

        case WM_SIZE:
            lpWnd->OnSize(wParam, lParam);
            break;

        case WM_COMMAND:
            lpWnd->OnCommand(wParam, lParam);
            break;

        case WM_NOTIFY:
            lpWnd->OnNotify(wParam, lParam);
            break;

        default:
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
    }

    return 0;
}

// ���ƴ���
void ServerListWnd::OnPaint(HDC hdc)
{
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    int cx = rect.right - rect.left;
    int cy = rect.bottom - rect.top;

    HDC memdc = CreateCompatibleDC(hdc);
    assert(memdc);

    HBITMAP hbmp = CreateCompatibleBitmap(hdc, cx, cy);
    assert(hbmp);

    HBITMAP oldbmp = (HBITMAP)SelectObject(memdc, hbmp);

    HBRUSH hbrush = CreateSolidBrush(SLW_TREE_BG_CLR);
    if (hbrush != NULL) {
        FillRect(memdc, &rect, hbrush);

        HPEN hpen = CreatePen(PS_SOLID, SLW_ROUND_FRAME_WIDTH, SLW_ROUND_FRAME_CLR);
        if (hpen != NULL) {
            HPEN oldpen = (HPEN)SelectObject(memdc, hpen);
            HBRUSH oldbrush = (HBRUSH)SelectObject(memdc, GetStockObject(NULL_BRUSH));

            // ����һ��Բ�Ǳ߿�
            RoundRect(memdc,
                rect.left + SLW_ROUND_FRAME_OFFSET,
                rect.top + SLW_ROUND_FRAME_OFFSET,
                rect.right - SLW_ROUND_FRAME_OFFSET,
                rect.bottom - SLW_ROUND_FRAME_OFFSET,
                SLW_ROUND_FRAME_ANGEL,
                SLW_ROUND_FRAME_ANGEL);
            
            SelectObject(memdc, oldbrush);
            SelectObject(memdc, oldpen);
            DeleteObject(hpen);
        }
        DeleteObject(hbrush);
    }

    BitBlt(hdc, 0, 0, cx, cy, memdc, 0, 0, SRCCOPY);

    SelectObject(memdc, oldbmp);
    DeleteObject(hbmp);
    DeleteDC(memdc);
}

// �����Ӵ��ڿؼ�
void ServerListWnd::CreateChildWindows(void)
{
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    int cx = rect.right - rect.left;
    int cy = rect.bottom - rect.top;

    m_hServerListTree = CreateWindow(WC_TREEVIEW, _T(""), 
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER |
        TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | TVS_LINESATROOT |
        TVS_NOTOOLTIPS | TVS_HASLINES | TVS_FULLROWSELECT,
        SLW_CTRL_OFFSET,
        SLW_CTRL_OFFSET,
        (2 * SLW_CTRL_OFFSET),
        (2 * SLW_CTRL_OFFSET) - (3 * (SLW_BUTTON_HEIGHT + SLW_BUTTON_V_INTERVAL)),
        m_hWnd,
        (HMENU)ID_SERVER_LIST_TREE,
        m_hInstance,
        NULL);

    m_hButtonAdd = CreateWindow(WC_BUTTON, _T("��ӷ�����"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        SLW_CTRL_OFFSET,
        cy - SLW_CTRL_OFFSET - (3 * SLW_BUTTON_HEIGHT) - (2 * SLW_BUTTON_V_INTERVAL),
        cx - (2 * SLW_CTRL_OFFSET),
        SLW_BUTTON_HEIGHT,
        m_hWnd,
        (HMENU)ID_SERVER_BUTTON_ADD,
        m_hInstance,
        NULL);

    m_hButtonEdit = CreateWindow(WC_BUTTON, _T("�༭������"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        SLW_CTRL_OFFSET,
        cy - SLW_CTRL_OFFSET - (2 * SLW_BUTTON_HEIGHT) - (1 * SLW_BUTTON_V_INTERVAL),
        cx - (2 * SLW_CTRL_OFFSET),
        SLW_BUTTON_HEIGHT,
        m_hWnd,
        (HMENU)ID_SERVER_BUTTON_EDIT,
        m_hInstance,
        NULL);

    m_hButtonRemove = CreateWindow(WC_BUTTON, _T("ɾ��������"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        SLW_CTRL_OFFSET,
        cy - SLW_CTRL_OFFSET - (1 * SLW_BUTTON_HEIGHT) - (0 * SLW_BUTTON_V_INTERVAL),
        cx - (2 * SLW_CTRL_OFFSET),
        SLW_BUTTON_HEIGHT,
        m_hWnd,
        (HMENU)ID_SERVER_BUTTON_REMOVE,
        m_hInstance,
        NULL);

    SendMessage(m_hServerListTree, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    SendMessage(m_hButtonAdd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    SendMessage(m_hButtonEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    SendMessage(m_hButtonRemove, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);

    TreeView_SetBkColor(m_hServerListTree, SLW_TREE_BG_CLR);

    // ��ʼ��ʱ�������ĸ����
    InsertServerNodeToTree(_T("��������Ϸ������"), 0, 0, TVI_ROOT, INVALID_SERVER_INDEX);
}

// ��Ӧ�ؼ�����
void ServerListWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    UINT nCtrlID = LOWORD(wParam);

    switch (nCtrlID) {
        case ID_SERVER_BUTTON_ADD:
            OnButtonAdd();
            break;

        case ID_SERVER_BUTTON_EDIT:
            OnButtonEdit();
            break;

        case ID_SERVER_BUTTON_REMOVE:
            OnButtonRemove();
            break;
    }
}

// ��Ӧ�ؼ���֪ͨ
void ServerListWnd::OnNotify(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    LPNMHDR nmhdr = (LPNMHDR)lParam;
    switch (nmhdr->idFrom) {
    case ID_SERVER_LIST_TREE:
        switch (nmhdr->code) {
        case NM_RETURN:
        case NM_DBLCLK:
            SetFocus(GetParent(m_hWnd));
            OnDoubleClickServerTree(nmhdr->hwndFrom);
            break;

        case TVN_SELCHANGED:
            OnServerTreeSelChange(nmhdr->hwndFrom);
            break;

        case NM_RCLICK:
            OnRClickServerTree(nmhdr->hwndFrom);
            break;
        }
        break;
    }
}

// ���ڳߴ�仯
void ServerListWnd::OnSize(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    int width = LOWORD(lParam);
    int height = HIWORD(lParam);

    if ((width <= 0) || (height <= 0)) { return; }

    //
    // �����Ӵ��ڳߴ�
    //
    ::MoveWindow(m_hServerListTree,
        SLW_CTRL_OFFSET,
        SLW_CTRL_OFFSET,
        width - (2 * SLW_CTRL_OFFSET),
        height - (2 * SLW_CTRL_OFFSET) - (3 * (SLW_BUTTON_HEIGHT + SLW_BUTTON_V_INTERVAL)),
        TRUE);

    ::MoveWindow(m_hButtonAdd,
        SLW_CTRL_OFFSET,
        height - SLW_CTRL_OFFSET - (3 * SLW_BUTTON_HEIGHT) - (2 * SLW_BUTTON_V_INTERVAL),
        width - (2 * SLW_CTRL_OFFSET),
        SLW_BUTTON_HEIGHT,
        TRUE);

    ::MoveWindow(m_hButtonEdit,
        SLW_CTRL_OFFSET,
        height - SLW_CTRL_OFFSET - (2 * SLW_BUTTON_HEIGHT) - (1 * SLW_BUTTON_V_INTERVAL),
        width - (2 * SLW_CTRL_OFFSET),
        SLW_BUTTON_HEIGHT,
        TRUE);

    ::MoveWindow(m_hButtonRemove,
        SLW_CTRL_OFFSET,
        height - SLW_CTRL_OFFSET - (1 * SLW_BUTTON_HEIGHT) - (0 * SLW_BUTTON_V_INTERVAL),
        width - (2 * SLW_CTRL_OFFSET),
        SLW_BUTTON_HEIGHT,
        TRUE);
}

// ˫�����������б�
void ServerListWnd::OnDoubleClickServerTree(HWND hWnd)
{
    HTREEITEM hItem = TreeView_GetSelection(m_hServerListTree);

    if (hItem != NULL) {
        TVITEM tvi = { 0 };
        tvi.mask = TVIF_PARAM;
        tvi.hItem = hItem;

        if (TreeView_GetItem(hWnd, &tvi) == TRUE) {
            int index = tvi.lParam;

            if (index != INVALID_SERVER_INDEX) {
                SendMessage(m_hWndParent, WM_CONNECT_SERVER, 
                    (WPARAM)m_ServerInfo[index].svi.ip,
                    MAKELPARAM(m_ServerInfo[index].svi.port, 0));
            }
        }
    }
}

// ����������������б�
void ServerListWnd::OnServerTreeSelChange(HWND hWnd)
{
    HTREEITEM hItem = TreeView_GetSelection(m_hServerListTree);

    if (hItem != NULL) {
        TVITEM tvi = { 0 };
        tvi.mask = TVIF_PARAM;
        tvi.hItem = hItem;

        if (TreeView_GetItem(hWnd, &tvi) == TRUE) {
            int index = tvi.lParam;

            if (index == INVALID_SERVER_INDEX) { // �������㣬�����Ƿ��������
                SendMessage(m_hWndParent, WM_SHOW_HOME_PAGE, 0, 0);
            }
        }
    }
}

// �Ҽ��������������б�
void ServerListWnd::OnRClickServerTree(HWND hWnd)
{
    UNREFERENCED_PARAMETER(hWnd);
}

// ����TreeViewͼ��
void ServerListWnd::SetServerInfoImageList(HIMAGELIST himl)
{
    TreeView_SetImageList(m_hServerListTree, himl, TVSIL_NORMAL);
}

// ��ʾ�����ش���
void ServerListWnd::Show(BOOL bShow /*= TRUE*/)
{
    if (m_bShow != bShow) {
        ShowWindow(m_hWnd, bShow ? SW_SHOW : SW_HIDE);
        m_bShow = bShow;
    }
}

// ��ѯ�����Ƿ�ɼ�
BOOL ServerListWnd::IsVisible(void)
{
    return m_bShow;
}

// ���ô��ڳߴ�
void ServerListWnd::SetWindowRect(int x, int y, int cx, int cy)
{
    // ������崰�ڳߴ�
    ::MoveWindow(m_hWnd, x, y, cx, cy, TRUE);
}

// ����Ҫ���صķ����б��ļ���������ļ�����������·��
void ServerListWnd::SetServerInfoConfigFile(LPCTSTR lpszFileName)
{
    if (lpszFileName == NULL) {
        return;
    }

    int nFileNameLen = _tcslen(lpszFileName);
    if (nFileNameLen >= MAX_PATH) {
        return;
    }

    GetModuleFileName(NULL, m_szConfigFile, sizeof(m_szConfigFile) / sizeof(m_szConfigFile[0]));
    PathRemoveFileSpec(m_szConfigFile);
    _tcscat_s(m_szConfigFile, sizeof(m_szConfigFile) / sizeof(m_szConfigFile[0]), _T("\\"));
    _tcscat_s(m_szConfigFile, sizeof(m_szConfigFile) / sizeof(m_szConfigFile[0]), lpszFileName);

    LoadServerInfoFile(m_szConfigFile);
    InitServerInfoTree();
}

// ��ӷ�������㵽TREEVIEW
void ServerListWnd::InsertServerNodeToTree(LPTSTR szLabel,
    int nImage, int nSelImage, HTREEITEM htiParent, LPARAM lParam)
{
    TVITEM tvi = { 0 };
    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvi.pszText = szLabel;
    tvi.cchTextMax = _tcslen(szLabel);
    tvi.iImage = nImage;
    tvi.iSelectedImage = nSelImage;
    tvi.lParam = lParam;

    TVINSERTSTRUCT tvins = { 0 };
    tvins.hParent = htiParent;
    tvins.hInsertAfter = TVI_SORT; //TVI_LAST;
    tvins.item = tvi;

    TreeView_InsertItem(m_hServerListTree, &tvins);
}

// ����һ�����õ����������ڱ����������Ϣ
int ServerListWnd::FindAvailableIndex(void)
{
    for (int i = 0; i < MAX_SERVER_NUM_SUPPORTED; i++) {
        if (m_ServerInfo[i].valid != TRUE) {
            return i;
        }
    }

    return INVALID_SERVER_INDEX;
}

// ����INI�ļ��е�SECTION������ʶ����ʹ�õ����������뱣��INIʱ����һ�£�
int ServerListWnd::CreateIndexFromSectionName(LPCTSTR szSecName)
{
    if (szSecName != NULL) {
        LPCTSTR pNumber = szSecName + _tcslen(SERVER_SECTION_NAME);
        int index = _tstoi(pNumber);
        if ((index >= 0) && (index < MAX_SERVER_NUM_SUPPORTED)) {
            return index;
        }
    }

    return INVALID_SERVER_INDEX;
}

// ����������б���Ϣ
void ServerListWnd::SaveServerInfo(int index, LPCTSTR name, DWORD ip, USHORT port)
{
    if ((index < 0) || (index >= MAX_SERVER_NUM_SUPPORTED)) {
        return;
    }

    _tcscpy_s(m_ServerInfo[index].svi.name, MAX_SERVER_NAME_LEN, name);
    m_ServerInfo[index].svi.ip = ip;
    m_ServerInfo[index].svi.port = port;
    m_ServerInfo[index].valid = TRUE;
}

// ���ɾ���ķ�������Ϣ
void ServerListWnd::RemoveServerInfo(int index)
{
    if ((index < 0) || (index >= MAX_SERVER_NUM_SUPPORTED)) {
        return;
    }

    ZeroMemory(&m_ServerInfo[index], sizeof(SERVER_INFO));
}

// ���������Ϣ���ļ�
void ServerListWnd::SaveServerInfoToConfigFile(int index, SF_FLAG flag)
{
    if ((index < 0) || (index >= MAX_SERVER_NUM_SUPPORTED)) {
        return;
    }

    TCHAR section[sizeof(SERVER_SECTION_NAME) / sizeof(TCHAR) + 4] = { 0 };
    TCHAR temp[_LOCAL_CHAR_ARRAY_SZ] = { 0 };

    // ����SECTION����
    _stprintf_s(temp, _LOCAL_CHAR_ARRAY_SZ, _T("%d"), index);
    _tcscpy_s(section, sizeof(section) / sizeof(section[0]), SERVER_SECTION_NAME);
    _tcscat_s(section, sizeof(section) / sizeof(section[0]), temp);

    if (flag == SF_REMOVE) { // Remove
        WritePrivateProfileString(section, SERVER_SECTION_KEY_NAME, NULL, m_szConfigFile);
        WritePrivateProfileString(section, SERVER_SECTION_KEY_IP, NULL, m_szConfigFile);
        WritePrivateProfileString(section, SERVER_SECTION_KEY_PORT, NULL, m_szConfigFile);
    } else { // Add or Modify
        // name
        WritePrivateProfileString(section, SERVER_SECTION_KEY_NAME, 
            m_ServerInfo[index].svi.name, m_szConfigFile);

        // IP
        _stprintf_s(temp, _LOCAL_CHAR_ARRAY_SZ, _T("%u.%u.%u.%u"),
            FIRST_IPADDRESS(m_ServerInfo[index].svi.ip),
            SECOND_IPADDRESS(m_ServerInfo[index].svi.ip),
            THIRD_IPADDRESS(m_ServerInfo[index].svi.ip),
            FOURTH_IPADDRESS(m_ServerInfo[index].svi.ip));

        WritePrivateProfileString(section, SERVER_SECTION_KEY_IP, temp, m_szConfigFile);

        // PORT
        _stprintf_s(temp, _LOCAL_CHAR_ARRAY_SZ, _T("%d"), m_ServerInfo[index].svi.port);
        WritePrivateProfileString(section, SERVER_SECTION_KEY_PORT, temp, m_szConfigFile);
    }
}

// ���ط������б������ļ�
BOOL ServerListWnd::LoadServerInfoFile(LPCTSTR lpszFileName)
{
    // ��ȡ����SECTION
    int offset = 0;
    DWORD nAllSecLen = 0;
    TCHAR* buf = NULL;
    
    nAllSecLen = MAX_SERVER_NUM_SUPPORTED * (_tcslen(SERVER_SECTION_NAME) + 2) * sizeof(TCHAR);

    buf = (TCHAR*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, nAllSecLen);
    if (buf == NULL) {
        return FALSE;
    }

    GetPrivateProfileSectionNames(buf, nAllSecLen, lpszFileName);

    while (buf[offset]) {
        TCHAR* section = buf + offset;

        int index = CreateIndexFromSectionName(section);
        if ((index == INVALID_SERVER_INDEX)) {
            goto ReadNextSection;
        }

        // ����������
        DWORD nReadBytes = GetPrivateProfileString(section,
            SERVER_SECTION_KEY_NAME, NULL,
            m_ServerInfo[index].svi.name, MAX_SERVER_NAME_LEN, lpszFileName);

        // IP
        TCHAR szIpAddr[_LOCAL_CHAR_ARRAY_SZ];
        GetPrivateProfileString(section, SERVER_SECTION_KEY_IP,
            NULL, szIpAddr, _LOCAL_CHAR_ARRAY_SZ, lpszFileName);
        
        DWORD b1, b2, b3 ,b4;
        _stscanf_s(szIpAddr, _T("%u.%u.%u.%u"), &b1, &b2, &b3, &b4);

        m_ServerInfo[index].svi.ip = MAKEIPADDRESS(b1, b2, b3, b4);

        // �˿�
        m_ServerInfo[index].svi.port = (USHORT)GetPrivateProfileInt(section,
            SERVER_SECTION_KEY_PORT, 0, lpszFileName);

        if (nReadBytes > 0) {
            if (m_ServerInfo[index].svi.ip > 0) {
                if (m_ServerInfo[index].svi.port > 0) {
                    m_ServerInfo[index].valid = TRUE; // ��ʶռ�ø�����
                }
            }
        }

ReadNextSection:
        offset += _tcslen(section);
        offset += 1; // NULL character of the current section
    }

    LocalFree(buf);
    return TRUE;
}

// ��ʼ�����������б���Ŀ
BOOL ServerListWnd::InitServerInfoTree(void)
{
    HTREEITEM htiParent = TreeView_GetRoot(m_hServerListTree);
    if (htiParent == NULL) {
        return FALSE;
    }

    for (int i = 0; i < MAX_SERVER_NUM_SUPPORTED; i++) {
        if (m_ServerInfo[i].valid == TRUE) {
            InsertServerNodeToTree(m_ServerInfo[i].svi.name, 1, 1, htiParent, i);
        }
    }

    return TRUE;
}

// ȷ����ӷ�����
BOOL ServerListWnd::OnAddServerOK(HWND hDlg)
{
    TCHAR name[MAX_SERVER_NAME_LEN] = { 0 };
    DWORD ip = 0;
    TCHAR port[_LOCAL_CHAR_ARRAY_SZ] = { 0 };

    GetWindowText(GetDlgItem(hDlg, IDC_SERVER_NAME), name, MAX_SERVER_NAME_LEN);
    SendMessage(GetDlgItem(hDlg, IDC_SERVER_IP), IPM_GETADDRESS, 0, (LPARAM)&ip);
    GetWindowText(GetDlgItem(hDlg, IDC_SERVER_PORT), port, _LOCAL_CHAR_ARRAY_SZ);

    if ((_tcslen(name) == 0) && (_tcslen(port) == 0) && (ip == 0)) {
        MessageBox(hDlg, _T("�������������Ϣ"), _T("��ʾ"), MB_OK);
        return FALSE;
    }

    if (_tstoi(port) > (int)((USHORT)-1)) {
        MessageBox(hDlg, _T("����Ķ˿ںŲ��Ϸ�"), _T("��ʾ"), MB_OK);
        return FALSE;
    }

    SERVER_INFO* si = (SERVER_INFO*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    if (si == NULL) {
        return FALSE;
    }

    _tcscpy_s(si->svi.name, MAX_SERVER_NAME_LEN, name);
    si->svi.ip = ip;
    si->svi.port = (USHORT)_tstoi(port);
    si->valid = TRUE;

    EndDialog(hDlg, IDOK);
    return TRUE;
}

// ��ӷ������ĶԻ�������
INT_PTR
CALLBACK
ServerListWnd::AddServerInfoProc(HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    switch (nMsg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        
        if (LOWORD(wParam) == IDOK) {
            return (INT_PTR)OnAddServerOK(hDlg);
        }
        break;

    case WM_DESTROY:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
        break;
    }

    return (INT_PTR)FALSE;
}

// ��Ӧ��ӷ�������ť
void ServerListWnd::OnButtonAdd(void)
{
    int index = FindAvailableIndex();
    if (index == INVALID_SERVER_INDEX) {
        MessageBox(NULL, _T("�����������Ѿ��ﵽ�޶�ֵ�����������"), _T("��������������"), MB_OK);
        return;
    }

    int nResult = DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_SERVER_INFO),
        m_hWnd, AddServerInfoProc, (LPARAM)&m_ServerInfo[index]);

    if (nResult == IDOK) {
        HTREEITEM htiParent = TreeView_GetRoot(m_hServerListTree);
        if (htiParent != NULL) {
            InsertServerNodeToTree(m_ServerInfo[index].svi.name, 1, 1, htiParent, index);
            SaveServerInfoToConfigFile(index, SF_APPEND);
        }
    }
}

// �༭��������Ϣʱ���OK
BOOL ServerListWnd::OnEditServerOK(HWND hDlg)
{
    return OnAddServerOK(hDlg);
}

// �༭�������ĶԻ�������
INT_PTR
CALLBACK
ServerListWnd::EditServerInfoProc(HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    SERVER_INFO* si = NULL;
    TCHAR szPort[_LOCAL_CHAR_ARRAY_SZ] = { 0 };

    switch (nMsg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);

        si = (SERVER_INFO*)lParam;
        assert(si != NULL);

        SetWindowText(GetDlgItem(hDlg, IDC_SERVER_NAME), si->svi.name);
        SendDlgItemMessage(hDlg, IDC_SERVER_IP, IPM_SETADDRESS, 0, si->svi.ip);

        _stprintf_s(szPort, _LOCAL_CHAR_ARRAY_SZ, _T("%d"), si->svi.port);
        SetWindowText(GetDlgItem(hDlg, IDC_SERVER_PORT), szPort);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL) {
            return (INT_PTR)EndDialog(hDlg, IDCANCEL);
        }

        if (LOWORD(wParam) == IDOK) {
            return (INT_PTR)OnEditServerOK(hDlg);
        }
        break;

    case WM_DESTROY:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
        break;
    }

    return (INT_PTR)FALSE;
}

// ��Ӧ�༭��������ť
void ServerListWnd::OnButtonEdit(void)
{
    HTREEITEM hItem = TreeView_GetSelection(m_hServerListTree);
    if (hItem == NULL) {
        return;
    }

    TVITEM tvi = { 0 };
    tvi.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvi.hItem = hItem;

    if (TreeView_GetItem(m_hServerListTree, &tvi) == TRUE) {
        int index = tvi.lParam;

        if (tvi.lParam != INVALID_SERVER_INDEX) { // not the ROOT
            int ret = DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_SERVER_INFO),
                m_hWnd, EditServerInfoProc, (LPARAM)&m_ServerInfo[index]);

            if (ret == IDOK) {
                // ������ʾ�ı�ǩ
                tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
                tvi.pszText = m_ServerInfo[index].svi.name;
                tvi.cchTextMax = _tcslen(m_ServerInfo[index].svi.name);
                TreeView_SetItem(m_hServerListTree, &tvi);

                SaveServerInfoToConfigFile(index, SF_MODIFY);
            }
        }
    }
}

// ��Ӧɾ����������ť
void ServerListWnd::OnButtonRemove(void)
{
    HTREEITEM hItem = TreeView_GetSelection(m_hServerListTree);
    if (hItem == NULL) {
        return;
    }

    TVITEM tvi = { 0 };
    tvi.mask = TVIF_PARAM;
    tvi.hItem = hItem;

    if (TreeView_GetItem(m_hServerListTree, &tvi) == TRUE) {
        int index = tvi.lParam;

        if (index != INVALID_SERVER_INDEX) { // ROOT not to delete
            if (TreeView_DeleteItem(m_hServerListTree, hItem) == TRUE) {
                RemoveServerInfo(index);
                SaveServerInfoToConfigFile(index, SF_REMOVE);
            }
        }
    }
}
