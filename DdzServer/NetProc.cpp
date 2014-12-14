//
// NetProc.cpp
//
//  ����������Ϣ�����ܿͻ��˵�����
//
// Modification:
//
//  2009-06-05  Change the calling point of DeallocID()
//      The ID is used to identify a client or a gamer. It's critical to handle 
//      ID resource correctly for the server since it is used to synchronize the
//      data structure of server with client's and do many other operations.
//      Here, there's an issue, say, one client connects to the server and been 
//      allocated an ID with 0, and then he leaves. So, the server will reclaim
//      the ID and tell the OnTimeBroadcast thread that someone's left with 
//      EVT_LEAVE flag. Since the broadcast thread will be scheduled each second. 
//      While, before that happens, another client logons the server, and with 
//      ID 0 assigned. So this client might receive a message of [ID:0 EVT_LEAVE] 
//      from OnTimeBroadcast sometime and will receive a message [ID:0 EVT_ENTER] 
//      about himself at the next second. This may not be harmful for the client,
//      but i don't think it's something good. 
//      From this point, i decide to make a change that there will only one place
//      to call DeallocID(), that's within OnTimeBroadcast thread when it processes
//      a node with EVT_LEAVE flagged.
//
// 2009-06-12 Added bConnLost member to CONNECTION_INFO, and closed socket when DeallocID
//      For instance, three gamers were in game, and two of them disconnected 
//      unexpectedly. Since the game had started, the server would output cards
//      for the disconnected gamer. Before the game was over, server should not 
//      remove the info of these disconnected players, or label them unusable. 
//      Instead, it should keep them valid until the game was over. Otherwise, 
//      other new login clients would see that one guy was just gaming with 
//      nobody in the game room.
//
// 2010-05-22 Added IP address selection dialog on application startup
//
//
#include "stdafx.h"
#include <Wspiapi.h> // BACKWARD-COMPATIBILITY: for `addrinfo' be used on win2k
#include "resource.h"
#include "Crc.h"
#include "NetProc.h"
#include "GameProc.h"

extern DWORD g_nGameThreadIds[];
extern DWORD g_nMainThreadId;

// ���������׽���
SOCKET g_ServerSocket = INVALID_SOCKET;
volatile BOOL g_bServerWillShutdown = FALSE;

DWORD   g_dwServerIP = 0;   // ������ʹ�õ�IP��������
USHORT  g_nServerPort = 0;  // ������ʹ�õĶ˿ڣ�������

// ��ǰ��������
volatile int g_nCurConnectionNum = 0;

// ID��Դ
CONNECTION_ID_RES g_ConnectionIdRes = { 0 };
CRITICAL_SECTION g_csConnectionNum = { 0 };

// ��Ϸ����Ϣ
GAME_TABLE_INFO g_GameTableInfo[GAME_TABLE_NUM] = { 0 };

// �ͻ������ӣ���ң������Ϣ
CONNECTION_INFO g_ConnectionInfo[MAX_CONNECTION_NUM] = { 0 };

// ��¼��ҽ�����뿪���䣬Ȼ��ʱ����Щ��Ϣ�㲥���͸������������
// �����Զ��з�ʽ�������������β������Ӳ�����������ͷ��ʼ��ȡ��ɾ������
USER_STATE_CHANGE_LIST g_UserStateChangeList = { 0 };
CRITICAL_SECTION g_csUserStateChangeList = { 0 };

HANDLE g_hConnectionThread[MAX_CONNECTION_NUM] = { 0 };
HANDLE g_hListenThread = NULL;
HANDLE g_hOnTimeBroadcastThread = NULL;
HANDLE g_mutexOnTimeBroadcast = NULL;

// �ͻ��������Ͽ����Ӷ��Ƿ����������Ͽ�����ʱ���ر������߳̾��
void CloseConnectionThreadHandle(HANDLE hThread)
{
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
}

// ֹͣ�������
void StopNetService(void)
{
    g_bServerWillShutdown = TRUE;

    shutdown(g_ServerSocket, SD_BOTH);
    closesocket(g_ServerSocket);

    // �˳��ȴ������߳�
    WaitForSingleObject(g_hListenThread, INFINITE);
    CloseHandle(g_hListenThread);

    // �ͷ�Mutex���˳���ʱ�㲥�߳�
    ReleaseMutex(g_mutexOnTimeBroadcast);
    WaitForSingleObject(g_mutexOnTimeBroadcast, INFINITE);
    CloseHandle(g_mutexOnTimeBroadcast);
    CloseHandle(g_hOnTimeBroadcastThread);

    // �ر�SOCKET���˳������߳�
    for (int id = 0; id < MAX_CONNECTION_NUM; id++) {
        if (M_CONNECT_SOCK(id) != INVALID_SOCKET) {
            shutdown(M_CONNECT_SOCK(id), SD_BOTH);
            closesocket(M_CONNECT_SOCK(id));
        }

        if (g_hConnectionThread[id] != NULL) {
            WaitForSingleObject(g_hConnectionThread[id], INFINITE);
            CloseHandle(g_hConnectionThread[id]);
        }
    }

    // ���Winsockϵͳ��Դ
    WSACleanup();

    // �ͷ���Դ
    ReleaseNetServiceRes();

    // �������������������ʹ�õĶ˿ں�
    WriteServerPortToIni();
}

// ��ʼ��WinSock�����������������ӵȴ��߳�
BOOL StartNetService(void)
{
    DWORD nThreadID = 0;
    if (InitNetServiceRes() == FALSE) {
        return FALSE;
    }

    if (InitWinSocket() == FALSE) {
        ReleaseNetServiceRes();
        return FALSE;
    }

    // �����ȴ������߳�
    g_hListenThread = CreateThread(NULL, 0,
        (LPTHREAD_START_ROUTINE)WaitForConnectionThreadProc, (LPVOID)NULL, 0, &nThreadID);

    // ������ʱ�㲥�߳�
    g_hOnTimeBroadcastThread = CreateThread(NULL, 0, 
        (LPTHREAD_START_ROUTINE)OnTimeBroadcastThreadProc, (LPVOID)NULL, 0, &nThreadID);

    return TRUE;
}

// �ͷ����������ռ�õ������Դ
static void ReleaseNetServiceRes(void)
{
    EnterCriticalSection(&g_csUserStateChangeList);
    if (g_UserStateChangeList.lpFirstNode != NULL) {
        USER_STATE_CHANGE* pNode = NULL;
        USER_STATE_CHANGE* pTemp = NULL;

        // �������
        pNode = g_UserStateChangeList.lpFirstNode;
        while (pNode != NULL) {
            pTemp = pNode;
            pNode = pNode->next;
            LocalFree(pTemp);
        }
        g_UserStateChangeList.lpFirstNode = NULL;
        g_UserStateChangeList.lpLastNode = NULL;
    }
    LeaveCriticalSection(&g_csUserStateChangeList);

    DeleteCriticalSection(&g_csConnectionNum);
    DeleteCriticalSection(&g_csUserStateChangeList);
}

// ��ʼ���������������ݽṹ
static BOOL InitNetServiceRes(void)
{
    g_bServerWillShutdown = FALSE;
    g_nCurConnectionNum = 0;

    // �������ݽṹ��λΪ0
    ZeroMemory(&g_ConnectionIdRes, sizeof(CONNECTION_ID_RES));
    ZeroMemory(&g_UserStateChangeList, sizeof(USER_STATE_CHANGE_LIST));
    ZeroMemory(&g_ConnectionInfo, sizeof(CONNECTION_INFO) * MAX_CONNECTION_NUM);
    ZeroMemory(&g_GameTableInfo, sizeof(GAME_TABLE_INFO) * GAME_TABLE_NUM);

    // ��ʼ��������ϢĬ��ֵ
    for (int id = 0; id < MAX_CONNECTION_NUM; id++) {
        M_CONNECT_SOCK(id) = INVALID_SOCKET;
        M_CONNECT_TABLE(id) = INVALID_TABLE;
        M_CONNECT_SEAT(id) = INVALID_SEAT;
        M_CONNECT_STATE(id) = STATE_IDLE;
        M_CONNECT_ALLOW_LOOKON(id) = TRUE;
    }

    // ��ʼ����Ϸ����ϢĬ��ֵ
    for (int table = 0; table < GAME_TABLE_NUM; table++) {
        for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
            M_TABLE_PLAYER_ID(table, seat) = INVALID_USER_ID;

            for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
                M_TABLE_LOOKON_ID(table, seat)[i] = INVALID_USER_ID;
            }
        }
    }

    // ����Ϊ��ռ�õ�Mutex����Ϊ��ʱ�㲥�߳��ṩ��ʱ���Ļ���
    g_mutexOnTimeBroadcast = CreateMutex(NULL, TRUE, NULL);
    if (g_mutexOnTimeBroadcast == NULL) {
        return FALSE;
    }

    InitializeCriticalSection(&g_csConnectionNum);
    InitializeCriticalSection(&g_csUserStateChangeList);

    return TRUE;
}

// ON Init IP Selection Dialog
INT_PTR InitIpSelDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    HWND hIpList = NULL;

    // ���Ի�����������ͼ��
    HICON hicon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DDZSERVER));
    SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hicon);

    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
    hIpList = GetDlgItem(hDlg, IDC_LIST_ADAPTER);
    ListView_SetExtendedListViewStyle(hIpList, LVS_EX_FULLROWSELECT);

    // insert column for list-view ctrl 
    struct _lv_colum_data {
        LPTSTR  label;
        int     width;
        int     format;
    } lvcolumn[] = {
        { _T("IP��ַ"), 110, LVCFMT_LEFT },
        { _T("��������"), 110, LVCFMT_LEFT },
        { _T("����"), 160, LVCFMT_LEFT },
    };

    for (int i = 0; i < (sizeof(lvcolumn) / sizeof(lvcolumn[0])); i++) {
        LVCOLUMN lvc = { 0 };
        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
        lvc.pszText = lvcolumn[i].label;
        lvc.cchTextMax = _tcslen(lvcolumn[i].label);
        lvc.cx = lvcolumn[i].width;
        lvc.fmt = lvcolumn[i].format;
        ListView_InsertColumn(hIpList, i, &lvc);
    }

    // init data for list-view ctrl
    IP_ADAPTER_INFO* pAdapterInfo = NULL;
    ULONG ulOutBufLen = 0;

    // query the size needed
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_BUFFER_OVERFLOW) {
        return (INT_PTR)EndDialog(hDlg, IDCANCEL);
    }

    //assert(ulOutBufLen > 0);
    pAdapterInfo = (IP_ADAPTER_INFO*)GlobalAlloc(GPTR, ulOutBufLen); 
    if (pAdapterInfo == NULL) {
        return (INT_PTR)EndDialog(hDlg, IDCANCEL);
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != NO_ERROR) {
        GlobalFree(pAdapterInfo);
        return (INT_PTR)EndDialog(hDlg, IDCANCEL);
    }

    IP_ADAPTER_INFO* pAdapter = NULL;
    IP_ADDR_STRING* pIpAddrStr = NULL;
    int nItemIndex = 0;

    // use ANSI type string functions for easy operation
    for (pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next) { // iterate all adapters
        for (pIpAddrStr = &pAdapter->IpAddressList; pIpAddrStr; pIpAddrStr = pIpAddrStr->Next) { //iterate all ip within the adapter
            // get network-bit-order IP
            DWORD dwIP = inet_addr(pIpAddrStr->IpAddress.String);
            if (dwIP == 0) {
                continue;
            }

            // insert item
            LVITEMA lvi = { 0 };
            lvi.iItem = nItemIndex;
            SendMessageA(hIpList, LVM_INSERTITEMA, 0, (LPARAM)&lvi);

            // set item text
            lvi.mask = LVIF_TEXT;

            lvi.iSubItem = 0;
            lvi.pszText = pIpAddrStr->IpAddress.String;
            SendMessageA(hIpList, LVM_SETITEMTEXTA, nItemIndex, (LPARAM)&lvi);

            lvi.iSubItem = 1;
            lvi.pszText = pIpAddrStr->IpMask.String;
            SendMessageA(hIpList, LVM_SETITEMTEXTA, nItemIndex, (LPARAM)&lvi);

            lvi.iSubItem = 2;
            lvi.pszText = pAdapter->Description;
            SendMessageA(hIpList, LVM_SETITEMTEXTA, nItemIndex, (LPARAM)&lvi);

            nItemIndex++;
        }
    }

    GlobalFree(pAdapterInfo);
    EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
    return (INT_PTR)FALSE;
}

INT_PTR FinishIpSel(HWND hDlg, int iItem)
{
    // use ANSI type functions, so we don't need to convert ip string.
    IP_ADDRESS_STRING ipstr = { 0 };
    LVITEMA lvi = { 0 };
    lvi.iItem = iItem;
    lvi.iSubItem = 0; // ip addr column
    lvi.pszText = &ipstr.String[0];
    lvi.cchTextMax = sizeof(ipstr.String) / sizeof(ipstr.String[0]);
    SendMessageA(GetDlgItem(hDlg, IDC_LIST_ADAPTER), LVM_GETITEMTEXTA,
        (WPARAM)iItem, (LPARAM)&lvi);

    DWORD dwIpSelected = inet_addr(ipstr.String);
    if (dwIpSelected > 0) {
        DWORD* lpRetIp = (DWORD*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
        if (lpRetIp) {
            *lpRetIp = dwIpSelected;
            return EndDialog(hDlg, IDOK);
        }
    }

    return (INT_PTR)FALSE;
}

INT_PTR IpListNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    LPNMHDR pNotifyHdr = (LPNMHDR)lParam;
    if (pNotifyHdr->idFrom != IDC_LIST_ADAPTER) {
        return (INT_PTR)FALSE;
    }

    LPNMITEMACTIVATE pNMItemSel = (LPNMITEMACTIVATE)lParam;
    switch (pNotifyHdr->code) {
    case NM_CLICK:
        if (pNMItemSel->iItem >= 0) {
            EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
        } else {
            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
        }
        break;

    case NM_DBLCLK:
        if (pNMItemSel->iItem >= 0) {
            return FinishIpSel(hDlg, pNMItemSel->iItem);
        }
        break;
    }

    return (INT_PTR)FALSE;
}

// ������IP��ַѡ��Ի������Ϣ�������
INT_PTR CALLBACK IpSelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hIpList = NULL;
    int nItemCount = 0;
    int nItemSel = -1;
    DWORD dwState = 0;

    switch (message) {
    case WM_INITDIALOG:
        return InitIpSelDialog(hDlg, wParam, lParam);

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL) {
            return (INT_PTR)EndDialog(hDlg, IDCANCEL);
        }

        if (LOWORD(wParam) == IDOK) {
            hIpList = GetDlgItem(hDlg, IDC_LIST_ADAPTER);
            nItemCount = SendMessage(hIpList, LVM_GETITEMCOUNT, 0, 0);
            for (int i = 0; i < nItemCount; i++) {
                dwState = SendMessage(hIpList, LVM_GETITEMSTATE, i, LVIS_SELECTED);
                if (dwState & LVIS_SELECTED) {
                    nItemSel = i;
                    break;
                }
            }

            if (nItemSel >= 0) {
                return FinishIpSel(hDlg, nItemSel);
            }
        }
        break;

    case WM_NOTIFY:
        return IpListNotify(hDlg, wParam, lParam);

    case WM_DESTROY:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
        break;
    }

    return (INT_PTR)FALSE;
}

// ���÷�����ʹ�õ�IP
BOOL InitServerIP(void)
{
    DWORD dwIpArray[8] = { 0 };
    int nIpCount = 0;

    GetHostIP(dwIpArray, sizeof(dwIpArray) / sizeof(dwIpArray[0]), &nIpCount);
    if (nIpCount <= 0) {
        return FALSE;
    }

    // ʹ�������ϵ�һ��IP
    g_dwServerIP = dwIpArray[0];

    // Shining added ip addresses selection dialog, 2010-05
    if (nIpCount > 1) {
        extern HINSTANCE g_hAppInstance;
        DWORD dwIpSelected = 0;
        int ret = DialogBoxParam(g_hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_IP_SEL),
            NULL, IpSelProc, (LPARAM)&dwIpSelected);
        if (ret != IDOK) {
            return FALSE;
        }

        g_dwServerIP = dwIpSelected;
    }

    return TRUE;
}

// ���÷�����ʹ�õĶ˿�
BOOL InitServerPort(void)
{
    TCHAR szIniFile[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szIniFile, MAX_PATH);
    PathRemoveFileSpec(szIniFile);
    _tcscat_s(szIniFile, MAX_PATH, _T("\\"));
    _tcscat_s(szIniFile, MAX_PATH, SERVER_CONF_FILE);

    UINT port = GetPrivateProfileInt(SVR_INI_SEC_SERVER, SVR_INI_KEY_PORT, DDZSERVER_PORT, szIniFile);

    g_nServerPort = htons((USHORT)port);
    return TRUE;
}

// �������������ʹ�õĶ˿ں�
void WriteServerPortToIni(void)
{
    TCHAR szIniFile[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szIniFile, MAX_PATH);
    PathRemoveFileSpec(szIniFile);
    _tcscat_s(szIniFile, MAX_PATH, _T("\\"));
    _tcscat_s(szIniFile, MAX_PATH, SERVER_CONF_FILE);

    TCHAR szPort[16] = { 0 };
    USHORT port = ntohs(g_nServerPort);

    _stprintf_s(szPort, sizeof(szPort) / sizeof(szPort[0]), _T("%d"), port);

    WritePrivateProfileString(SVR_INI_SEC_SERVER, SVR_INI_KEY_PORT, szPort, szIniFile);
}

// ��ʼ��Winsocket
static BOOL InitWinSocket(void)
{
    WORD wVersion;
    WSADATA wsaData;

    wVersion = MAKEWORD(2, 2);
    if (WSAStartup(wVersion, &wsaData) != NO_ERROR) {
        return FALSE;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        return FALSE; 
    }

    g_ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_ServerSocket == INVALID_SOCKET) {
        WSACleanup();
        return FALSE; 
    }

    if (!InitServerIP() || !InitServerPort()) {
        WSACleanup();
        return FALSE;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = g_dwServerIP;
    service.sin_port = g_nServerPort;

    if (bind(g_ServerSocket, (SOCKADDR*)&service, sizeof(SOCKADDR)) != NO_ERROR) {
        closesocket(g_ServerSocket);
        WSACleanup();
        return FALSE;
    }

    if (listen(g_ServerSocket, 1) != NO_ERROR) {
        closesocket(g_ServerSocket);
        WSACleanup();
        return FALSE;
    }

    return TRUE;
}

// ��ȡ����IP��ַ��32-bit�����������򣩣����ز���count ��ʾ�����м���IP
void GetHostIP(DWORD ipArray[], int num, int* count)
{
    int ret = SOCKET_ERROR;
    char nodename[64] = { 0 };
    char servname[] = "2048";
    struct addrinfo aiHints = { 0 };
    struct addrinfo* ai = NULL;
    struct addrinfo* aiLists = NULL;
    struct sockaddr_in* skaddr = NULL;

    // ��ʼ�����ص�IP����
    int n = 0;
    *count = 0;

    aiHints.ai_family = AF_INET;
    aiHints.ai_socktype = SOCK_STREAM;
    aiHints.ai_protocol = IPPROTO_TCP;

    ret = gethostname(nodename, sizeof(nodename) / sizeof(nodename[0]));
    if (ret != NO_ERROR) {
        return;
    }

    ret = getaddrinfo(nodename, servname, &aiHints, &aiLists);
    if (ret != NO_ERROR) {
        return;
    }

    for (ai = aiLists; ai != NULL; ai = ai->ai_next) {
        if (ai->ai_family != AF_INET) {
            continue;
        }

        skaddr = (struct sockaddr_in*)ai->ai_addr;
        if (skaddr != NULL) {
            ipArray[n++] = skaddr->sin_addr.s_addr;
            if (n >= num) {
                break;
            }
        }
    }

    if (aiLists != NULL) {
        freeaddrinfo(aiLists);
        aiLists = NULL;
    }

    *count = n;
}

//
// Ϊÿ�����ӷ���һ����ΨһID������ȷ����������û�����������ֵ
//
// �����������߳̽��ܿͻ�������ʱ������øú���Ϊ�ͻ��˷���ID
// ���ʹ�����Դg_ConnectionIdRes������ʱ��Ҫ��������
//
// ���������жϷ���ֵ�Ƿ�ΪINVALID_USER_ID�����ǣ����ʾ����IDʧ��
//
int AllocID(void)
{
    static int ELEM_BITS = sizeof(g_ConnectionIdRes.idResource[0]) * 8;

    for (int id = 0; id < MAX_CONNECTION_NUM; id++) {
        if ((g_ConnectionIdRes.idResource[id / ELEM_BITS] & (1 << (id % ELEM_BITS))) == 0) {
            g_ConnectionIdRes.idResource[id / ELEM_BITS] |= (1 << (id % ELEM_BITS));
            return id;
        }
    }

    WriteLog(LOG_ERROR, _T("ID��Դ�ľ� [Check your program!]"));
    return INVALID_USER_ID;
}

//
// ����ID��Դ
// 
// �����߳��˳�ʱ������Ϸ�����߳̽�����Ϸʱ������øú����Ի���ID����ʾ�ͻ��˶Ͽ�����
// ���ʹ�����Դg_ConnectionIdRes�������߸����������
//
void DeallocID(int id)
{
    static int ELEM_BITS = sizeof(g_ConnectionIdRes.idResource[0]) * 8;

    if ((id >= 0) && (id < MAX_CONNECTION_NUM)) {
        g_ConnectionIdRes.idResource[id / ELEM_BITS] &= ~(1 << (id % ELEM_BITS));
    } else {
        WriteLog(LOG_ERROR, _T("�޷����շǷ�ID: %d [Check your program!]"), id);
    }
}

// �ȴ������̵߳���ں���
// ���̵߳Ĺ����ǵȴ����ӣ�������һ�����ӣ��ʹ���һ�������߳�
DWORD WaitForConnectionThreadProc(LPVOID lpData)
{
    UNREFERENCED_PARAMETER(lpData);

    int id;
    DWORD nThreadID;
    SOCKET sock = INVALID_SOCKET;

    int len = 0;
    SOCKADDR_IN addr = { 0 };

    for (;;) {
        sock = INVALID_SOCKET;
        while (sock == INVALID_SOCKET) {
            if (g_bServerWillShutdown == TRUE) { return 0; }

            len = sizeof(SOCKADDR_IN);
            ZeroMemory(&addr, len);

            // �ȴ��ͻ���������
            sock = accept(g_ServerSocket, (SOCKADDR*)&addr, &len);
        }

        EnterCriticalSection(&g_csConnectionNum);

        // ���ƿͻ���������
        if (g_nCurConnectionNum >= MAX_CONNECTION_NUM) {
            shutdown(sock, SD_BOTH);
            closesocket(sock);
            WriteLog(LOG_WARN, _T("�ͻ��������ﵽ����ֵ: %d���ܾ�Զ������: [%d.%d.%d.%d: %d]"),
                g_csConnectionNum, (BYTE)addr.sin_addr.s_net, (BYTE)addr.sin_addr.s_host,
                (BYTE)addr.sin_addr.s_imp, (BYTE)addr.sin_addr.s_impno, ntohs(addr.sin_port));

            LeaveCriticalSection(&g_csConnectionNum);
            continue;
        }

        id = AllocID();
        if (IS_INVALID_USER_ID(id)) {
            shutdown(sock, SD_BOTH);
            closesocket(sock);

            LeaveCriticalSection(&g_csConnectionNum);
            continue;
        }

        g_nCurConnectionNum++;
        LeaveCriticalSection(&g_csConnectionNum);

        ZeroMemory(&g_ConnectionInfo[id], sizeof(CONNECTION_INFO));
        M_CONNECT_SOCK(id) = sock;
        M_CONNECT_STATE(id) = STATE_IDLE;
        M_CONNECT_TABLE(id) = INVALID_TABLE;
        M_CONNECT_SEAT(id) = INVALID_SEAT;
        M_CONNECT_ALLOW_LOOKON(id) = TRUE;
        GetLocalTime(&M_CONNECT_TIME(id));

        WriteLog(LOG_INFO, _T("���ܿͻ��� [%d.%d.%d.%d: %d] ���ӣ�����ID: %d"), 
            (BYTE)addr.sin_addr.s_net, (BYTE)addr.sin_addr.s_host,
            (BYTE)addr.sin_addr.s_imp, (BYTE)addr.sin_addr.s_impno,
            ntohs(addr.sin_port), id);

        // Ϊ�����Ӵ���һ�������߳�
        g_hConnectionThread[id] = CreateThread(NULL, 0,
            (LPTHREAD_START_ROUTINE)ConnectionThreadProc, (LPVOID)id, 0, &nThreadID);
    }
}

// �����̵߳���ں���
// ���̵߳Ĺ�������ͻ���ͨ�ţ�������Ϣ��������Ϣ��������Ϣ��
DWORD ConnectionThreadProc(LPVOID lpData)
{
    char* buf = NULL;
    int nRecv = 0;

    int id = (int)lpData;
    int len = sizeof(SOCKADDR_IN);
    SOCKADDR_IN addr = { 0 };
    SOCKET sock = INVALID_SOCKET;

    sock = M_CONNECT_SOCK(id);
    getpeername(sock, (SOCKADDR*)&addr, &len);

    buf = (char*)LocalAlloc(LMEM_FIXED, CONNECTION_RECV_BUF_SZ);
    if (buf == NULL) {
        shutdown(sock, SD_BOTH);
        closesocket(sock);
        M_CONNECT_SOCK(id) = INVALID_SOCKET;

        //
        // ֱ�ӻ���ID��������Ҫ��ʱ�㲥�߳�ȥ����
        // ��Ϊ���߳̽����أ�������տͻ��˷������� REQ_ROOM_INFO��������֪ͨ
        // ��ʱ�㲥�߳������ EVT_ENTER��
        //
        EnterCriticalSection(&g_csConnectionNum);
        WriteLog(LOG_INFO, _T("����ID: %d ���ر��׽��֡�[����������ݵĻ�����ʧ�ܣ��������Ͽ���ͻ��˵�����]"), id);
        DeallocID(id);
        g_nCurConnectionNum--;
        LeaveCriticalSection(&g_csConnectionNum);

        // ֪ͨ���̹߳رո������߳̾��
        PostThreadMessage(g_nMainThreadId, TM_CLIENT_DISCONNECT, (WPARAM)g_hConnectionThread[id], 0);
        g_hConnectionThread[id] = NULL;

        return 0;
    }

    for (;;) {
        nRecv = recv(sock, buf, CONNECTION_RECV_BUF_SZ, 0);

        if (nRecv > 0) {
#if 0
#define DBG_RCV_SZ      64
            TCHAR strData[DBG_RCV_SZ];
            int len = 0;

            for (int i = 0; i < nRecv; i++) {
                if (len >= DBG_RCV_SZ - 8) { break; }
                len += _stprintf_s(strData + len, sizeof(strData) / sizeof(strData[0]) - len,
                    _T("0x%02X "), (BYTE)buf[i]);
            }

            WriteLog(LOG_DEBUG, _T("�յ�����: [%4d] %s"), nRecv, strData);
#endif
            // ������յ�����Ϣ
            OnMessage(id, buf, nRecv);
        }
        
        else if (nRecv == 0) {
            WriteLog(LOG_INFO, _T("Զ������:[%d.%d.%d.%d: %d] �����Ͽ�����"),
                (BYTE)addr.sin_addr.s_net, (BYTE)addr.sin_addr.s_host,
                (BYTE)addr.sin_addr.s_imp, (BYTE)addr.sin_addr.s_impno, ntohs(addr.sin_port));
            break;
        }
        
        else {
            WriteLog(LOG_WARN, _T("�������ݳ���: [%d]����Զ������ ID: [%d] �����������"),
                WSAGetLastError(), id);
            break;
        }
    }

    LocalFree(buf);

    if (g_bServerWillShutdown == TRUE) {
        // �������˳�������ر����ӡ����߳̽�����ر�Socket���رո��߳̾��
        return 0;
    }

    // ��ʶ�ÿͻ��˶Ͽ�����
    M_CONNECT_LOST(id) = TRUE;

    //
    // ���ӶϿ���֪ͨ��Ϸ�����̣߳����й���������������TABLE_INFO�ṹ����Ӧ
    // ��λ�����ID�����������ڽ�����Ϸ������ȵ���Ϸ�����󣬲��ܲ���������
    // �����Ҿͻ�������λ
    //
    if (M_CONNECT_STATE(id) != STATE_IDLE) {
        int table = M_CONNECT_TABLE(id);
        int seat = M_CONNECT_SEAT(id);
        PostThreadMessage(g_nGameThreadIds[table], TM_CONNECTION_LOST, (WPARAM)id, (LPARAM)seat);
    } else {
        WriteLog(LOG_INFO, _T("ID: %d �������ա�[������ҶϿ�����]"), id);
        AddUserStateChangeEvent(id, EVT_LEAVE, (PLAYER_STATE)0, FALSE);
    }

    //
    // ���̼߳����˳������߳̾����Ҫ���رա�
    // ����ǿͻ��˶Ͽ����ӣ��������Ϣ֪ͨ���̹߳رո������߳̾��������
    // ��ʾ�����������˳����򣬷����������˳�ʱ��رո������߳̾����
    //
    if (g_bServerWillShutdown != TRUE) {
        PostThreadMessage(g_nMainThreadId, TM_CLIENT_DISCONNECT, (WPARAM)g_hConnectionThread[id], 0);
        g_hConnectionThread[id] = NULL;
    }

    return 0;
}

// ��ʱ�㲥�̵߳���ں���
// ���̵߳Ĺ����Ƕ�ʱ��������뿪���䣬��״̬�仯�������Ϣ���͸��������߿ͻ���
DWORD OnTimeBroadcastThreadProc(LPVOID lpData)
{
    UNREFERENCED_PARAMETER(lpData);

#define MAX_SEND_DATA_LEN   4096
    char* data = (char*)LocalAlloc(LMEM_FIXED, MAX_SEND_DATA_LEN);
    if (data == NULL) {
        WriteLog(LOG_ERROR, _T("��ʱ�㲥�̷߳��䷢�����ݵ��ڴ�ʧ�ܣ��߳̽��˳�"));
        return 0;
    }

    while (WaitForSingleObject(g_mutexOnTimeBroadcast, BROADCAST_TIME_INTERVAL) != WAIT_OBJECT_0) {
        EnterCriticalSection(&g_csUserStateChangeList);

        OnTimeBuildMessageAndBroadcast(data, MAX_SEND_DATA_LEN);

        LeaveCriticalSection(&g_csUserStateChangeList);
    }

    ReleaseMutex(g_mutexOnTimeBroadcast);

    LocalFree(data);
    return 0;
}

// ��ʱ�������״̬�仯����Ϣ�����㲥��ȥ
//
// Param:
//      mem  - memory for fill in the message data
//      len  - length of the memory
//
static void OnTimeBuildMessageAndBroadcast(char* mem, int len)
{
    int id = INVALID_USER_ID;
    BYTE* data = (BYTE*)mem;

    USER_STATE_CHANGE* pNode = NULL;
    USER_STATE_CHANGE* pTemp = NULL;

    // ��¼���ι㲥�У������Ľڵ�����
    int nodecount = 0;

    // ��¼��Щ����˳���Ȼ�������ID
    static int s_LeftUserIds[MAX_CONNECTION_NUM];
    int nLeftUserCount = 0;

    // ��0�ֽ�Ϊ��Ϣ�룬��1��2�ֽ�Ϊ��Ϣ�峤�ȣ���3��6�ֽ�Ϊ��������ǰ״̬��
    // ��7��8�ֽ�Ϊ���͵Ľڵ�����
    int offset = 9;

    pNode = g_UserStateChangeList.lpFirstNode;
    while (pNode != NULL) {
        pTemp = pNode;
        id = pTemp->id;

        // ���ID: 16-bit 
        data[offset + 0] = (BYTE)(id >> 8);
        data[offset + 1] = (BYTE)(id >> 0);
        offset += 2;

        if (pTemp->evt == EVT_CHGSTATE) {
            // ״̬����
            data[offset] = (BYTE)((int)EVT_CHGSTATE);
            offset++;

            // ���״̬
            data[offset] = (BYTE)pTemp->newState;
            offset++;

            // �����Ҵ��ڿ���״̬������һ���ֽڵ����λ��ʾ���Ƿ������Թ�
            // �����Ҳ��Ǵ��ڿ���״̬������һ���ֽڵ����λ��ʾ���Ƿ������Թۣ�
            // ��4λ��ʾ�������Ӻţ��ټ�һ�ֽڱ�ʾ�����ڵ����Ӻ�
            data[offset] = 0;
            if (pTemp->newState == STATE_IDLE) {
                data[offset] = M_CONNECT_ALLOW_LOOKON(id) ? 0x80 : 0x00;
                offset++;
            } else {
                data[offset] = M_CONNECT_ALLOW_LOOKON(id) ? 0x80 : 0x00;
                data[offset] |= (BYTE)(M_CONNECT_SEAT(id) & 0x0F);
                offset++;

                data[offset] = (BYTE)(M_CONNECT_TABLE(id));
                offset++;
            }

            if (pTemp->bContainStatistics == FALSE) { // �Ƿ���ҪЯ�����ͳ����Ϣ
                data[offset] = 0;
                offset++;
            } else {
                data[offset] = 0x80;
                offset++;

                // ��ҵĻ��ֵ�ͳ����Ϣ
                data[offset + 0] = (BYTE)(M_CONNECT_PLAYER_SCORE(id) >> 24);
                data[offset + 1] = (BYTE)(M_CONNECT_PLAYER_SCORE(id) >> 16);
                data[offset + 2] = (BYTE)(M_CONNECT_PLAYER_SCORE(id) >> 8);
                data[offset + 3] = (BYTE)(M_CONNECT_PLAYER_SCORE(id) >> 0);
                offset += 4;

                data[offset + 0] = (BYTE)(M_CONNECT_PLAYER_TOTALGAMES(id) >> 24);
                data[offset + 1] = (BYTE)(M_CONNECT_PLAYER_TOTALGAMES(id) >> 16);
                data[offset + 2] = (BYTE)(M_CONNECT_PLAYER_TOTALGAMES(id) >> 8);
                data[offset + 3] = (BYTE)(M_CONNECT_PLAYER_TOTALGAMES(id) >> 0);
                offset += 4;

                data[offset + 0] = (BYTE)(M_CONNECT_PLAYER_WINGAMES(id) >> 24);
                data[offset + 1] = (BYTE)(M_CONNECT_PLAYER_WINGAMES(id) >> 16);
                data[offset + 2] = (BYTE)(M_CONNECT_PLAYER_WINGAMES(id) >> 8);
                data[offset + 3] = (BYTE)(M_CONNECT_PLAYER_WINGAMES(id) >> 0);
                offset += 4;

                data[offset + 0] = (BYTE)(M_CONNECT_PLAYER_RUNAWAY(id) >> 24);
                data[offset + 1] = (BYTE)(M_CONNECT_PLAYER_RUNAWAY(id) >> 16);
                data[offset + 2] = (BYTE)(M_CONNECT_PLAYER_RUNAWAY(id) >> 8);
                data[offset + 3] = (BYTE)(M_CONNECT_PLAYER_RUNAWAY(id) >> 0);
                offset += 4;
            }
        } else if (pTemp->evt == EVT_ENTER) {
            // ״̬����
            data[offset] = (BYTE)((int)EVT_ENTER);
            offset++;

            // ���ֳ��ȣ�1�ֽڣ��ȱ������±꣬�Ȼ�������ֵ
            int name_len_offset = offset;
            offset++;

            // �������
            _tcscpy_s((TCHAR*)(data + offset), MAX_USER_NAME_LEN, M_CONNECT_PLAYER_NAME(id));
            int name_len = _tcslen(M_CONNECT_PLAYER_NAME(id)) * sizeof(TCHAR);
            offset += name_len;

            // ������ֳ��ȣ��ֽ���)
            data[name_len_offset] = (BYTE)name_len;

            // ��7-bit��ʾ�Ա�6~4-bitΪ״̬��3~0-bitΪͷ������
            data[offset] = 0;
            data[offset] |= (BYTE)((int)(M_CONNECT_PLAYER_GENDER(id)) << 7);
            data[offset] |= (BYTE)((int)(STATE_IDLE) << 4);
            data[offset] |= (BYTE)((int)(M_CONNECT_PLAYER_IMG_IDX(id) & 0x0F));
            offset++;
        } else if (pTemp->evt == EVT_LEAVE) {
            // ״̬����
            data[offset] = (BYTE)((int)EVT_LEAVE);
            offset++;

            s_LeftUserIds[nLeftUserCount++] = id;
        }

        // ����һ����㣬��ɾ��һ�����
        pNode = pNode->next;
        g_UserStateChangeList.lpFirstNode = pNode;
        LocalFree(pTemp);

        nodecount++;

        if (offset > len - (MAX_USER_NAME_LEN + 8)) {
            break;
        }
    }

    if (nodecount > 0) { // �д���������
        // �����Ϣ��
        data[0] = NMSG_PLAYER_CHANGE_STATE;

        // �����Ϣ�峤�ȣ���ȥ��Ϊ��Ϣ������ֽڼ���Ϣ�峤���ֶ����ֽ�
        data[1] = (BYTE)((offset - 3) >> 8);
        data[2] = (BYTE)((offset - 3) >> 0);

        // ����������ǰ����״��
        //EnterCriticalSection(&g_csConnectionNum);
        data[3] = (BYTE)(g_nCurConnectionNum >> 8);
        data[4] = (BYTE)(g_nCurConnectionNum >> 0);
        //LeaveCriticalSection(&g_csConnectionNum);

        int max_conn_num = MAX_CONNECTION_NUM;
        data[5] = (BYTE)(max_conn_num >> 8);
        data[6] = (BYTE)(max_conn_num >> 0);

        data[7] = (BYTE)(nodecount >> 8);
        data[8] = (BYTE)(nodecount >> 0);

        // ׷��CRC�ֶ�
        WORD crc = CRC16(data, offset);
        data[offset] = (BYTE)(crc >> 8);
        data[offset + 1] = (BYTE)(crc >> 0);
        offset += 2;

        // �㲥��Ϣ���������߿ͻ���
        for (int id = 0; id < MAX_CONNECTION_NUM; id++) {
            SOCKET sock = M_CONNECT_SOCK(id);
            if (sock != INVALID_SOCKET) {
                if (M_CONNECT_LOST(id) == FALSE) {
                    int num = send(sock, (char*)data, offset, 0);
                    if (num <= 0) {
                        WriteLog(LOG_WARN, _T("��ʱ�㲥��Ϣʱ�����͵��ͻ���[ID: %d]ʧ�ܡ�[�ÿͻ��˿����ѵ���]"), id);
                    }
                }
            }
        }

        //WriteLog(LOG_INFO, _T("���ζ�ʱ�㲥��Ϣʱ�������� %d ����Ϣ"), nodecount);

        // ������ȫ��������
        if (g_UserStateChangeList.lpFirstNode == NULL) {
            g_UserStateChangeList.lpLastNode = NULL;
        } else {
            WriteLog(LOG_WARN, _T("��ʱ�߳�δ���������д������ݣ����µ����ݽ��ۻ����´η���"));
        }
    } else {
        //WriteLog(LOG_DEBUG, _T("���ζ�ʱ�㲥��Ϣʱ��û����Ҫ���͵�����"));
    }

    // ����������ҵ�ID
    if (nLeftUserCount > 0) {
        EnterCriticalSection(&g_csConnectionNum);

        for (int i = 0; i < nLeftUserCount; i++) {
            DeallocID(s_LeftUserIds[i]);
            g_nCurConnectionNum--;

            closesocket(M_CONNECT_SOCK(s_LeftUserIds[i]));
            M_CONNECT_SOCK(s_LeftUserIds[i]) = INVALID_SOCKET;
            M_CONNECT_PALYER_AVAILABLE(s_LeftUserIds[i]) = FALSE;

            WriteLog(LOG_INFO, _T("����ID: %d"), s_LeftUserIds[i]);
        }

        LeaveCriticalSection(&g_csConnectionNum);
    }
}

//
// �������յ�����Ϣ�����ַ�������Ϣ������
//
// ����ͻ���ͬʱ���Ͷ����Ϣ����������ֻҪÿ����Ϣ�ṹ�Ϸ�����
// ��Ҳ�ǿ��ǵ�����������æ����������æ����ȥ��������ʱ�������Ѿ��ۻ������Ϣ������
//
static void OnMessage(int id, char* buf, int len)
{
    WORD myCrc, yourCrc;

    int nMsgCode = 0;       // ��Ϣ��
    int nMsgBodyLen = 0;    // ��Ϣ�峤��
    int nParsedLen = 0;     // ���������ݵĳ���

    BYTE* data = (BYTE*)buf;

    char* msg = NULL;
    int   msglen = 0;
    int   msgcount = 0;

    while (nParsedLen < len) {
        if (nParsedLen + 2 >= len) {
            WriteLog(LOG_ERROR, _T("[1]������Ϣʱ�������ṹ���Ϸ�����Ϣ���������ν������ѽ��� %d ���Ϸ���Ϣ"), msgcount);
            return;
        }

        nMsgBodyLen = (data[nParsedLen + 1] << 8) | data[nParsedLen + 2];

        // 1���ֽ���Ϣ�룬2���ֽ���Ϣ�峤�ȣ�nMsgBodyLen���ֽ���Ϣ�壬2���ֽ�CRC
        if (nParsedLen + 1 + 2 + nMsgBodyLen + 2 > len) {
            WriteLog(LOG_ERROR, _T("[2]������Ϣʱ�������ṹ���Ϸ�����Ϣ���������ν������ѽ��� %d ���Ϸ���Ϣ"), msgcount);
            return;
        }

        myCrc = CRC16((data + nParsedLen), 1 + 2 + nMsgBodyLen);
        yourCrc = (data[nParsedLen + 1 + 2 + nMsgBodyLen] << 8) | data[nParsedLen + 1 + 2 + nMsgBodyLen + 1];
        if (myCrc != yourCrc) {
            WriteLog(LOG_ERROR, _T("���յ�����CRC�ֶβ���ȷ���������ν������ѽ��� %d ���Ϸ���Ϣ"), msgcount);
            return;
        }

        nMsgCode = data[nParsedLen];
        msg = (char*)(data + nParsedLen);
        msglen = 1 + 2 + nMsgBodyLen;

        // 1���ֽ���Ϣ�룬2���ֽ���Ϣ�峤�ȣ�nMsgBodyLen���ֽ���Ϣ�壬2���ֽ�CRC
        nParsedLen += (1 + 2 + nMsgBodyLen + 2);
        msgcount++;

        // �ַ���Ϣʱ����������Ϣ��������CRC�ֶεĳ��ȴ��ݸ���Ӧ������
        switch (nMsgCode) {
        case NMSG_REQ_ROOM_INFO:
            OnMsgReqRoomInfo(id, msg, msglen);
            break;

        case NMSG_REQ_GAMER_TAKE_SEAT:
            OnMsgReqGamerTakeSeat(id, msg, msglen);
            break;

        case NMSG_REQ_GAMER_LEAVE_SEAT:
            OnMsgReqGamerLeaveSeat(id, msg, msglen);
            break;

        case NMSG_REQ_LOOKON_TAKE_SEAT:
            OnMsgReqLookonTakeSeat(id, msg, msglen);
            break;

        case NMSG_REQ_LOOKON_LEAVE_SEAT:
            OnMsgReqLookonLeaveSeat(id, msg, msglen);
            break;

        case NMSG_REQ_GAMER_READY:
            OnMsgReqGamerReady(id, msg, msglen);
            break;

        // ������ϢΪ��Ϸ��Ϣ��ֱ��ת������Ϸ�����߳�
        default:
            PostMsgDataToGameThread(id, msg, msglen);
            break;
        }
    }
}

// ���ʹ������ݸ��ͻ���
static void SendRoomInfo(int nAllocID)
{
    SOCKET sock = M_CONNECT_SOCK(nAllocID);

    int offset = 0;
    BYTE* data = NULL;

    // ����һ���㹻����ڴ�
    int nBufLen = (sizeof(CONNECTION_INFO) - sizeof(SYSTEMTIME)) * MAX_CONNECTION_NUM;

    data = (BYTE*)LocalAlloc(LMEM_FIXED, nBufLen);
    if (data == NULL) {
        WriteLog(LOG_ERROR, _T("���ʹ������ݸ��ͻ���ʧ�ܣ������ڴ�ʧ��"));
        return;
    }

    //
    // �����Ϣ��
    //
    // ��0�ֽ�Ϊ��Ϣ�룬��1��2�ֽ�Ϊ��Ϣ�峤�ȣ���3��4�ֽ�Ϊ������Ϊ�ÿͻ��˷����ID��
    // ��5��6�ֽ�Ϊ��ǰ��������������ӵ�7�ֽڿ�ʼ��������Ϣ
    //
    offset = 7;

    int nUserCount = 0;

    // ����Ϣ�н��������������������Ϣ
    for (int id = 0; id < MAX_CONNECTION_NUM; id++) {
        if (M_CONNECT_PALYER_AVAILABLE(id) == TRUE) {
            // ���ID: 16-bit 
            data[offset] = (BYTE)(id >> 8);
            data[offset + 1] = (BYTE)(id >> 0);
            offset += 2;

            // ���ֳ��ȣ��ֽ�������ռ��1�ֽڡ��ȱ������±꣬�Ȼ�������ֵ
            int name_len_offset = offset;
            offset++;

            // �������
            _tcscpy_s((TCHAR*)(data + offset), MAX_USER_NAME_LEN, M_CONNECT_PLAYER_NAME(id));
            int name_len = _tcslen(M_CONNECT_PLAYER_NAME(id)) * sizeof(TCHAR);
            offset += name_len;

            // ������ֳ���
            data[name_len_offset] = (BYTE)name_len;

            // ��7-bit��ʾ�Ա�6~4-bitΪ״̬��3~0-bitΪͷ������
            data[offset] = 0;
            data[offset] |= (BYTE)((int)(M_CONNECT_PLAYER_GENDER(id)) << 7);
            data[offset] |= (BYTE)((int)(M_CONNECT_STATE(id) & 0x07) << 4);
            data[offset] |= (BYTE)((int)(M_CONNECT_PLAYER_IMG_IDX(id) & 0x0F));
            offset++;

            // �����Ҵ��ڿ���״̬������һ���ֽڵ����λ��ʾ���Ƿ������Թ�
            // �����Ҳ��Ǵ��ڿ���״̬������һ���ֽڵ����λ��ʾ���Ƿ������Թۣ�
            // ��4λ��ʾ�������Ӻţ��ټ�һ�ֽڱ�ʾ�����ڵ����Ӻ�
            data[offset] = 0;
            if (M_CONNECT_STATE(id) == STATE_IDLE) {
                data[offset] = M_CONNECT_ALLOW_LOOKON(id) ? 0x80 : 0x00;
                offset++;
            } else {
                data[offset] = M_CONNECT_ALLOW_LOOKON(id) ? 0x80 : 0x00;
                data[offset] |= (BYTE)(M_CONNECT_SEAT(id) & 0x0F);
                offset++;

                data[offset] = (BYTE)(M_CONNECT_TABLE(id));
                offset++;
            }

            // ��ҵĻ��ֵ�ͳ����Ϣ
            data[offset + 0] = (BYTE)(M_CONNECT_PLAYER_SCORE(id) >> 24);
            data[offset + 1] = (BYTE)(M_CONNECT_PLAYER_SCORE(id) >> 16);
            data[offset + 2] = (BYTE)(M_CONNECT_PLAYER_SCORE(id) >> 8);
            data[offset + 3] = (BYTE)(M_CONNECT_PLAYER_SCORE(id) >> 0);
            offset += 4;

            data[offset + 0] = (BYTE)(M_CONNECT_PLAYER_TOTALGAMES(id) >> 24);
            data[offset + 1] = (BYTE)(M_CONNECT_PLAYER_TOTALGAMES(id) >> 16);
            data[offset + 2] = (BYTE)(M_CONNECT_PLAYER_TOTALGAMES(id) >> 8);
            data[offset + 3] = (BYTE)(M_CONNECT_PLAYER_TOTALGAMES(id) >> 0);
            offset += 4;

            data[offset + 0] = (BYTE)(M_CONNECT_PLAYER_WINGAMES(id) >> 24);
            data[offset + 1] = (BYTE)(M_CONNECT_PLAYER_WINGAMES(id) >> 16);
            data[offset + 2] = (BYTE)(M_CONNECT_PLAYER_WINGAMES(id) >> 8);
            data[offset + 3] = (BYTE)(M_CONNECT_PLAYER_WINGAMES(id) >> 0);
            offset += 4;

            data[offset + 0] = (BYTE)(M_CONNECT_PLAYER_RUNAWAY(id) >> 24);
            data[offset + 1] = (BYTE)(M_CONNECT_PLAYER_RUNAWAY(id) >> 16);
            data[offset + 2] = (BYTE)(M_CONNECT_PLAYER_RUNAWAY(id) >> 8);
            data[offset + 3] = (BYTE)(M_CONNECT_PLAYER_RUNAWAY(id) >> 0);
            offset += 4;

            // ���һ�������Ϣ���ռ�
            nUserCount++;
        }
    }

    // ��Ϣ��
    data[0] = NMSG_ACK_ROOM_INFO;

    // �����Ϣ�峤�ȣ���ȥ��Ϊ��Ϣ������ֽڼ���Ϣ�峤���ֶ����ֽ�
    data[1] = (BYTE)((offset - 3) >> 8);
    data[2] = (BYTE)((offset - 3) >> 0);

    // ������������ÿͻ��˵�ID
    data[3] = (BYTE)(nAllocID >> 8);
    data[4] = (BYTE)(nAllocID >> 0);

    // �������
    data[5] = (BYTE)(nUserCount >> 8);
    data[6] = (BYTE)(nUserCount >> 0);

    // ׷��CRC�ֶ�
    WORD crc = CRC16(data, offset);
    data[offset + 0] = (BYTE)(crc >> 8);
    data[offset + 1] = (BYTE)(crc >> 0);
    offset += 2;

    int num = send(sock, (char*)data, offset, 0);
    if (num < 0) {
        WriteLog(LOG_ERROR, _T("���ʹ�����Ϣʧ��"));
    }

    LocalFree(data);
}

// ���½��뷿��������Ϣ��������
// ������̬�������ڴ棬�ɶ�ʱ�㲥��Ϣ���߳��ͷ�
void AddUserStateChangeEvent(int id, STATE_CHANGE_EVENT evt, PLAYER_STATE newState, BOOL bContainStat)
{
    USER_STATE_CHANGE* pNode;

    if (g_bServerWillShutdown == TRUE) {
        return;
    }

    pNode = (USER_STATE_CHANGE*)LocalAlloc(LMEM_FIXED, sizeof(USER_STATE_CHANGE));
    if (pNode == NULL) {
        WriteLog(LOG_ERROR, _T("����¿ͻ���Ϣ������ʱ�������ڴ�ʧ��"));
        return;
    }

    EnterCriticalSection(&g_csUserStateChangeList);

    pNode->id = id;
    pNode->evt = evt;
    pNode->newState = newState;
    pNode->bContainStatistics = bContainStat;
    pNode->next = NULL;

    if (g_UserStateChangeList.lpFirstNode == NULL) {
        g_UserStateChangeList.lpFirstNode = pNode;
        g_UserStateChangeList.lpLastNode = pNode;
    } else {
        g_UserStateChangeList.lpLastNode->next = pNode;
        g_UserStateChangeList.lpLastNode = pNode;
    }

    LeaveCriticalSection(&g_csUserStateChangeList);
}

// �ͻ�����ҽ�������������������
static void OnMsgReqRoomInfo(int id, char* msg, int len)
{
    //
    // �ȸ��ͻ��˷��ʹ������ݣ����ű���ͻ��˵Ļ�����Ϣ
    //
    SendRoomInfo(id);

    // �����������ĸÿͻ�����һ�����Ϣ
    int offset = 0;
    BYTE* data = (BYTE*)msg;

    // ������Ϣ���ֶ�
    offset++;

    // ��Ϣ�峤��
    int nMsgBodyLen = (data[offset] << 8) | data[offset + 1];
    if (nMsgBodyLen != len - 3) { // ��ȥ1�ֽ���Ϣ���2�ֽڵ���Ϣ�峤��
        WriteLog(LOG_ERROR, _T("NMSG_REQ_ROOM_INFO ��Ϣ�ṹ����ȷ"));
        return;
    }
    offset += 2;

    // ���ֳ���
    int name_len = data[offset];
    offset++;

    // ����
    int max_valid_len = (MAX_USER_NAME_LEN - 1) * sizeof(TCHAR);
    int valid_len = name_len < max_valid_len ?  name_len : max_valid_len;

    BYTE* pName = (BYTE*)M_CONNECT_PLAYER_NAME(id);
    for (int i = 0; i < valid_len; i++) {
        *(pName + i) = data[offset + i];
    }

    int valid_char_num = valid_len / sizeof(TCHAR);
    M_CONNECT_PLAYER_NAME(id)[valid_char_num] = _T('\0');
    offset += name_len;

    // �Ա�״̬��ͷ������
    M_CONNECT_PLAYER_GENDER(id) = (PLAYER_GENDER)(data[offset] >> 7);
    M_CONNECT_STATE(id) = STATE_IDLE; // �ս����������IDLE״̬
    M_CONNECT_PLAYER_IMG_IDX(id) = data[offset] & 0x0F;
    offset++;

    // ��ʶ����ҵ���Ϣ�Ѿ��ϴ���������
    M_CONNECT_PALYER_AVAILABLE(id) = TRUE;

    // ׷���û�״̬�仯������
    AddUserStateChangeEvent(id, EVT_ENTER, (PLAYER_STATE)0, FALSE);

    WriteLog(LOG_INFO, _T("�յ��û� [ID: %d] ������Ϣ: %s, %s, %d"), id,
        M_CONNECT_PLAYER_NAME(id),
        M_CONNECT_PLAYER_GENDER(id) == MALE ? _T("MALE"): _T("FEMALE") ,
        M_CONNECT_PLAYER_IMG_IDX(id));
}

// ����Ϸ���ڹ㲥���г�Ա��������Ϣ
// �ռ���Ϸ�������г�Ա����Ϣ�����㲥��ÿ����Ա
//
// ������ʱ�����ǵ�����һ�����⣺��������������ӷ������󣬷ֱ�ѡ������1������
// 1�����Ӻ�2�����ӣ�����������REQ_GAMER_TAKE_SEAT��Ϣ���������ʱ����1���ӵ�
// ��ʱ�㲥֮ǰ�������ӵ����û�������ӵ���һ�����Ϣ��Ϊ�˱���������⣬
// ��ʱ���ʱ���ص��ڻظ��������ʱ��������������ACK_GAMER_TAKE_SEATʱ����
// ����Ϸ�������г�Ա��������Ϣ�����͵��ͻ��ˡ������Ʊ��붨ʱ�㲥�̵߳Ĺ�������
// �ظ����������ӷ����������縺�ء�������������ʵû��Ҫ��������Ϊ�˱���ÿ��ܴ���
// �����⣬�ͻ���������յ�ACK_GAMER_TAKE_SEAT��ACK_LOOKON_TAKE_SEATʱ����1���
// ������Ϸ���档
//
// �޸������������������Ϸ�������������Ϣ��������Ϸ�ڹ㲥���������ID�������Ӻ�
// 2009-06-07
//
static void ExchangeInfoInTable(int myid, int table, int seat, BYTE msgcode)
{
    int id;
    int offset = 0;

    int nBufLen = (MAX_USER_NAME_LEN + 10) * (GAME_SEAT_NUM_PER_TABLE) * (MAX_LOOKON_NUM_PER_SEAT + 1);
    BYTE* data = (BYTE*)LocalAlloc(LMEM_FIXED, nBufLen);
    if (data == NULL) {
        WriteLog(LOG_ERROR, _T("��Ϸ���ڹ㲥��Ϣʱ��Ϊ�������ݶ������ڴ�ʧ��"));
        return;
    }

    // ��Ϣ��
    data[0] = (BYTE)msgcode;

    // ��Ϣ�峤��
    data[1] = 0;
    data[2] = 0;

    // ��Ϣ��
    data[3] = (BYTE)TRUE; // ��һ��Թ��߿��Խ�����Ϸ��
    offset = 4;

#if 0
    for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
        id = M_TABLE_PLAYER_ID(table, seat);

        // ����λ�Ѿ���һ�����
        if (id != INVALID_USER_ID) {
            // ID
            data[offset] = (BYTE)(id >> 8);
            data[offset + 1] = (BYTE)(id >> 0);
            offset += 2;

            // ���ֳ��ȣ�1�ֽڣ��ȱ������±꣬�Ȼ�������ֵ
            int name_len_offset = offset;
            offset++;

            // �������
            _tcscpy_s((TCHAR*)(data + offset), MAX_USER_NAME_LEN, M_CONNECT_PLAYER_NAME(id));
            int name_len = _tcslen(M_CONNECT_PLAYER_NAME(id)) * sizeof(TCHAR);
            offset += name_len;

            // ������ֳ���
            data[name_len_offset] = (BYTE)name_len;

            // ��7-bit��ʾ�Ա�6~4-bitΪ״̬��3~0-bitΪͷ������
            data[offset] = 0;
            data[offset] |= (BYTE)((int)(M_CONNECT_PLAYER_GENDER(id)) << 7);
            data[offset] |= (BYTE)((int)(M_CONNECT_STATE(id) & 0x07) << 4);
            data[offset] |= (BYTE)((int)(M_CONNECT_PLAYER_IMG_IDX(id) & 0x0F));
            offset++;

            data[offset] = M_CONNECT_ALLOW_LOOKON(id) ? 0x80 : 0x00;
            data[offset] |= (BYTE)(M_CONNECT_SEAT(id) & 0x0F);
            offset++;

            data[offset] = (BYTE)(M_CONNECT_TABLE(id));
            offset++;

            // ����λ���Թ�����Ϣ
            for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
                id = M_TABLE_LOOKON_ID(table, seat)[i];

                if (id != INVALID_USER_ID) {
                    // ID
                    data[offset] = (BYTE)(id >> 8);
                    data[offset + 1] = (BYTE)(id >> 0);
                    offset += 2;

                    // ���ֳ��ȣ�1�ֽڣ��ȱ������±꣬�Ȼ�������ֵ
                    int name_len_offset = offset;
                    offset++;

                    // �������
                    _tcscpy_s((TCHAR*)(data + offset), MAX_USER_NAME_LEN, M_CONNECT_PLAYER_NAME(id));
                    int name_len = _tcslen(M_CONNECT_PLAYER_NAME(id)) * sizeof(TCHAR);
                    offset += name_len;

                    // ������ֳ���
                    data[name_len_offset] = (BYTE)name_len;

                    // ��7-bit��ʾ�Ա�6~4-bitΪ״̬��3~0-bitΪͷ������
                    data[offset] = 0;
                    data[offset] |= (BYTE)((int)(M_CONNECT_PLAYER_GENDER(id)) << 7);
                    data[offset] |= (BYTE)((int)(M_CONNECT_STATE(id)& 0x07) << 4);
                    data[offset] |= (BYTE)((int)(M_CONNECT_PLAYER_IMG_IDX(id)& 0x0F));
                    offset++;

                    data[offset] = M_CONNECT_ALLOW_LOOKON(id)? 0x80 : 0x00;
                    data[offset] |= (BYTE)(M_CONNECT_SEAT(id) & 0x0F);
                    offset++;

                    data[offset] = (BYTE)(M_CONNECT_TABLE(id));
                    offset++;
                }
            }
        }
    }
#else
    data[offset] = (BYTE)table;
    offset++;

    data[offset] = (BYTE)seat;
    offset++;

    data[offset + 0] = (BYTE)(myid >> 8);
    data[offset + 1] = (BYTE)(myid >> 0);
    offset += 2;
#endif

    // �����Ϣ�峤�ȣ���ȥ��Ϊ��Ϣ������ֽڼ���Ϣ�峤���ֶ����ֽ�
    data[1] = (BYTE)((offset - 3) >> 8);
    data[2] = (BYTE)((offset - 3) >> 0);

    // ׷��CRC�ֶ�
    WORD crc = CRC16(data, offset);
    data[offset] = (BYTE)(crc >> 8);
    data[offset + 1] = (BYTE)(crc >> 0);
    offset += 2;

    // �ռ������ݺ�����Ϸ���й㲥
    for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
        id = M_TABLE_PLAYER_ID(table, seat);

        if (id != INVALID_USER_ID) {
            SOCKET sock = M_CONNECT_SOCK(id);
            if (sock != INVALID_SOCKET) {
                if (M_CONNECT_LOST(id) == FALSE) {
                    int num = send(sock, (char*)data, offset, 0);
                    if (num <= 0) {
                        WriteLog(LOG_ERROR, _T("��Ϸ���ڹ㲥��Ϣʱ��������Ϣ���ͻ���[ID: %d]ʧ��"), id);
                    }
                }
            }

            // �����ӵ��Թ��ߣ���������û����ң��������û���Թ��ߣ�
            for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
                id = M_TABLE_LOOKON_ID(table, seat)[i];

                if (id != INVALID_USER_ID) {
                    SOCKET sock = M_CONNECT_SOCK(id);
                    if (sock != INVALID_SOCKET) {
                        if (M_CONNECT_LOST(id) == FALSE) {
                            int num = send(sock, (char*)data, offset, 0);
                            if (num <= 0) {
                                WriteLog(LOG_ERROR, _T("��Ϸ���ڹ㲥��Ϣʱ��������Ϣ���ͻ���[ID: %d]ʧ��"), id);
                            }
                        }
                    }
                }
            }
        }
    }

    LocalFree(data);
}

// ����Ϸ���ڹ㲥ĳ����ҵ�״̬��Ϣ
// ������뿪��Ϸ�����Թ����뿪��Ϸ�������׼������)
static void BroadcastInfoInTable(int id, int table, int seat, BYTE msgcode)
{
    int len = 0;

    BYTE data[16] = { 0 };
    WORD crc = 0;

    //
    // 2009-06-08 �޸���������Ϣ�ĸ�ʽ�����ĵ�����Ҫ���·����Ϣ�壬���޸Ĵ������¡�
    // ����һ��Թ����뿪����������֪ͨ��Ϸ����������Ա�����׼����������������֪ͨ
    // ��Ϸ�������г�Ա����
    //
    switch (msgcode) {
        case NMSG_ACK_GAMER_LEAVE_SEAT:
        case NMSG_ACK_GAMER_READY:
            data[0] = msgcode;
            data[1] = 0;
            data[2] = 2; // ��Ϣ�峤��Ϊ2�ֽ�

            data[3] = (BYTE)table;
            data[4] = (BYTE)seat;

            crc = CRC16(data, 5);
            data[5] = (BYTE)(crc >> 8);
            data[6] = (BYTE)(crc >> 0);

            len = 7; // Ҫ���͵����ݳ���
            break;

        case NMSG_ACK_LOOKON_LEAVE_SEAT:
            data[0] = msgcode;
            data[1] = 0;
            data[2] = 4; // ��Ϣ�峤��Ϊ4�ֽ�

            data[3] = (BYTE)table;
            data[4] = (BYTE)seat;
            data[5] = (BYTE)(id >> 8);
            data[6] = (BYTE)(id >> 0);

            crc = CRC16(data, 7);
            data[7] = (BYTE)(crc >> 8);
            data[8] = (BYTE)(crc >> 0);

            len = 9; // Ҫ���͵����ݳ���
            break;

        default:
            // No other type of message supported.
            return;
    }

    // ����Ϸ���й㲥������Ա
    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        int gamerid = M_TABLE_PLAYER_ID(table, i);

        if (gamerid != INVALID_USER_ID) {
            SOCKET sock = M_CONNECT_SOCK(gamerid);
            if (sock != INVALID_SOCKET) {
                if (M_CONNECT_LOST(id) == FALSE) {
                    int num = send(sock, (char*)data, len, 0);
                    if (num <= 0) {
                        WriteLog(LOG_ERROR, _T("��Ϸ���ڹ㲥��Ϣʱ��������Ϣ���ͻ���[ID: %d]ʧ��"), gamerid);
                    }
                }
            }
        }

        // ����ҵ��Թ���
        //��������뿪����������û����ң������������Թ��ߣ�����ҽ��룬��������������������ID��
        for (int j = 0; j < MAX_LOOKON_NUM_PER_SEAT; j++) {
            int lookonid = M_TABLE_LOOKON_ID(table, i)[j];

            if (lookonid != INVALID_USER_ID) {
                SOCKET sock = M_CONNECT_SOCK(lookonid);
                if (sock != INVALID_SOCKET) {
                    if (M_CONNECT_LOST(id) == FALSE) {
                        int num = send(sock, (char*)data, len, 0);
                        if (num <= 0) {
                            WriteLog(LOG_ERROR, _T("��Ϸ���ڹ㲥��Ϣʱ��������Ϣ���ͻ���[ID: %d]ʧ��"), lookonid);
                        }
                    }
                }
            }
        }
    }
}

// ��Ӧ��Ϣ���ͻ������������λ��
static void OnMsgReqGamerTakeSeat(int id, char* msg, int len)
{
    int offset = 0;
    BYTE* data = (BYTE*)msg;

    offset++; // ������Ϣ��

    int nMsgBodyLen = (data[offset] << 8) | data[offset + 1];
    if (nMsgBodyLen != len - 3) { // ��ȥ1�ֽ���Ϣ���2�ֽڵ���Ϣ�峤��
        WriteLog(LOG_ERROR, _T("NMSG_REQ_GAMER_TAKE_SEAT ��Ϣ�ṹ����ȷ"));
        return;
    }
    offset += 2;

    int table = data[offset];
    IF_INVALID_TABLE_RET(table);
    offset++;

    int seat = data[offset];
    IF_INVALID_SEAT_RET(seat);
    offset++;

    BOOL bSeatAvailable = FALSE;
    if (IS_INVALID_USER_ID(M_TABLE_PLAYER_ID(table, seat))) { // ��λ��
        bSeatAvailable = TRUE;
    } else {
        bSeatAvailable = FALSE;
    }

    if (bSeatAvailable == FALSE) { // ��������
        BYTE msgRet[8];
        msgRet[0] = NMSG_ACK_GAMER_TAKE_SEAT;
        msgRet[1] = 0;
        msgRet[2] = 3; // ��Ϣ�峤��Ϊ3
        msgRet[3] = (BYTE)bSeatAvailable;
        msgRet[4] = (BYTE)table;
        msgRet[5] = (BYTE)seat;

        // ׷��CRC�ֶ�
        WORD crc = CRC16(msgRet, 6);
        msgRet[6] = (BYTE)(crc >> 8);
        msgRet[7] = (BYTE)(crc >> 0);

        SOCKET sock = M_CONNECT_SOCK(id);
        int nSend = send(sock, (char*)msgRet, 8, 0);
        if (nSend <= 0) {
            WriteLog(LOG_ERROR, _T("NMSG_ACK_GAMER_TAKE_SEAT ��Ϣ����ʧ��"));
            return;
        }
    } else { // ������������������ݽṹ��ͬʱ֪ͨ��Ӧ����Ϸ�������߳�
        M_CONNECT_STATE(id)= STATE_SIT;
        M_CONNECT_TABLE(id)= table;
        M_CONNECT_SEAT(id)= seat;
        M_TABLE_PLAYER_ID(table, seat) = id;

        for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
            M_TABLE_LOOKON_ID(table, seat)[i] = INVALID_USER_ID;
        }

        // ��Ϸ���ڹ㲥
        ExchangeInfoInTable(id, table, seat, NMSG_ACK_GAMER_TAKE_SEAT);

        // ״̬�ı�
        AddUserStateChangeEvent(id, EVT_CHGSTATE, STATE_SIT, FALSE);

        // ���̸߳���㲥��Ϣ����Ϸ��������Ա��Ȼ��֪ͨ��Ϸ�����߳�
        PostThreadMessage(g_nGameThreadIds[table], TM_GAMER_ENTER, (WPARAM)id, (LPARAM)seat);
    }

    if (bSeatAvailable == TRUE) {
        WriteLog(LOG_INFO, _T("����: ��� [ID: %d] ����� %d ���� %d ��λ��"), id, table, seat);
    } else {
        WriteLog(LOG_INFO, _T("�ܾ�: ��� [ID: %d] ����� %d ���� %d ��λ�ӡ���� [ID: %d] �Ѿ��������λ��"),
            id, table, seat, M_TABLE_PLAYER_ID(table, seat));
    }
}

// ��Ӧ��Ϣ���ͻ�����뿪λ��
static void OnMsgReqGamerLeaveSeat(int id, char* msg, int len)
{
    int offset = 0;
    BYTE* data = (BYTE*)msg;

    offset++; // ������Ϣ��

    int nMsgBodyLen = (data[offset] << 8) | data[offset + 1];
    if (nMsgBodyLen != len - 3) { // ��ȥ1�ֽ���Ϣ���2�ֽڵ���Ϣ�峤��
        WriteLog(LOG_ERROR, _T("NMSG_GAMER_LEAVE_SEAT ��Ϣ�ṹ����ȷ"));
        return;
    }
    offset += 2;

    int table = data[offset];
    IF_INVALID_TABLE_RET(table);
    offset++;

    int seat = data[offset];
    IF_INVALID_SEAT_RET(seat);
    offset++;

    // �������ݽṹ
    M_CONNECT_STATE(id)= STATE_IDLE;
    M_TABLE_PLAYER_ID(table, seat) = INVALID_USER_ID;

    // ����뿪��Ϸ����֪ͨ��Ϸ��������Ա
    BroadcastInfoInTable(id, table, seat, NMSG_ACK_GAMER_LEAVE_SEAT);

    // ״̬�ı�
    AddUserStateChangeEvent(id, EVT_CHGSTATE, STATE_IDLE, FALSE);

    // ���̸߳���㲥��Ϣ����Ϸ��������Ա��Ȼ��֪ͨ��Ϸ�����߳�
    PostThreadMessage(g_nGameThreadIds[table], TM_GAMER_LEAVE, (WPARAM)id, (LPARAM)seat);

    WriteLog(LOG_INFO, _T("��� [ID: %d] �뿪�� %d ���� %d ��λ��"), id, table, seat);
}

// ��Ӧ��Ϣ����������Թ�
static void OnMsgReqLookonTakeSeat(int id, char* msg, int len)
{
    //
    // �ͻ�������Ƚ����жϣ������Ϸ������ò������Թ�ѡ���ͻ������������
    // �����Թ۵���Ϣ����������ֱ���ڿͻ�����ʾ�������Թ�
    //
    int offset = 0;
    BYTE* data = (BYTE*)msg;

    offset++; // ������Ϣ��

    int nMsgBodyLen = (data[offset] << 8) | data[offset + 1];
    if (nMsgBodyLen != len - 3) { // ��ȥ1�ֽ���Ϣ���2�ֽڵ���Ϣ�峤��
        WriteLog(LOG_ERROR, _T("NMSG_REQ_LOOKON_TAKE_SEAT ��Ϣ�ṹ����ȷ"));
        return;
    }
    offset += 2;

    int table = data[offset];
    IF_INVALID_TABLE_RET(table);
    offset++;

    int seat = data[offset];
    IF_INVALID_SEAT_RET(seat);
    offset++;

    // ��Ҫ�Թ��ĸ����
    int nPlayerID = M_TABLE_PLAYER_ID(table, seat);

    // �����Թ۵�ԭ��
    int nNoLookonReason = 0;
    BOOL bCanLookon = FALSE;
    if (IS_INVALID_USER_ID(nPlayerID)) { // ��λ��
        // �ͻ��˵����ݿ���û��ʱ�������ͬ�������ԣ����ܻ�������������Թ�һ�����뿪�����
        bCanLookon = FALSE;
        nNoLookonReason = 0;
    } else if (M_CONNECT_ALLOW_LOOKON(nPlayerID)== FALSE) {
        bCanLookon = FALSE;
        nNoLookonReason = 1;
    } else {
        bCanLookon = TRUE;
    }

    int nLookonIndex = -1;

    // �ж�һ���Թ������Ƿ�����
    for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
        if (IS_INVALID_USER_ID(M_TABLE_LOOKON_ID(table, seat)[i])) {
            nLookonIndex = i;
            break;
        }
    }

    if (nLookonIndex == -1) { // �Թ�λ������
        bCanLookon = FALSE;
        nNoLookonReason = 2;
    }

    if (bCanLookon == FALSE) { // �����Թ�
        BYTE msgRet[8];
        msgRet[0] = NMSG_ACK_LOOKON_TAKE_SEAT;
        msgRet[1] = 0;
        msgRet[2] = 3; // ��Ϣ�峤��Ϊ3
        msgRet[3] = (BYTE)bCanLookon;
        msgRet[4] = (BYTE)table;
        msgRet[5] = (BYTE)seat;

        // ׷��CRC�ֶ�
        WORD crc = CRC16(msgRet, 6);
        msgRet[6] = (BYTE)(crc >> 8);
        msgRet[7] = (BYTE)(crc >> 0);

        SOCKET sock = M_CONNECT_SOCK(id);
        int nSend = send(sock, (char*)msgRet, 8, 0);
        if (nSend <= 0) {
            WriteLog(LOG_ERROR, _T("NMSG_ACK_LOOKON_TAKE_SEAT ��Ϣ����ʧ��"));
            return;
        }
    } else { // �������ݽṹ��ͬʱ֪ͨ��Ӧ����Ϸ�������߳�
        M_CONNECT_STATE(id) = STATE_LOOKON;
        M_CONNECT_TABLE(id)= table;
        M_CONNECT_SEAT(id)= seat;
        M_TABLE_LOOKON_ID(table, seat)[nLookonIndex] = id;

        // ��Ϸ���ڹ㲥
        ExchangeInfoInTable(id, table, seat, NMSG_ACK_LOOKON_TAKE_SEAT);

        // ״̬�ı�
        AddUserStateChangeEvent(id, EVT_CHGSTATE, STATE_LOOKON, FALSE);

        // ���̸߳���㲥��Ϣ����Ϸ��������Ա��Ȼ��֪ͨ��Ϸ�����߳�
        PostThreadMessage(g_nGameThreadIds[table], TM_LOOKON_ENTER, (WPARAM)id, (LPARAM)seat);
    }

    if (bCanLookon == TRUE) {
        WriteLog(LOG_INFO, _T("����: ��� [ID: %d] �ڵ� %d ���� %d ��λ���Թ���� [ID: %d]"), id, table, seat, nPlayerID);
    } else if (nNoLookonReason == 0) {
        WriteLog(LOG_INFO, _T("�ܾ�: ��� [ID: %d] �ڵ� %d ���� %d ��λ���Թ���� [ID: %d]����Ϊ������Ѿ��뿪!"),
            id, table, seat, nPlayerID);
    } else if (nNoLookonReason == 1) {
        WriteLog(LOG_INFO, _T("�ܾ�: ��� [ID: %d] �ڵ� %d ���� %d ��λ���Թ���� [ID: %d]����Ϊ���������Ϊ�������Թ�!"),
            id, table, seat, nPlayerID);
    } else if (nNoLookonReason == 2) {
        WriteLog(LOG_INFO, _T("�ܾ�: ��� [ID: %d] �ڵ� %d ���� %d ��λ���Թ���� [ID: %d]����Ϊ�Թ۸���ҵ������Ѿ��ﵽ���ֵ!"),
            id, table, seat, nPlayerID);
    }
}

// ��Ӧ��Ϣ���Թ�����ϯ
static void OnMsgReqLookonLeaveSeat(int id, char* msg, int len)
{
    int offset = 0;
    BYTE* data = (BYTE*)msg;

    offset++; // ������Ϣ��

    int nMsgBodyLen = (data[offset] << 8) | data[offset + 1];
    if (nMsgBodyLen != len - 3) { // ��ȥ1�ֽ���Ϣ���2�ֽڵ���Ϣ�峤��
        WriteLog(LOG_ERROR, _T("NMSG_REQ_LOOKON_LEAVE_SEAT ��Ϣ�ṹ����ȷ"));
        return;
    }
    offset += 2;

    int table = data[offset];
    offset++;

    int seat = data[offset];
    offset++;

    // �������ݽṹ
    M_CONNECT_STATE(id) = STATE_IDLE;

    for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
        if (M_TABLE_LOOKON_ID(table, seat)[i] == id) {
            M_TABLE_LOOKON_ID(table, seat)[i] = INVALID_USER_ID;
            break;
        }
    }

    // �Թ����뿪��Ϸ����֪ͨ��Ϸ��������Ա
    BroadcastInfoInTable(id, table, seat, NMSG_ACK_LOOKON_LEAVE_SEAT);

    // ״̬�ı�
    AddUserStateChangeEvent(id, EVT_CHGSTATE, STATE_IDLE, FALSE);

    WriteLog(LOG_INFO, _T("�Թ���� [ID: %d] �뿪�� %d ���� %d ��λ��"), id, table, seat);

    // ���̸߳���㲥��Ϣ����Ϸ��������Ա��Ȼ��֪ͨ��Ϸ�����߳�
    PostThreadMessage(g_nGameThreadIds[table], TM_LOOKON_LEAVE, (WPARAM)id, (LPARAM)seat);
}

// ��Ӧ��Ϣ�����׼���������������������
static void OnMsgReqGamerReady(int id, char* msg, int len)
{
    int offset = 0;
    BYTE* data = (BYTE*)msg;

    if (M_CONNECT_STATE(id) != STATE_SIT) {
        WriteLog(LOG_ERROR, _T("��Ҳ���׼����Ϸ����״̬����ȷ���ͻ��˷��Ͳ���ȷ����Ϣ��������"));
        return;
    }

    offset++; // ������Ϣ��

    int nMsgBodyLen = (data[offset] << 8) | data[offset + 1];
    if (nMsgBodyLen != len - 3) { // ��ȥ1�ֽ���Ϣ���2�ֽڵ���Ϣ�峤��
        WriteLog(LOG_ERROR, _T("NMSG_REQ_GAMER_READY ��Ϣ�ṹ����ȷ"));
        return;
    }
    offset += 2;

    int table = data[offset];
    IF_INVALID_TABLE_RET(table);
    offset++;

    int seat = data[offset];
    IF_INVALID_SEAT_RET(seat);
    offset++;

    // �������ݽṹ
    M_CONNECT_STATE(id)= STATE_READY;

    // ���׼��������֪ͨ��Ϸ��������Ա
    BroadcastInfoInTable(id, table, seat, NMSG_ACK_GAMER_READY);

    // ״̬�ı�
    AddUserStateChangeEvent(id, EVT_CHGSTATE, STATE_READY, FALSE);

    WriteLog(LOG_INFO, _T("��� [ID: %d] �ڵ� %d ���� %d ��λ��׼������"), id, table, seat);

    // ���̸߳���㲥��Ϣ����Ϸ��������Ա��Ȼ��֪ͨ��Ϸ�����߳�
    PostThreadMessage(g_nGameThreadIds[table], TM_GAMER_READY, (WPARAM)id, (LPARAM)seat);
}

//�����յ�����Ϣת������Ϸ�����̡߳���Ϸ�����̴߳�������Ϣ����Ҫ�����ͷ��ڴ�
static void PostMsgDataToGameThread(int id, char* buf, int len)
{
    int table = M_CONNECT_TABLE(id);

    LPVOID lpMem = HeapAlloc(GetProcessHeap(), 0, len);
    if (lpMem == NULL) {
        WriteLog(LOG_ERROR, _T("�ӽ��̵Ķ��з������ڱ���������ݵ��ڴ�ʧ��: %d"), len);
        return;
    }

    CopyMemory(lpMem, buf, len);

    WPARAM wp = MAKEWPARAM(len, id);
    LPARAM lp = (LPARAM)lpMem;

    PostThreadMessage(g_nGameThreadIds[table], TM_RECEIVE_DATA, wp, lp);
}

// �������ݵ��ͻ���
int SendData(int id, char* buf, int len)
{
    int num = -1;
    SOCKET sock = INVALID_SOCKET;

    if (!IS_INVALID_USER_ID(id)) {
        sock = M_CONNECT_SOCK(id);
        if (sock != INVALID_SOCKET) {
            if (M_CONNECT_LOST(id) == FALSE) {
                num = send(sock, buf, len, 0);
            }
        }
    }

    return num;
}

