// DdzServer.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "PokerAlgor.h"
#include "NetProc.h"
#include "GameProc.h"
#include "DdzServer.h"


volatile int g_nCurPokerAlgorithm = 0;

// ��������Ϸϴ���㷨�б�
POKER_ALOGRITHM g_PokerAlgorithm[] = {
    { PA_Randomize1,        _T("Randomize1") },
    { PA_Randomize10,       _T("Randomize10") },
    { PA_RandRemainder,     _T("�����λ") },
    { PA_Test1,             _T("Test1") },
    { PA_Test2,             _T("Test2") },
    { PA_Test3,             _T("Test3") }
};

// ���������߳�ID
DWORD g_nMainThreadId = 0;

HWND g_hMainWnd = NULL;
HWND g_hLogWnd = NULL;
HWND g_hPokerSelStatic = NULL;
HWND g_hPokerSelCombo = NULL;
HWND g_hClearLogButton = NULL;
HWND g_hLogInfoButton = NULL;
HWND g_hLogWarnButton = NULL;
HWND g_hLogErrorButton = NULL;
HWND g_hLogDebugButton = NULL;


HINSTANCE g_hAppInstance;                       // ��ǰʵ��
TCHAR g_szAppTitle[MAX_LOADSTRING];             // �������ı�
TCHAR g_szAppWindowClass[MAX_LOADSTRING];       // ����������

extern DWORD    g_dwServerIP;
extern USHORT   g_nServerPort;


// ������
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    // �����߳�ID�����������Ա������߳̿������̷߳�����Ϣ
    g_nMainThreadId = GetCurrentThreadId();

	// ִ��Ӧ�ó����ʼ��:
    if (!InitInstance(hInstance, nCmdShow)) {
		return -1;
	}

    // ��ʼ����������������־����
    if (!StartServerLog()) {
        DestroyWindow(g_hMainWnd);
        return -1;
    }

    // ������������
    if (!StartNetService()) {
        StopServerLog();
        DestroyWindow(g_hMainWnd);
        return -1;
    }

    // ������Ϸ�����߳�
    if (!BeginGameThreads()) {
        StopNetService();
        StopServerLog();
        DestroyWindow(g_hMainWnd);
        return -1;
    }

    // Show main window when all tasks succeed. 2010-05
    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);

    // ������б�Ҫ�ĳ�ʼ�������������Ѿ���ʼ����
    WriteLog(LOG_INFO, _T("�������������������� [%d.%d.%d.%d: %d]����������ʼ����..."),
        (ntohl(g_dwServerIP) >> 24) & 0xFF,
        (ntohl(g_dwServerIP) >> 16) & 0xFF,
        (ntohl(g_dwServerIP) >> 8) & 0xFF,
        (ntohl(g_dwServerIP) >> 0) & 0xFF,
        (ntohs(g_nServerPort)));

	// ����Ϣѭ��
    int ret;
    MSG msg;
    HACCEL hAccelTable;
    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DDZSERVER));

	while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (ret == -1) { continue; }

        if (msg.message == TM_CLIENT_DISCONNECT) {
            CloseConnectionThreadHandle((HANDLE)msg.wParam);
            continue;
        }

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

    // �˳���Ϸ�����߳�
    EndGameThreads();

    // �����������ӣ����ȴ����������߳��˳�
    StopNetService();

    // �رշ�������־����
    StopServerLog();

	return (int)msg.wParam;
}

//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DDZSERVER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DDZSERVER);
	wcex.lpszClassName	= g_szAppWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    UNREFERENCED_PARAMETER(nCmdShow);

    HWND hWnd;

    // ��ʵ������洢��ȫ�ֱ�����
    g_hAppInstance = hInstance;

    // ��ʼ��ȫ���ַ���
    LoadString(hInstance, IDS_APP_TITLE, g_szAppTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_DDZSERVER, g_szAppWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // ����������
    hWnd = CreateWindow(g_szAppWindowClass, g_szAppTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (hWnd == NULL) {
        return FALSE;
    }

    // ���������ھ��
    g_hMainWnd = hWnd;

    // �����Ӵ��ڿؼ�
    CreateChildWindows();

    // Show main window until all tasks succeed. 2010-05
    //ShowWindow(hWnd, nCmdShow);
    //UpdateWindow(hWnd);
    return TRUE;
}

//
//  ����: MainWndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int nID, nEvent;
    PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {
	case WM_COMMAND:
        nID = LOWORD(wParam);
        nEvent = HIWORD(wParam);

        if ((HWND)lParam == g_hPokerSelCombo) {
            DoPokerSelCombo((HWND)lParam, nEvent);
            break;
        }

		// �����˵�ѡ��:
		switch (nID) {
		case IDM_ABOUT:
			DialogBox(g_hAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

        case IDC_BTN_LOGINFO:
        case IDC_BTN_LOGWARN:
        case IDC_BTN_LOGERROR:
        case IDC_BTN_LOGDEBUG:
            OnLogOptions((HWND)lParam, nID, nEvent);
            break;

        case IDC_BTN_CLEARLOG:
            ClearLog();
            break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            EnumChildWindows(hWnd, EnumChildWndProc, lParam);
        } else {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
        PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
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

// �����Ӵ���
BOOL CreateChildWindows(void)
{
    // ��ʼ��ϵͳͨ�ÿؼ�
    INITCOMMONCONTROLSEX ccex;
    ccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    ccex.dwICC = ICC_STANDARD_CLASSES | ICC_USEREX_CLASSES;
    InitCommonControlsEx(&ccex);

    // RichEdit
    HINSTANCE hRichED = LoadLibrary(_T("riched20.dll"));
    if (hRichED == NULL) {
        return FALSE;
    }
    g_hLogWnd = CreateWindow(RICHEDIT_CLASS, NULL,
        WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_DISABLENOSCROLL | ES_SUNKEN | ES_MULTILINE | ES_READONLY,
        0, 0, 0, 0,
        g_hMainWnd, (HMENU)IDC_RICHED_LOGWND, g_hAppInstance, NULL);
    if (g_hLogWnd == NULL) {
        goto CREATE_CHILD_WND_FAIL;
    }

    g_hPokerSelStatic = CreateWindow(_T("STATIC"), _T("ϴ���㷨: "),
        WS_CHILD | SS_RIGHT,
        0, 0, 0, 0,
        g_hMainWnd, (HMENU)IDC_STATIC_POKERSEL, g_hAppInstance, NULL);
    if (g_hPokerSelStatic == NULL) {
        goto CREATE_CHILD_WND_FAIL;
    }

    g_hLogWarnButton = CreateWindow(_T("BUTTON"), _T("WARNING"),
        WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 0, 0,
        g_hMainWnd, (HMENU)IDC_BTN_LOGWARN, g_hAppInstance, NULL);
    if (g_hLogWarnButton == NULL) {
        goto CREATE_CHILD_WND_FAIL;
    }

    g_hLogErrorButton = CreateWindow(_T("BUTTON"), _T("ERROR"),
        WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 0, 0,
        g_hMainWnd, (HMENU)IDC_BTN_LOGERROR, g_hAppInstance, NULL);
    if (g_hLogErrorButton == NULL) {
        goto CREATE_CHILD_WND_FAIL;
    }

    g_hLogDebugButton = CreateWindow(_T("BUTTON"), _T("DEBUG"),
        WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 0, 0,
        g_hMainWnd, (HMENU)IDC_BTN_LOGDEBUG, g_hAppInstance, NULL);
    if (g_hLogDebugButton == NULL) {
        goto CREATE_CHILD_WND_FAIL;
    }

    g_hLogInfoButton = CreateWindow(_T("BUTTON"), _T("INFO"),
        WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 0, 0,
        g_hMainWnd, (HMENU)IDC_BTN_LOGINFO, g_hAppInstance, NULL);
    if (g_hLogInfoButton == NULL) {
        goto CREATE_CHILD_WND_FAIL;
    }

    g_hClearLogButton = CreateWindow(_T("BUTTON"), _T("�����־"),
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0,
        g_hMainWnd, (HMENU)IDC_BTN_CLEARLOG, g_hAppInstance, NULL);
    if (g_hClearLogButton == NULL) {
        goto CREATE_CHILD_WND_FAIL;
    }

    //
    // ComboBoxEx ����֧��ͼ�꣬������ʡȥ�Ի��б�ͼ�ꡣ
    // �� ComboBoxEx ��֧�� CBS_DISABLENOSCROLL ��񣬼�ʼ����ʾ��ֱ�����������ң�
    // ѡ�����е��б���ʱ����������ֻ���ı���ȶ����������б���εĿ�ȡ�
    // ���⣬����������һ�����⣬�� WM_SIZE ����Ϣ����
    //
//#define _USE_COMBO_EX_CTRL
#ifdef _USE_COMBO_EX_CTRL
    g_hPokerSelCombo = CreateWindowEx(0, WC_COMBOBOXEX, NULL,
        WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL,
        0, 0, 0, 0,
        g_hMainWnd, (HMENU)IDC_COMBO_POKERSEL, g_hAppInstance, NULL);
#else
    g_hPokerSelCombo = CreateWindow(WC_COMBOBOX, NULL,
        WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL,
        0, 0, 0, 0,
        g_hMainWnd, (HMENU)IDC_COMBO_POKERSEL, g_hAppInstance, NULL);
#endif
    if (g_hPokerSelCombo == NULL) {
        goto CREATE_CHILD_WND_FAIL;
    }

    for (int i = 0; i < sizeof(g_PokerAlgorithm) / sizeof(g_PokerAlgorithm[0]); i++) {
#ifdef _USE_COMBO_EX_CTRL
        COMBOBOXEXITEM cbei;
        cbei.mask       = CBEIF_TEXT;
        cbei.iItem      = i;
        cbei.pszText    = g_PokerAlgorithm[i].name;
        cbei.cchTextMax = (int)_tcslen(g_PokerAlgorithm[i].name);
        SendMessage(g_hPokerSelCombo, CBEM_INSERTITEM, 0, (LPARAM)&cbei);
#else
        SendMessage(g_hPokerSelCombo, CB_ADDSTRING, i, (LPARAM)g_PokerAlgorithm[i].name);
#endif
    }

    // Ĭ��ʹ�õ�0���㷨
    SendMessage(g_hPokerSelCombo, CB_SETCURSEL, 0, 0);

    // Ĭ�Ͽ���������־ѡ��μ�����s_logCurLevel
    SendMessage(g_hLogWarnButton, BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(g_hLogErrorButton, BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(g_hLogDebugButton, BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(g_hLogInfoButton, BM_SETCHECK, BST_CHECKED, 0);

    // �������壬����ϵͳĬ������
    SendMessage(g_hLogWnd, WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), (LPARAM)FALSE);

    SendMessage(g_hPokerSelStatic, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    SendMessage(g_hPokerSelCombo, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    SendMessage(g_hLogWarnButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    SendMessage(g_hLogErrorButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    SendMessage(g_hLogDebugButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    SendMessage(g_hLogInfoButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    SendMessage(g_hClearLogButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)FALSE);
    return TRUE;

CREATE_CHILD_WND_FAIL:
#define DESTROY_CHILD_WND(_wnd) if (_wnd != NULL) { DestroyWindow(_wnd); _wnd = NULL; }
    DESTROY_CHILD_WND(g_hLogWnd);
    DESTROY_CHILD_WND(g_hPokerSelStatic);
    DESTROY_CHILD_WND(g_hLogWarnButton);
    DESTROY_CHILD_WND(g_hLogErrorButton);
    DESTROY_CHILD_WND(g_hLogDebugButton);
    DESTROY_CHILD_WND(g_hLogInfoButton);
    DESTROY_CHILD_WND(g_hClearLogButton);
    DESTROY_CHILD_WND(g_hPokerSelCombo);
    return FALSE;
}

// ����COMMOBOX�¼�
void DoPokerSelCombo(HWND hWnd, int notify)
{
    int i = 0;
    int len = 0;
    TCHAR szText[64] = { 0 };

    switch (notify) {
    case CBN_SELCHANGE:
        i = (int)SendMessage(hWnd, CB_GETCURSEL, 0, 0);
        len = (int)SendMessage(hWnd, CB_GETLBTEXT, i, (LPARAM)szText);
        if (len != CB_ERR) {
            if (g_nCurPokerAlgorithm != i) {
                g_nCurPokerAlgorithm = i;
                WriteLog(LOG_INFO, _T("������ϴ���㷨�Ѿ�����Ϊ [ %s ]�������´η���ʱ��Ч"), szText);
            }
        }
        break;
    }
}

// �����Ӵ��ڳߴ�
BOOL CALLBACK EnumChildWndProc(HWND hWnd, LPARAM lParam)
{
    int id = GetWindowLong(hWnd, GWL_ID);
    int width = LOWORD(lParam);
    int height = HIWORD(lParam);
    int x = 0;
    int y = 0;
    int cx = 0;
    int cy = 0;

#define MY_STARTX               0
#define MY_STARTY               4
#define MY_CHECKBOX_WIDTH       100
#define MY_BTN_HEIGHT           24
#define MY_BTN_WIDTH            100

    if ((width < MY_BTN_WIDTH) || (height < MY_BTN_HEIGHT)) {
        return FALSE;
    }

    if (id == IDC_STATIC_POKERSEL) {
        x = MY_STARTX;
        y = MY_STARTY + 4;
        cx = 70;
        cy = MY_BTN_HEIGHT - 4;

#ifdef _USE_COMBO_EX_CTRL
    } else if (hWnd == (HWND)SendMessage(g_hPokerSelCombo, CBEM_GETCOMBOCONTROL, 0, 0)) {
        //
        // ����COMBOBOXEX�ؼ������е��Ӵ���COMBOBOX
        //
        // ���ʹ�ÿؼ�ComboBoxEx����ÿؼ��᷵������WM_SIZE��Ϣ�������ڣ�
        // һ����ComboBoxEx���صģ�һ����ComboBox���صġ�ΪʲôҪ���������أ�
        //
        // ��MoveWindow��SetWindowPosʹComboBoxEx�ؼ��ı�λ�û�ߴ�ʱ����Ҫ��
        // ComboBoxEx��ComboBox�ؼ��������µľ��Σ����⣬ComboBox�ľ���������
        // ������丸����ComboBoxEx��
        //
        //�����ʹ��ComboBox�ؼ����򲻻��д����⣩
        //
        x = 0;
        y = 0;
        cx = 120;
        cy = 160;
#endif

    } else if (id == IDC_COMBO_POKERSEL) {
        x = 70;
        y = MY_STARTY;
        cx = 120;
        cy = 160;
    } else if (id == IDC_BTN_LOGWARN) {
        x = 240;
        y = MY_STARTY;
        cx = MY_CHECKBOX_WIDTH;
        cy = MY_BTN_HEIGHT;
    } else if (id == IDC_BTN_LOGERROR) {
        x = 340;
        y = MY_STARTY;
        cx = MY_CHECKBOX_WIDTH;
        cy = MY_BTN_HEIGHT;
    } else if (id == IDC_BTN_LOGDEBUG) {
        x = 440;
        y = MY_STARTY;
        cx = MY_CHECKBOX_WIDTH;
        cy = MY_BTN_HEIGHT;
    } else if (id == IDC_BTN_LOGINFO) {
        x = 540;
        y = MY_STARTY;
        cx = MY_CHECKBOX_WIDTH;
        cy = MY_BTN_HEIGHT;
    } else if (id == IDC_BTN_CLEARLOG) {
        x = width - (MY_BTN_WIDTH + 10);
        y = MY_STARTY;
        cx = MY_BTN_WIDTH;
        cy = MY_BTN_HEIGHT;
    } else if (id == IDC_RICHED_LOGWND) {
        x = MY_STARTX;
        y = MY_STARTY + MY_BTN_HEIGHT + MY_STARTY;
        cx = width;
        cy = height - (MY_STARTY + MY_BTN_HEIGHT + MY_STARTY);
    } else {
        return TRUE;
    }

    MoveWindow(hWnd, x, y, cx, cy, TRUE);
    ShowWindow(hWnd, SW_SHOW);
    return TRUE;
}

// ����ť֪ͨ
void OnLogOptions(HWND hWnd, int id, int nEvent)
{
    if (hWnd == NULL) { return; }
    if (nEvent != BN_CLICKED) { return; }

    BOOL bChecked = FALSE;
    bChecked = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE;

    switch (id) {
        case IDC_BTN_LOGINFO:
            LogEnableInfo(bChecked);
            break;
        case IDC_BTN_LOGWARN:
            LogEnableWarn(bChecked);
            break;
        case IDC_BTN_LOGERROR:
            LogEnableError(bChecked);
            break;
        case IDC_BTN_LOGDEBUG:
            LogEnableDebug(bChecked);
            break;
    }
}

