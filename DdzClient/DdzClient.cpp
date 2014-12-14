// DdzClient.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "NetProcess.h"

#include "Logon.h"
#include "MyButton.h"

#include "UserInfoWnd.h"
#include "ServerListWnd.h"

#include "GameSeatWnd.h"
#include "GameTableWnd.h"
#include "GameRoomWnd.h"

#include "HomePageWnd.h"
#include "PageTabWnd.h"
#include "UserListWnd.h"

#include "CurPokerWnd.h"
#include "OutCardWnd.h"
#include "UnderCardWnd.h"
#include "GamerVisualWnd.h"
#include "GameMainWnd.h"
#include "GameMainPage.h"

#include "DdzClient.h"


// ȫ�ֱ���:
HINSTANCE   g_hAppInstance;                             // ��ǰʵ��
TCHAR       g_szAppTitle[MAX_LOADSTRING];               // �������ı�
TCHAR       g_szAppWindowClass[MAX_LOADSTRING];         // ����������

LPCTSTR     g_lpszSingleInstEntryName = _T("DdzClient_Single_Instance");

DWORD       g_nMainThreadId = 0;
HWND        g_hMainWnd = NULL;

BOOL        g_bShutdownConnection = FALSE;
BOOL        g_bProgramWillExit = FALSE;
SOCKET      g_ConSocket = INVALID_SOCKET;
DWORD       g_nConThreadId = 0;
HANDLE      g_hConThread = NULL;


// ��ǰ�û�������Ϣ
USER_LOGON_INFO     g_UserLogonInfo = { _T("Shining"), 2, TRUE };

// �û����棨���ڣ�
PageTabWnd          g_PageTabWnd;

UserInfoWnd         g_UserInfoWnd;
ServerListWnd       g_ServerListWnd;
HomePageWnd         g_HomePageWnd;

GameRoomWnd         g_GameRoomWnd;
UserListWnd         g_UserListWnd;

GameMainPage        g_GameMainPage;



// ���������
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    // ��ʼ��ȫ���ַ���
    LoadString(hInstance, IDS_APP_TITLE, g_szAppTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_DDZCLIENT, g_szAppWindowClass, MAX_LOADSTRING);

    g_nMainThreadId = GetCurrentThreadId();
    g_hAppInstance  = hInstance;

    // ��Ϊ��һʵ������
#ifdef RUN_AS_SINGLE_INSTANCE
    if (!IsFirstInstance()) {
        return 0;
    }
#endif

    // ע������ʹ�õĴ�����
    RegisterWindowClass(hInstance);

    // ��ʼ�����������Ϣ�����ݽṹ
    ResetPlayerDataTable();

    // ��ʼ��WinSock
    if (!InitWinSocket()) {
        MessageBox(NULL, _T("��ϵͳ��WinSock�汾����2.2�������޷����С�\n�밲װ�߼��汾��WinSock��"),
            _T("WinSock�汾̫��"), MB_OK | MB_ICONERROR);
        return -1;
    }

    // ������Դ
    if (!LoadAppResource()) {
        MessageBox(NULL, _T("���س�����������Ҫ����Դʧ�ܣ���ȷ����ԴDLLû���𻵡�"),
            _T("������Դ����"), MB_OK | MB_ICONERROR);
        CloseWinSock();
        return -1;
    }


	// ��������
    if (!InitInstance(hInstance, nCmdShow)) {
        FreeAppResource();
        CloseWinSock();
		return -1;
	}

    int ret;
    MSG msg;
    HACCEL hAccelTable;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DDZCLIENT));

    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (ret == -1) { continue; }

        if (ProcessNetMessage(&msg)) { continue; }

        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    ExitInstance();

	return (int)msg.wParam;
}

// �˳�����ʱ��������
void ExitInstance(void)
{
    if (g_ConSocket != INVALID_SOCKET) {
        g_bShutdownConnection = TRUE;
        CloseConnection(g_ConSocket);
        g_ConSocket = INVALID_SOCKET;

        if (g_hConThread != NULL) {
            WaitForSingleObject(g_hConThread, INFINITE);
            CloseHandle(g_hConThread);
            g_hConThread = NULL;
        }
    }

    CloseWinSock();
    FreeAppResource();
}

// ע�ᴰ����
ATOM MainWndRegister(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
    wcex.lpszClassName	= g_szAppWindowClass;
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DDZCLIENT));
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hbrBackground	= NULL;//(HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_DDZCLIENT);

	return RegisterClassEx(&wcex);
}

// ע������н�ʹ�õ��Ĵ�����
BOOL RegisterWindowClass(HINSTANCE hInstance)
{
    // ��ʼ�����õĿؼ�������
    INITCOMMONCONTROLSEX ccex;
    ccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    ccex.dwICC = ICC_STANDARD_CLASSES | ICC_USEREX_CLASSES;
    InitCommonControlsEx(&ccex);

    // ��������
    MainWndRegister(hInstance);

    // ע������õ��Ĵ�����
    UserInfoWnd::UserInfoWndRegister(hInstance);
    ServerListWnd::ServerListWndRegister(hInstance);
    HomePageWnd::HomePageWndRegister(hInstance);
    PageTabWnd::PageTabWndRegister(hInstance);

    GameSeatWnd::GameSeatWndRegister(hInstance);
    GameTableWnd::GameTableWndRegister(hInstance);
    GameRoomWnd::GameRoomWndRegister(hInstance);

    UserListWnd::UserListWndRegister(hInstance);

    MyButton::MyButtonRegister(hInstance);
    CurPokerWnd::CurPokerWndRegister(hInstance);
    OutCardWnd::OutCardWndRegister(hInstance);
    UnderCardWnd::UnderCardWndRegister(hInstance);
    GamerVisualWnd::GamerVisualWndRegister(hInstance);
    GameMainWnd::GameMainWndRegister(hInstance);
    GameMainPage::GameMainPageRegister(hInstance);

    return TRUE;
}

// ��Ϊ��һ����ʵ������
BOOL IsFirstInstance(void)
{
    HANDLE hSem = CreateSemaphore(NULL, 1, 1, g_lpszSingleInstEntryName);

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hSem);

        HWND hWndPrevious = GetWindow(GetDesktopWindow(), GW_CHILD);

        while (IsWindow(hWndPrevious)) {
            if (GetProp(hWndPrevious, g_lpszSingleInstEntryName) != NULL) {
                if (IsIconic(hWndPrevious)) {
                    ShowWindow(hWndPrevious, SW_RESTORE);
                }

                SetForegroundWindow(hWndPrevious);
                return FALSE;
            }

            hWndPrevious = GetWindow(hWndPrevious, GW_HWNDNEXT);
        }

        MessageBox(NULL, _T("Ӧ�ó���ʵ���Ѵ��ڣ����Ҳ�����������"), _T("����"), MB_OK);
        return FALSE;
    }

    return TRUE;
}

// ��ʼ��Ӧ�ó���ʵ������������ʾ������
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    UNREFERENCED_PARAMETER(nCmdShow);

    HWND hWnd = CreateWindow(g_szAppWindowClass, g_szAppTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) {
        return FALSE;
    }

    g_hMainWnd = hWnd;
    SetProp(hWnd, g_lpszSingleInstEntryName, (HANDLE)1);

#if 1
    // ��ʾ��½�Ի��򣬲���ȡ�û�������Ϣ
    if (!UserLogon(hInstance, NULL, (LPARAM)&g_UserLogonInfo)) {
        DestroyWindow(hWnd);
        return FALSE;
    }
#endif

    // �����û���Ϣ���
    g_UserInfoWnd.Create(0, 0, 0, 0, hWnd, hInstance);
    g_UserInfoWnd.SetUserInfo(g_UserLogonInfo.szName, g_UserLogonInfo.nImage);

    // �����������б����
    g_ServerListWnd.Create(0, 0, 0, 0, hWnd, hInstance);
    g_ServerListWnd.SetServerInfoConfigFile(SERVER_LIST_INI_FILE);
    g_ServerListWnd.SetServerInfoImageList(g_himlServerTree);

    // ����PageTableCtrl
    g_PageTabWnd.Create(0, 0, 0, 0, hWnd, hInstance);
    g_PageTabWnd.SetTabImageList(g_himlTabCtrl);
    g_PageTabWnd.InsertHomeTabPage();

    HMODULE hModule = LoadLibrary(RES_POKER_DLL_NAME);
    if (hModule != NULL) {
        g_PageTabWnd.m_btnReturn.SetBitmap(hModule,
            MAKEINTRESOURCE(IDB_BTN_TEMPLATE), RGB(255,255,255));
        FreeLibrary(hModule);
    }

    g_PageTabWnd.m_btnReturn.SetText(_T("����"), RGB(255,255,255));

    // ������ҳ����
    g_HomePageWnd.Create(0, 0, 400, 300, hWnd, hInstance);

#ifdef USE_LOCAL_FILE_AS_HOMEPAGE
    TCHAR homepage[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, homepage, MAX_PATH);
    PathRemoveFileSpec(homepage);
    _tcscat_s(homepage, MAX_PATH, _T("\\"));
    _tcscat_s(homepage, MAX_PATH, DEFAULT_HOME_PAGE);
    g_HomePageWnd.SetPageURL(homepage);
#else
    g_HomePageWnd.SetPageURL(DEFAULT_HOME_PAGE);
#endif

    // ������������
    g_GameRoomWnd.Create(0, 0, 400, 300, hWnd, hInstance);
    g_GameRoomWnd.Show(FALSE);

    // �����û��б����
    g_UserListWnd.Create(0, 0, 0, 0, hWnd, hInstance);
    g_UserListWnd.Show(FALSE);

    // ������Ϸ��ҳ�洰��
    g_GameMainPage.Create(0, 0, 0, 0, hWnd, hInstance);
    g_GameMainPage.Show(FALSE);

    //ShowWindow(hWnd, nCmdShow);
    ShowWindow(hWnd, SW_MAXIMIZE);
    UpdateWindow(hWnd);

    return TRUE;
}

// ���������ڵ���Ϣ��
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	UINT wmId;
    UINT wmEvent;

    HDC hdc;
    PAINTSTRUCT ps;

    LPMINMAXINFO lpMinMaxInfo;

	switch (nMsg) {
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

        switch (wmId) {
		    case IDM_ABOUT:
                DialogBox(g_hAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			    break;
		    case IDM_EXIT:
			    DestroyWindow(hWnd);
			    break;
		    default:
			    return DefWindowProc(hWnd, nMsg, wParam, lParam);
		}
		break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        OnPaint(hdc);
        EndPaint(hWnd, &ps);
        break;

    case WM_SIZE:
        OnSize(wParam, lParam);
        break;

    case WM_GETMINMAXINFO:
        lpMinMaxInfo = (LPMINMAXINFO)lParam;
        lpMinMaxInfo->ptMinTrackSize.x = MAIN_WND_MIN_WIDTH;
        lpMinMaxInfo->ptMinTrackSize.y = MAIN_WND_MIN_HEIGHT;
        break;

    case WM_SHOW_HOME_PAGE:
        if (g_PageTabWnd.GetCurTabSel() != HOME_PAGE_TAB_INDEX) {
            g_PageTabWnd.SetCurTabSel(HOME_PAGE_TAB_INDEX);
            OnPageSelChange(hWnd, HOME_PAGE_TAB_PARAM);
        }
        break;

    case WM_CONNECT_SERVER:
        OnConnectServer(wParam, lParam);
        break;

    case WM_PAGE_RETURN:
        OnPageReturn(hWnd, lParam);
        break;

    case WM_PAGE_SEL_CHANGING:
        OnPageSelChanging(hWnd, lParam);
        break;

    case WM_PAGE_SEL_CHANGE:
        OnPageSelChange(hWnd, lParam);
        break;

    case WM_USER_CLICK_SEAT:
        OnUserClickSeat(wParam, lParam);
        break;

	case WM_DESTROY:
        RemoveProp(hWnd, g_lpszSingleInstEntryName);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, nMsg, wParam, lParam);
	}

	return 0;
}

// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (nMsg) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}

	return (INT_PTR)FALSE;
}

// ���ƴ���
void OnPaint(HDC hdc)
{
    RECT rect;
    GetClientRect(g_hMainWnd, &rect);

    // �����ڽ�����һ���߿�
    HPEN hpen = CreatePen(PS_SOLID | PS_INSIDEFRAME, MAIN_WND_MARGIN, MAIN_WND_MARGIN_CLR);
    HPEN oldpen = (HPEN)SelectObject(hdc, hpen);
    HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Rectangle(hdc, 0, 0, rect.right, rect.bottom);

    SelectObject(hdc, oldbrush);
    SelectObject(hdc, oldpen);
    DeleteObject(hpen);
}

// ���ڳߴ�ı�
void OnSize(WPARAM wParam, LPARAM lParam)
{
    int cx = LOWORD(lParam);
    int cy = HIWORD(lParam);

    if ((cx <= 0) || (cy <= 0)) { return; }

    // ����������һ��հ����������Եò�̫ӵ��������Щ
    cx -= (2 * MAIN_WND_MARGIN);
    cy -= (2 * MAIN_WND_MARGIN);

    switch (wParam) {
    case SIZE_MAXIMIZED:
    case SIZE_RESTORED:
        if (g_UserInfoWnd.IsVisible() && g_ServerListWnd.IsVisible()) {
            g_UserInfoWnd.SetWindowRect(MAIN_WND_MARGIN, MAIN_WND_MARGIN,
                CX_USER_INFO_WND, CY_USER_INFO_WND);

            g_ServerListWnd.SetWindowRect(MAIN_WND_MARGIN, MAIN_WND_MARGIN + CY_USER_INFO_WND,
                CX_SERVER_LIST_WND, cy - CY_USER_INFO_WND);

            g_PageTabWnd.SetWindwRect(MAIN_WND_MARGIN + CX_USER_INFO_WND, MAIN_WND_MARGIN,
                cx - CX_USER_INFO_WND, CY_PAGE_TAB_CTRL);

            g_HomePageWnd.SetWindowRect(MAIN_WND_MARGIN + CX_USER_INFO_WND, MAIN_WND_MARGIN + CY_PAGE_TAB_CTRL,
                cx - CX_USER_INFO_WND, cy - CY_PAGE_TAB_CTRL);
        } else if (g_GameRoomWnd.IsVisible()) {
            g_PageTabWnd.SetWindwRect(MAIN_WND_MARGIN, MAIN_WND_MARGIN,
                cx - CX_USER_LIST_WND, CY_PAGE_TAB_CTRL);

            g_GameRoomWnd.SetWindowRect(MAIN_WND_MARGIN, MAIN_WND_MARGIN + CY_PAGE_TAB_CTRL,
                cx - CX_USER_LIST_WND, cy - CY_PAGE_TAB_CTRL);

            g_UserListWnd.SetWindowRect(MAIN_WND_MARGIN + cx - CX_USER_LIST_WND, MAIN_WND_MARGIN,
                CX_USER_LIST_WND, cy);
        } else {// if (g_GameMainPage.IsVisible()) {
            g_PageTabWnd.SetWindwRect(MAIN_WND_MARGIN, MAIN_WND_MARGIN,
                cx, CY_PAGE_TAB_CTRL);

            g_GameMainPage.SetWindowRect(MAIN_WND_MARGIN, MAIN_WND_MARGIN + CY_PAGE_TAB_CTRL,
                cx, cy - CY_PAGE_TAB_CTRL);
        }
        break;
    }
}

// �Ͽ����ӣ�����λ�������
void DisconnectServer(void)
{
    if (g_ConSocket == INVALID_SOCKET) {
        return;
    }

    g_bShutdownConnection = TRUE;
    CloseConnection(g_ConSocket);
    g_ConSocket = INVALID_SOCKET;

    WaitForSingleObject(g_hConThread, INFINITE);
    CloseHandle(g_hConThread);
    g_hConThread = NULL;
    g_bShutdownConnection = FALSE;

    g_UserListWnd.RemoveAllUserItems();
    g_GameRoomWnd.ResetGameRoom();
    g_GameMainPage.Init(FALSE);

    ResetPlayerDataTable();
}

// ���ӷ�����
void OnConnectServer(WPARAM wParam, LPARAM lParam)
{
    DWORD ip = (DWORD)wParam;
    USHORT port = LOWORD(lParam);

    PLAYER_STATE state = GetLocalUserState();
    if (state != STATE_IDLE) {
        g_GameMainPage.WriteLog(LOG_WARN, _T("�����뿪��Ϸ�������ص������������ӱ�ķ�������"));
        return;
    }

    if (g_ConSocket != INVALID_SOCKET) { // IDLE�������Ѿ���ĳ�������������ӣ���ɾ������Tabҳ��
        g_PageTabWnd.RemoveTabPage(ROOM_PAGE_TAB_INDEX);
        g_PageTabWnd.SetCurTabSel(HOME_PAGE_TAB_INDEX);
        OnPageSelChange(g_hMainWnd, (LPARAM)HOME_PAGE_TAB_PARAM);
    }

    // ����Ѿ����ӵ�һ�������������ȶϿ�����
    DisconnectServer();

    g_ConSocket = EstablishConnection(ip, port);
    if (g_ConSocket == INVALID_SOCKET) {
        MessageBox(g_hMainWnd, _T("���ӷ�����ʧ�ܡ�\n��ȷ������������������ȷ���Ѿ����п��á�"),
            _T("����"), MB_OK | MB_ICONERROR);
        return;
    }

    g_hConThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ConnectionReceiveDataProc,
        (LPVOID)g_ConSocket, CREATE_SUSPENDED, &g_nConThreadId);
    if (g_hConThread == NULL) {
        MessageBox(g_hMainWnd, _T("���ӷ������ɹ����������������ݵ��߳�ʧ�ܡ�\n�Ѿ��ر��������ӣ������Լ��Ρ�"),
            _T("����"), MB_OK | MB_ICONERROR);
        CloseConnection(g_ConSocket);
        g_ConSocket = INVALID_SOCKET;
        return;
    }

    //
    // ��������֮���ȷ����Լ�����Ϣ����������Ȼ��ʼ�ա�������
    //
    SendReqRoomInfo();
    ResumeThread(g_hConThread);

    //
    // Ϊ�����������󣬿��ٵ����������������ҿ���û���Լ�����Ϣ������ȴ�һ
    // �ᡣ�Ա��������1�붨ʱ�㲥�ܽ��Լ���½����Ϣ��֪������ҡ�
    //
    Sleep(500);

    g_PageTabWnd.InsertRoomTabPage();
    g_PageTabWnd.SetCurTabSel(ROOM_PAGE_TAB_INDEX);
    OnPageSelChange(g_hMainWnd, ROOM_PAGE_TAB_PARAM);
}

// Tabҳ�淵��
void OnPageReturn(HWND hWnd, LPARAM lParam)
{
    PLAYER_STATE state = GetLocalUserState();
    int nUserId = GetLocalUserId();
    int table = GetLocalUserTableId();
    int seat = GetLocalUserSeatId();

    if (lParam == HOME_PAGE_TAB_PARAM) {
        // No returning at this page.
    }

    if (lParam == ROOM_PAGE_TAB_PARAM) {
        if (state != STATE_IDLE) {
            TRACE(_T("��������Ϸ��������ֱ�ӴӴ������ء������뿪��Ϸ����\n"));
            g_GameMainPage.WriteLog(LOG_WARN, _T("��������Ϸ��������ֱ�ӴӴ������ء������뿪��Ϸ����"));
            return;
        }

        g_PageTabWnd.RemoveTabPage(ROOM_PAGE_TAB_INDEX);
        g_PageTabWnd.SetCurTabSel(HOME_PAGE_TAB_INDEX);
        OnPageSelChange(hWnd, (LPARAM)HOME_PAGE_TAB_PARAM);

        DisconnectServer();
    }

    if (lParam == GAME_PAGE_TAB_PARAM) {
        if (state == STATE_GAMING) {
            // ToDo: ������;�˳���Ϸ�Ի���
            TRACE(_T("��������Ϸ�����ܷ��ء�\n"));
            g_GameMainPage.WriteLog(LOG_WARN, _T("��������Ϸ�����ܷ��ء�"));
            return;
        }

        if (!IS_INVALID_TABLE(table) && !IS_INVALID_SEAT(seat)) {
            if (state == STATE_LOOKON) {
                SendReqLookonLeaveSeat(table, seat);
                g_GameRoomWnd.LookonLeaveSeat(nUserId, table, seat);
            } else {
                SendReqGamerLeaveSeat(table, seat);
                g_GameRoomWnd.GamerLeaveSeat(nUserId, table, seat);
            }

            // ���Լ��ӱ��ش�����Ϣ��ɾ�������صȴ�������1�붨ʱ�㲥��֪ͨ���뿪��λ������Ҫ��Щ
            g_PlayerData[nUserId].playerInfo.table = INVALID_TABLE;
            g_PlayerData[nUserId].playerInfo.seat = INVALID_SEAT;
            g_PlayerData[nUserId].playerInfo.state = STATE_IDLE;
        }

        g_PageTabWnd.RemoveTabPage(GAME_PAGE_TAB_INDEX);
        g_PageTabWnd.SetCurTabSel(ROOM_PAGE_TAB_INDEX);
        OnPageSelChange(hWnd, (LPARAM)ROOM_PAGE_TAB_PARAM);
    }
}

// Tabҳ��׼�������ı�
void OnPageSelChanging(HWND hWnd, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(hWnd);
    UNREFERENCED_PARAMETER(lParam);
}

// Tabҳ���Ѿ������ı�
void OnPageSelChange(HWND hWnd, LPARAM lParam)
{
    if (lParam == HOME_PAGE_TAB_PARAM) { // ��ҳ
        g_UserInfoWnd.Show();
        g_ServerListWnd.Show();
        g_HomePageWnd.Show();
        g_GameRoomWnd.Show(FALSE);
        g_UserListWnd.Show(FALSE);
        g_GameMainPage.Show(FALSE);
    } else if (lParam == ROOM_PAGE_TAB_PARAM) { // ����
        g_HomePageWnd.Show(FALSE);
        g_UserInfoWnd.Show(FALSE);
        g_ServerListWnd.Show(FALSE);
        g_GameRoomWnd.Show();
        g_UserListWnd.Show();
        g_GameMainPage.Show(FALSE);
    } else if (lParam == GAME_PAGE_TAB_PARAM) { // ��Ϸҳ��
        g_UserInfoWnd.Show(FALSE);
        g_ServerListWnd.Show(FALSE);
        g_HomePageWnd.Show(FALSE);
        g_GameRoomWnd.Show(FALSE);
        g_UserListWnd.Show(FALSE);
        g_GameMainPage.Show();
    }

    RECT rect;
    GetClientRect(hWnd, &rect);
    OnSize((WPARAM)SIZE_RESTORED, MAKELPARAM(rect.right, rect.bottom));
}

void OnUserClickSeat(int table, int seat)
{
    int nUserId = GetLocalUserId();
    PLAYER_STATE prev_state = GetLocalUserState();
    int prev_table = GetLocalUserTableId();
    int prev_seat = GetLocalUserSeatId();

    if ((table == prev_table) && (seat == prev_seat)) {
        return;
    }

    // �������Ƿ��Ѿ���������
    BOOL bAvailable = FALSE;

    int nGamerId = GetGamerId(table, seat);
    if (nGamerId == INVALID_USER_ID) {
        bAvailable = TRUE;
    }

    switch (prev_state) {
        case STATE_SIT:
        case STATE_READY:
            if (!IS_INVALID_TABLE(prev_table)) {
                if (!IS_INVALID_SEAT(prev_seat)) { // ��������뿪�������и��±�������
                    SendReqGamerLeaveSeat(prev_table, prev_seat);
                    g_GameRoomWnd.GamerLeaveSeat(nUserId, prev_table, prev_seat);
                }
            }

            g_PageTabWnd.RemoveGameTabPage();
            break;

        case STATE_LOOKON:
            if (!IS_INVALID_TABLE(prev_table)) {
                if (!IS_INVALID_SEAT(prev_seat)) { // �����Թ����뿪�������и��±�������
                    SendReqLookonLeaveSeat(prev_table, prev_seat);
                    g_GameRoomWnd.LookonLeaveSeat(nUserId, prev_table, prev_seat);
                }
            }

            g_PageTabWnd.RemoveGameTabPage();
            break;

        case STATE_GAMING:
            //MessageBox(g_hMainWnd, _T("��������Ϸ�У������뿪��λ��"), _T("��ʾ"), MB_OK);
            g_GameMainPage.WriteLog(LOG_WARN, _T("��������Ϸ�У�����ֱ�ӽ���������λ��"));
            return; // no more execution
    }

    if (bAvailable == TRUE) {
        SendReqGamerTakeSeat(table, seat);
    } else {
        SendReqLookonTakeSeat(table, seat);
    }
}

// ��˸�Ǽ����
void FlashInactiveWnd(HWND hWnd, UINT uCount)
{
    WINDOWINFO wi = { 0 };
    GetWindowInfo(hWnd, &wi);

    if (wi.dwWindowStatus != WS_ACTIVECAPTION) {
        FLASHWINFO fwi = { 0 };
        fwi.cbSize      = sizeof(FLASHWINFO);
        fwi.hwnd        = hWnd;
        fwi.dwFlags     = FLASHW_TRAY | FLASHW_TIMERNOFG;
        fwi.uCount      = uCount;
        fwi.dwTimeout   = 0;

        FlashWindowEx(&fwi);
    }
}

// ��˸������ 4 ��
void FlashInactiveWnd(void)
{
    FlashInactiveWnd(g_hMainWnd, 4);
}

// �������߳���Ϣ����Ҫ���ڴ��������̸߳����̼߳��͵���Ϣ
// ��������Ϣ�򷵻�TRUE�����򷵻�FALSE
BOOL ProcessNetMessage(LPMSG lpMsg)
{
    UINT nMsg = lpMsg->message;

#define ON_TMSG_BREAK(TMSG_ID) \
    case TMSG_ID: { On_ ## TMSG_ID (lpMsg->wParam, lpMsg->lParam); break; }

    switch (nMsg) {
        ON_TMSG_BREAK(  TM_CONNECTION_LOST);
        ON_TMSG_BREAK(  TM_RECV_SVR_ROOM_INFO);
        ON_TMSG_BREAK(  TM_RECV_SVR_STATUS);
        ON_TMSG_BREAK(  TM_RECV_PLAYER_STATE_CHANGE);
        ON_TMSG_BREAK(  TM_GAMER_TAKE_SEAT);
        ON_TMSG_BREAK(  TM_LOOKON_TAKE_SEAT);
        ON_TMSG_BREAK(  TM_GAMER_LEAVE_SEAT);
        ON_TMSG_BREAK(  TM_LOOKON_LEAVE_SEAT);
        ON_TMSG_BREAK(  TM_GAMER_READY);
        ON_TMSG_BREAK(  TM_DISTRIBUTE_POKER);
        ON_TMSG_BREAK(  TM_SVR_REQ_VOTE_LORD);
        ON_TMSG_BREAK(  TM_SVR_NOTIFY_VOTE_LORD);
        ON_TMSG_BREAK(  TM_VOTE_LORD_FINISH);
        ON_TMSG_BREAK(  TM_SVR_REQ_OUTPUT_CARD);
        ON_TMSG_BREAK(  TM_SVR_NOTIFY_OUTPUT_CARD);
        ON_TMSG_BREAK(  TM_SVR_NOTIFY_GAME_OVER);
        ON_TMSG_BREAK(  TM_GAMER_CHATTING);
        ON_TMSG_BREAK(  TM_GAMER_DELEGATED);
        ON_TMSG_BREAK(  TM_GAMER_DISCONNECTED);

        default:
            return FALSE;
    }

    return TRUE;
}

void On_TM_CONNECTION_LOST(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    int nReason = (int)wParam;

    if (nReason == -1) {
        MessageBox(g_hMainWnd, _T("�����̷߳���������ݻ�����ʧ�ܣ��޷���������������ӡ�"),
            _T("��������ʧ��"), MB_OK | MB_ICONERROR);
    } else {
        MessageBox(g_hMainWnd, _T("���������ܹرգ��������������������Ѿ������á�\n��ȷ�ϲ��������ӷ�������"),
            _T("�Ͽ�����"), MB_OK | MB_ICONERROR);
    }

    if (g_hConThread != NULL) {
        WaitForSingleObject(g_hConThread, INFINITE);
        CloseHandle(g_hConThread);
        g_hConThread = NULL;
    }

    g_PageTabWnd.RemoveGameTabPage();
    g_PageTabWnd.RemoveRoomTabPage();
    g_PageTabWnd.SetCurTabSel(HOME_PAGE_TAB_INDEX);
    OnPageSelChange(g_hMainWnd, HOME_PAGE_TAB_PARAM);
    
    // ��λ��Ҫ���ݽṹ
    DisconnectServer();
}

void On_TM_RECV_SVR_ROOM_INFO(WPARAM wParam, LPARAM lParam)
{
    int nClientID = LOWORD(wParam);
    int nPlayerCount = HIWORD(wParam);
    LPPLAYER_INFO lpPlayerInfo = (LPPLAYER_INFO)lParam;

    if (nPlayerCount == 0) { // It's the first user connected to the server, no other users yet.
        assert(lpPlayerInfo == NULL);
        g_nLocalPlayerId = nClientID;
        return;
    }

    assert(lpPlayerInfo != NULL);
    assert((nPlayerCount >= 0) && (nPlayerCount <= MAX_CONNECTION_NUM));
    TRACE2(_T("ACK_ROOM_INFO. Assigned ID: %d, Get %d users\n"), nClientID, nPlayerCount);

    for (int i = 0; i < nPlayerCount; i++) {
        int id = lpPlayerInfo[i].id;

        g_PlayerData[id].valid = TRUE;
        g_PlayerData[id].playerInfo.gender = lpPlayerInfo[i].gender;
        g_PlayerData[id].playerInfo.imageIdx = lpPlayerInfo[i].imageIdx;
        _tcscpy_s(g_PlayerData[id].playerInfo.name, MAX_USER_NAME_LEN, lpPlayerInfo[i].name);
        g_PlayerData[id].playerInfo.bAllowLookon = lpPlayerInfo[i].bAllowLookon;
        g_PlayerData[id].playerInfo.state = lpPlayerInfo[i].state;
        g_PlayerData[id].playerInfo.table = lpPlayerInfo[i].table;
        g_PlayerData[id].playerInfo.seat = lpPlayerInfo[i].seat;

        g_PlayerData[id].playerInfo.nScore = lpPlayerInfo[i].nScore;
        g_PlayerData[id].playerInfo.nTotalGames = lpPlayerInfo[i].nTotalGames;
        g_PlayerData[id].playerInfo.nWinGames = lpPlayerInfo[i].nWinGames;
        g_PlayerData[id].playerInfo.nRunawayTimes = lpPlayerInfo[i].nRunawayTimes;

        // Set UserListWnd Data
        g_UserListWnd.InsertUserItem(id);

        // Set GameRoomWnd Data
        switch (lpPlayerInfo[i].state) {
            case STATE_SIT:
                g_GameRoomWnd.GamerTakeSeat(id, lpPlayerInfo[i].table, lpPlayerInfo[i].seat);
                break;

            case STATE_READY:
                g_GameRoomWnd.GamerTakeSeat(id, lpPlayerInfo[i].table, lpPlayerInfo[i].seat);
                g_GameRoomWnd.GamerGetReady(lpPlayerInfo[i].table, lpPlayerInfo[i].seat, TRUE);
                break;

            case STATE_GAMING:
                g_GameRoomWnd.GamerTakeSeat(id, lpPlayerInfo[i].table, lpPlayerInfo[i].seat);
                g_GameRoomWnd.TableGameStarted(lpPlayerInfo[i].table, TRUE);
                break;

            case STATE_LOOKON:
                g_GameRoomWnd.LookonTakeSeat(id, lpPlayerInfo[i].table, lpPlayerInfo[i].seat);
                break;
        }
    }

    // save the ID which the server assigned for me
    g_nLocalPlayerId = nClientID;

    LocalFree(lpPlayerInfo);

    g_GameRoomWnd.UpdateWnd();
    g_UserListWnd.UpdateWnd();
}

void On_TM_RECV_SVR_STATUS(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    TRACE2(_T("SERVER status: (%d/%d)\n"), wParam, lParam);
    // ToDo: update the server-list tree view
}

void On_TM_RECV_PLAYER_STATE_CHANGE(WPARAM wParam, LPARAM lParam)
{
    int count = (int)wParam;
    LPPLAYER_CHG_DATA pChangeData = (LPPLAYER_CHG_DATA)lParam;

    assert(count > 0);
    assert(pChangeData != NULL);

    PLAYER_STATE    prevState;
    int             prevTable;
    int             prevSeat;

    // �ͻ������Ȼ�ȡ���������ݣ�������Ϣ�����Ŵ���״̬�仯��Ϣ��
    // ��û��ȡ������������Ϣ֮ǰ��û����һ��ID�����յ���ʱ�㲥����Ϣ���򲻴���
    if (GetLocalUserId() == INVALID_USER_ID) {
        LocalFree(pChangeData);
        return;
    }

    for (int i = 0; i < count; i++) {
        STATE_CHANGE_EVENT evtChange = pChangeData[i].event;
        int id = pChangeData[i].playerInfo.id;

        switch (evtChange) {
            case EVT_ENTER:
                TRACE2(_T("EVT_ENTER.[ID: %d, Name: %s]\n"), id, pChangeData[i].playerInfo.name);

                g_PlayerData[id].valid = TRUE;
                g_PlayerData[id].playerInfo.gender = pChangeData[i].playerInfo.gender;
                g_PlayerData[id].playerInfo.imageIdx = pChangeData[i].playerInfo.imageIdx;
                _tcscpy_s(g_PlayerData[id].playerInfo.name, MAX_USER_NAME_LEN, pChangeData[i].playerInfo.name);

                assert(pChangeData[i].playerInfo.state == STATE_IDLE);

                // other fields may use the fault values
                g_PlayerData[id].playerInfo.bAllowLookon = TRUE;
                g_PlayerData[id].playerInfo.state = STATE_IDLE;
                g_PlayerData[id].playerInfo.table = INVALID_TABLE;
                g_PlayerData[id].playerInfo.seat = INVALID_SEAT;

                // add this user to ListCtrl
                g_UserListWnd.InsertUserItem(id);
                break;

            case EVT_LEAVE:
                TRACE2(_T("EVT_LEAVE. [ID: %d, Name: %s]\n"), id, g_PlayerData[id].playerInfo.name);

                prevTable   = g_PlayerData[id].playerInfo.table;
                prevSeat    = g_PlayerData[id].playerInfo.seat;
                prevState   = g_PlayerData[id].playerInfo.state;

                if ((prevTable != INVALID_TABLE) && (prevSeat != INVALID_SEAT)) {
                    if (prevState == STATE_LOOKON) { // look-on leave seat
                        g_GameRoomWnd.LookonLeaveSeat(id, prevTable, prevSeat);
                    } else {
                        g_GameRoomWnd.GamerLeaveSeat(id, prevTable, prevSeat);
                    }
                }

                // reset the entry
                ZeroMemory(&g_PlayerData[id], sizeof(PLAYER_DATA));

                // Fields may set to the default values, or just leave them alone
                g_PlayerData[id].playerInfo.bAllowLookon = TRUE;
                g_PlayerData[id].playerInfo.state = STATE_IDLE;
                g_PlayerData[id].playerInfo.table = INVALID_TABLE;
                g_PlayerData[id].playerInfo.seat = INVALID_SEAT;

                g_UserListWnd.RemoveUserItem(id);
                break;

            case EVT_CHGSTATE:
                if (g_PlayerData[id].playerInfo.bAllowLookon != pChangeData[i].playerInfo.bAllowLookon) {
                    TRACE3(_T("EVT_CHGSTATE. [ID: %d] - [%s] set AllowLookOn [%s]\n"),
                        id, g_PlayerData[id].playerInfo.name, 
                        pChangeData[i].playerInfo.bAllowLookon ? _T("TRUE") : _T("FALSE"));
                }

                g_PlayerData[id].playerInfo.bAllowLookon = pChangeData[i].playerInfo.bAllowLookon;

                if (pChangeData[i].bContainStatistics == TRUE) {
                    g_PlayerData[id].playerInfo.nScore          = pChangeData[i].playerInfo.nScore;
                    g_PlayerData[id].playerInfo.nTotalGames     = pChangeData[i].playerInfo.nTotalGames;
                    g_PlayerData[id].playerInfo.nWinGames       = pChangeData[i].playerInfo.nWinGames;
                    g_PlayerData[id].playerInfo.nRunawayTimes   = pChangeData[i].playerInfo.nRunawayTimes;
                }

                switch (pChangeData[i].playerInfo.state) {
                    case STATE_IDLE:
                        TRACE2(_T("Player [ID: %d] - [%s] ==> IDLE\n"), id, g_PlayerData[id].playerInfo.name);

                        prevTable   = g_PlayerData[id].playerInfo.table;
                        prevSeat    = g_PlayerData[id].playerInfo.seat;
                        prevState   = g_PlayerData[id].playerInfo.state;

                        if ((prevTable != INVALID_TABLE) && (prevSeat != INVALID_SEAT)) {
                            if (prevState == STATE_LOOKON) { // look-on leave seat
                                g_GameRoomWnd.LookonLeaveSeat(id, prevTable, prevSeat);
                            } else {
                                g_GameRoomWnd.GamerLeaveSeat(id, prevTable, prevSeat);
                            }
                        }

                        g_PlayerData[id].playerInfo.table = INVALID_TABLE;
                        g_PlayerData[id].playerInfo.seat = INVALID_SEAT;

                        g_PlayerData[id].playerInfo.state = STATE_IDLE;
                        break;

                    case STATE_SIT:
                        TRACE2(_T("Player [ID: %d] - [%s] ==> SIT\n"), id, g_PlayerData[id].playerInfo.name);

                        g_PlayerData[id].playerInfo.table = pChangeData[i].playerInfo.table;
                        g_PlayerData[id].playerInfo.seat = pChangeData[i].playerInfo.seat;

                        prevState = g_PlayerData[id].playerInfo.state;
                        if (prevState == STATE_IDLE) { // gamer take seat
                            g_GameRoomWnd.GamerTakeSeat(id, g_PlayerData[id].playerInfo.table, g_PlayerData[id].playerInfo.seat);
                            g_GameRoomWnd.SetAllowLookon(g_PlayerData[id].playerInfo.table, g_PlayerData[id].playerInfo.seat, g_PlayerData[id].playerInfo.bAllowLookon);
                        } else if (prevState == STATE_GAMING) { // game over
                            g_GameRoomWnd.GamerGetReady(g_PlayerData[id].playerInfo.table, g_PlayerData[id].playerInfo.seat, FALSE);

                            if (id == GetLocalUserId()) {
                                g_GameMainPage.m_GameMainWnd.GamerCanStart();
                            }
                        }

                        g_PlayerData[id].playerInfo.state = STATE_SIT;
                        break;

                    case STATE_READY:
                        TRACE2(_T("Player [ID: %d] - [%s] ==> READY\n"), id, g_PlayerData[id].playerInfo.name);

                        g_PlayerData[id].playerInfo.table = pChangeData[i].playerInfo.table;
                        g_PlayerData[id].playerInfo.seat = pChangeData[i].playerInfo.seat;

                        prevState = g_PlayerData[id].playerInfo.state;
                        if (prevState == STATE_SIT) {
                            g_GameRoomWnd.GamerGetReady(g_PlayerData[id].playerInfo.table, g_PlayerData[id].playerInfo.seat, TRUE);
                        }

                        g_PlayerData[id].playerInfo.state = STATE_READY;
                        break;

                    case STATE_GAMING:
                        TRACE2(_T("Player [ID: %d] - [%s] ==> GAMING\n"), id, g_PlayerData[id].playerInfo.name);

                        g_PlayerData[id].playerInfo.table = pChangeData[i].playerInfo.table;
                        g_PlayerData[id].playerInfo.seat = pChangeData[i].playerInfo.seat;

                        prevState = g_PlayerData[id].playerInfo.state;
                        if (prevState == STATE_READY) {
                            g_GameRoomWnd.TableGameStarted(g_PlayerData[id].playerInfo.table, TRUE);
                        }

                        g_PlayerData[id].playerInfo.state = STATE_GAMING;
                        break;

                    case STATE_LOOKON:
                        TRACE2(_T("Player [ID: %d] - [%s] ==> LOOKON\n"), id, g_PlayerData[id].playerInfo.name);

                        g_PlayerData[id].playerInfo.table = pChangeData[i].playerInfo.table;
                        g_PlayerData[id].playerInfo.seat = pChangeData[i].playerInfo.seat;
                        g_PlayerData[id].playerInfo.state = STATE_LOOKON;

                        g_GameRoomWnd.LookonTakeSeat(id, g_PlayerData[id].playerInfo.table,
                            g_PlayerData[id].playerInfo.seat);
                        break;
                }
                break;
        }
    }

    LocalFree(pChangeData);

    g_GameRoomWnd.UpdateWnd();
    g_UserListWnd.UpdateWnd();
}

void On_TM_GAMER_TAKE_SEAT(WPARAM wParam, LPARAM lParam)
{
    int table = LOWORD(wParam);
    int seat = HIWORD(wParam);
    int id = (int)lParam;

    int myid = GetLocalUserId();

    if (id != myid) { // ���˽������Ϸ��
        g_GameMainPage.GamerTakeSeat(id, seat);
        return;
    }

    // ��ʼ��Ϸ�����������
    g_GameMainPage.Init(FALSE);
    g_GameMainPage.SetCurentUserSeat(seat);

    // ���Լ������ϣ���Ϊ�ڷ�����1�붨ʱ�㲥֮ǰ���������ݱ��У���Ϸ���п��ܻ�
    // û���Լ�����Ϣ������������������ģ����յ�GamerTakeSeat��LookonTakeSeat��
    // ȷ�Ͽ���������ѡ��λ��Ȼ��֪ͨ�ͻ���������Ϸ�������֪ͨ��ʱ�㲥�̣߳�
    // ĳ�����״̬����ΪSIT
    g_GameMainPage.m_GameMainWnd.GamerTakeSeat(myid, seat);

    // �����������GIFͼƬ
    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        int gamerid = GetGamerId(table, i);
        if (gamerid != INVALID_USER_ID) {
            if (gamerid != myid) { // added myself already
                g_GameMainPage.m_GameMainWnd.GamerTakeSeat(gamerid, i);

                if (g_GameRoomWnd.IsGamerReady(table, i) == TRUE) {
                    g_GameMainPage.m_GameMainWnd.GamerReady(gamerid, i);
                }
            }
        }
    }

    // ���Լ�������
    g_GameMainPage.ListCtrlAddUser(myid);

    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        int nUserId = GetGamerId(table, i);
        if (nUserId != INVALID_USER_ID) {
            if (nUserId != myid) { // added myself already
                g_GameMainPage.ListCtrlAddUser(nUserId);

                int* nLookonIds = GetLookonIdsArrayPtr(table, i);
                for (int j = 0; j < MAX_LOOKON_NUM_PER_SEAT; j++) {
                    if (nLookonIds[j] != INVALID_USER_ID) {
                        g_GameMainPage.ListCtrlAddUser(nLookonIds[j]);
                    }
                }
            }
        }
    }

    g_PageTabWnd.InsertGameTabPage();
    g_PageTabWnd.SetCurTabSel(GAME_PAGE_TAB_INDEX);
    OnPageSelChange(g_hMainWnd, GAME_PAGE_TAB_PARAM);
}

void On_TM_LOOKON_TAKE_SEAT(WPARAM wParam, LPARAM lParam)
{
    int table = LOWORD(wParam);
    int seat = HIWORD(wParam);
    int id = (int)lParam;

    int myid = GetLocalUserId();

    if (id != myid) { // ���˽������Ϸ��
        g_GameMainPage.LookonTakeSeat(id, seat);
        return;
    }

    // �������Ѿ�ȷ�ϸ��Թ��߿��Խ�����Ϸ�������ｫ����Ϣ��ӵ�������Ϣ�ṹ�У�
    // ��Ϊ����Ĳ�����Ҫʹ����Щ��Ϣ���ж��Լ��Ƿ����Թ��ߣ�����������ʱ���ܻ�
    // û�н���Щ��Ч����Ϣ�㲥���ͻ��ˡ��������и������ݡ�
    g_PlayerData[myid].playerInfo.table = table;
    g_PlayerData[myid].playerInfo.seat = seat;
    g_PlayerData[myid].playerInfo.state = STATE_LOOKON;

    // ��ʼ��Ϸ�����������
    g_GameMainPage.Init(TRUE);
    g_GameMainPage.SetCurentUserSeat(seat);

    // �����������GIFͼƬ
    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        int gamerid = GetGamerId(table, i);
        if (gamerid != INVALID_USER_ID) {
            g_GameMainPage.m_GameMainWnd.GamerTakeSeat(gamerid, i);

            if (g_GameRoomWnd.IsGamerReady(table, i) == TRUE) {
                g_GameMainPage.m_GameMainWnd.GamerReady(gamerid, i);
            }
        }
    }

    // ���Լ�������ListCtrl
    g_GameMainPage.ListCtrlAddUser(myid);

    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        int nUserId = GetGamerId(table, i);
        if (nUserId != INVALID_USER_ID) {
            g_GameMainPage.ListCtrlAddUser(nUserId);

            int* nLookonIds = GetLookonIdsArrayPtr(table, i);
            for (int j = 0; j < MAX_LOOKON_NUM_PER_SEAT; j++) {
                if (nLookonIds[j] != INVALID_USER_ID) {
                    if (nLookonIds[j] != myid) { // added myself already
                        g_GameMainPage.ListCtrlAddUser(nLookonIds[j]);
                    }
                }
            }
        }
    }

    g_PageTabWnd.InsertGameTabPage();
    g_PageTabWnd.SetCurTabSel(GAME_PAGE_TAB_INDEX);
    OnPageSelChange(g_hMainWnd, GAME_PAGE_TAB_PARAM);
}

void On_TM_GAMER_LEAVE_SEAT(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    int table = LOWORD(wParam);
    int seat = HIWORD(wParam);

    int nUserId = GetGamerId(table, seat);

    g_GameMainPage.GamerLeaveSeat(nUserId, seat);
}

void On_TM_LOOKON_LEAVE_SEAT(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    int table = LOWORD(wParam);
    int seat = HIWORD(wParam);
    int nUserId = (int)lParam;

    UNREFERENCED_PARAMETER(table);

    g_GameMainPage.LookonLeaveSeat(nUserId, seat);
}

void On_TM_GAMER_READY(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    int table = LOWORD(wParam);
    int seat = HIWORD(wParam);
    int nUserId = GetGamerId(table, seat);

    g_GameMainPage.GamerReady(nUserId, seat);
}

void On_TM_DISTRIBUTE_POKER(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    DISTRIBUTE_POKER* lpDistrPoker = (DISTRIBUTE_POKER*)lParam;
    assert(lpDistrPoker != NULL);

    g_GameMainPage.m_GameMainWnd.DistributePokerCards(0, lpDistrPoker->pokerSeat0, lpDistrPoker->numSeat0);
    g_GameMainPage.m_GameMainWnd.DistributePokerCards(1, lpDistrPoker->pokerSeat1, lpDistrPoker->numSeat1);
    g_GameMainPage.m_GameMainWnd.DistributePokerCards(2, lpDistrPoker->pokerSeat2, lpDistrPoker->numSeat2);

    g_GameMainPage.m_GameMainWnd.SetUnderPokerCards(lpDistrPoker->underCards, UNDER_CARDS_NUM);

    LocalFree(lpDistrPoker);
}

void On_TM_SVR_REQ_VOTE_LORD(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    int seat = LOWORD(wParam);
    int score = HIWORD(wParam);
    g_GameMainPage.m_GameMainWnd.ReqVoteLord(seat, score);
}

void On_TM_SVR_NOTIFY_VOTE_LORD(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    int seat = LOWORD(wParam);
    int score = HIWORD(wParam);
    g_GameMainPage.m_GameMainWnd.GamerVoteLord(seat, score);
}

void On_TM_VOTE_LORD_FINISH(WPARAM wParam, LPARAM lParam)
{
    int seat = LOWORD(wParam);
    int score = HIWORD(wParam);
    BOOL bVote = (BOOL)lParam;

    g_GameMainPage.m_GameMainWnd.VoteLordFinish(bVote, seat, score);
}

void On_TM_SVR_REQ_OUTPUT_CARD(WPARAM wParam, LPARAM lParam)
{
    int seat = LOWORD(wParam);
    int type = HIWORD(wParam);
    int value = LOWORD(lParam);
    int num = HIWORD(lParam);

    POKER_PROPERTY pp;
    pp.num = num;
    pp.type = (POKER_TYPE)type;
    pp.value = value;

    g_GameMainPage.m_GameMainWnd.ReqOutputCard(seat, &pp);
}

void On_TM_SVR_NOTIFY_OUTPUT_CARD(WPARAM wParam, LPARAM lParam)
{
    int seat = LOWORD(wParam);
    int num = HIWORD(wParam);
    OUTPUT_POKER* lpOutputPoker = (OUTPUT_POKER*)lParam;

    if (num <= 0) {
        g_GameMainPage.m_GameMainWnd.OutputCard(seat, num, NULL);
    } else {
        g_GameMainPage.m_GameMainWnd.OutputCard(seat, num, lpOutputPoker->poker);
        LocalFree(lpOutputPoker);
    }
}

void On_TM_SVR_NOTIFY_GAME_OVER(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    GAME_RESULT* lpResult = (GAME_RESULT*)lParam;
    assert(lpResult != NULL);

    g_GameMainPage.m_GameMainWnd.GameOver(lpResult);
    LocalFree(lpResult);
}

void On_TM_GAMER_CHATTING(WPARAM wParam, LPARAM lParam)
{
    int seat = LOWORD(wParam);
    assert(!IS_INVALID_SEAT(seat));

    BOOL bPredefinedMsg = (BOOL)(HIWORD(wParam));

    if (bPredefinedMsg == TRUE) {
        int nMsgIndex = (int)lParam;
        g_GameMainPage.m_GameMainWnd.OutputChatText(seat, nMsgIndex);
    } else {
        TCHAR* lpszChatText = (TCHAR*)lParam;
        if (lpszChatText != NULL) {
            g_GameMainPage.m_GameMainWnd.OutputChatText(seat, lpszChatText);
            LocalFree(lpszChatText);
        }
    }
}

void On_TM_GAMER_DELEGATED(WPARAM wParam, LPARAM lParam)
{
    int seat = (int)wParam;
    BOOL bDelegated = (BOOL)lParam;

    g_GameMainPage.m_GameMainWnd.GamerDelegate(seat, bDelegated);
}

void On_TM_GAMER_DISCONNECTED(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    int seat = (int)wParam;

    g_GameMainPage.m_GameMainWnd.GamerDisconnected(seat);
}

