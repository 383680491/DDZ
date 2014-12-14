//
// NetProc.h
//
//
#pragma once

// ���ͻ��˵��߻�ͻ��������Ͽ�����ʱ�������߳��Զ��˳�
// �����̷߳��͸���Ϣ�����̣߳������̸߳���رո������̵߳ľ��
#define TM_CLIENT_DISCONNECT                (WM_USER + 301)

// ��������ʱ�㲥�ͻ�״̬��ʱ����
#define BROADCAST_TIME_INTERVAL             1000

// ÿ�����Ӷ�Ԥ����һ���������ݵĻ�������Ϊ���ⱨ�ĵĶ�ʧ���ݶ�Ϊ4K
#define CONNECTION_RECV_BUF_SZ              4096

// ��������INI�ļ�
#define SERVER_CONF_FILE                _T("DdzServer.ini")
#define SVR_INI_SEC_SERVER              _T("server")
#define SVR_INI_KEY_PORT                _T("port")


// ��Ϸ�����Ƿ�Ϸ���������Ϣʱ�����жϣ�����������Բ��ж�
#define IF_INVALID_TABLE_RET(table) \
    if (IS_INVALID_TABLE(table)) { \
        WriteLog(LOG_ERROR, _T("��Ϸ���� [%d] ���Ϸ������������Ϣ"), (table)); \
        return; \
    }

// ��Ϸ�������Ӻ��Ƿ�Ϸ���������Ϣʱ�����жϣ�����������Բ��ж�
#define IF_INVALID_SEAT_RET(seat) \
    if (IS_INVALID_SEAT(seat)) { \
        WriteLog(LOG_ERROR, _T("��Ϸ���Ӻ� [%d] ���Ϸ������������Ϣ"), (seat)); \
        return; \
    }

#define M_CONNECT_SOCK(id)              g_ConnectionInfo[id].sock
#define M_CONNECT_LOST(id)              g_ConnectionInfo[id].bConnLost
#define M_CONNECT_TIME(id)              g_ConnectionInfo[id].time
#define M_CONNECT_STATE(id)             g_ConnectionInfo[id].state
#define M_CONNECT_ALLOW_LOOKON(id)      g_ConnectionInfo[id].bAllowLookon
#define M_CONNECT_TABLE(id)             g_ConnectionInfo[id].table
#define M_CONNECT_SEAT(id)              g_ConnectionInfo[id].seat
#define M_CONNECT_PALYER_AVAILABLE(id)  g_ConnectionInfo[id].bPlayerInfoAvail
#define M_CONNECT_PLAYER_NAME(id)       g_ConnectionInfo[id].playerInfo.name
#define M_CONNECT_PLAYER_GENDER(id)     g_ConnectionInfo[id].playerInfo.gender
#define M_CONNECT_PLAYER_IMG_IDX(id)    g_ConnectionInfo[id].playerInfo.imgIndex
#define M_CONNECT_PLAYER_SCORE(id)      g_ConnectionInfo[id].playerInfo.nScore
#define M_CONNECT_PLAYER_TOTALGAMES(id) g_ConnectionInfo[id].playerInfo.nTotalGames
#define M_CONNECT_PLAYER_WINGAMES(id)   g_ConnectionInfo[id].playerInfo.nWinGames
#define M_CONNECT_PLAYER_RUNAWAY(id)    g_ConnectionInfo[id].playerInfo.nRunawayTimes

#define M_TABLE_PLAYER_ID(table, seat)  g_GameTableInfo[table].seatInfo[seat].playerid
#define M_TABLE_LOOKON_ID(table, seat)  g_GameTableInfo[table].seatInfo[seat].lookonid

// ID ��Դ����1-bit��ʾһ��ID
typedef struct CONNECTION_ID_RES_t {
    BYTE idResource[(MAX_CONNECTION_NUM + 7) / 8];
} CONNECTION_ID_RES, *LPCONNECTION_ID_RES;


// �ͻ�����ҵĻ�����Ϣ
typedef struct PLAYER_BASIC_INFO_t {
    TCHAR               name[MAX_USER_NAME_LEN];    // �ǳ�
    PLAYER_GENDER       gender;                     // �Ա�
    int                 imgIndex;                   // ͷ��ͼƬ����

    int                 nScore;         // ����
    int                 nTotalGames;    // ����Ϸ����
    int                 nWinGames;      // Ӯ�ľ���
    int                 nRunawayTimes;  // ���ܴ���
} PLAYER_BASIC_INFO, *LPPLAYER_BASIC_INFO;

// �ͻ���������Ϣ
typedef struct CONNECTION_INFO_t {
    SOCKET              sock;                   // �ͻ������ӵ��׽���
    BOOL                bConnLost;              // ��������¼�ÿͻ����Ƿ��Ѿ�����
    BOOL                bPlayerInfoAvail;       // ��ʶ�ÿͻ�����ҵ���Ϣ�Ƿ����
    SYSTEMTIME          time;                   // ���ӽ���ʱ��
    PLAYER_BASIC_INFO   playerInfo;             // ��Ϸ��һ�����Ϣ
    PLAYER_STATE        state;                  // ���״̬
    BOOL                bAllowLookon;           // �������Թۣ�Ĭ��Ϊ1��TRUE������ʾ�����Թ�
    int                 table;                  // ����������Ӻ�
    int                 seat;                   // ���������λ�����ӣ���
} CONNECTION_INFO, *LPCONNECTION_INFO;

// ��Ϸ���е�������Ϣ
typedef struct GAME_SEAT_INFO_t {
    int playerid;                               // ��Ϸ��ҵ�ID
    int lookonid[MAX_LOOKON_NUM_PER_SEAT];      // ����Ϸ��ҵ��Թ���ID
} GAME_SEAT_INFO, *LPGAME_SEAT_INFO;

// ��Ϸ����Ϣ
typedef struct GAME_TABLE_INFO_t {
    GAME_SEAT_INFO      seatInfo[GAME_SEAT_NUM_PER_TABLE]; // ��������
    //DWORD               dwThreadId;             // ��Ӧ����Ϸ�����߳�ID
    //BOOL                bAllowLookon;           // �����Թ�
    //BOOL                bHasPassword;           // ����ʱ��Ҫ����
    //TCHAR               password[6];            // ����
} GAME_TABLE_INFO, *LPGAME_TABLE_INFO;

// �����ͻ��˽�����뿪����
typedef struct USER_STATE_CHANGE_t {
    int                                 id;     // �ͻ��˵�ΨһID
    STATE_CHANGE_EVENT                  evt;    // ���״̬�ı��¼����ͣ�������� MsgDef.h
    struct USER_STATE_CHANGE_t*         next;   // ������¸����

    PLAYER_STATE                        newState; // �ı�֮���״̬���������� EVT_STATE_CHG
    BOOL                                bContainStatistics; // �Ƿ���ҪЯ�����ͳ����Ϣ���������� EVT_STATE_CHG
} USER_STATE_CHANGE, *LPUSER_STATE_CHANGE;

// ����ṹ����¼��ҽ��뷿�䡢�뿪���䡢״̬�����ı�
typedef struct USER_STATE_CHANGE_LIST_t {
    USER_STATE_CHANGE*          lpFirstNode;    // �׽��
    USER_STATE_CHANGE*          lpLastNode;     // β���
} USER_STATE_CHANGE_LIST, *LPUSER_STATE_CHANGE_LIST;

// ����Ϸ�����߳��ṩ���ⲿȫ�ֱ�������
extern volatile int g_nCurConnectionNum;
extern CRITICAL_SECTION g_csConnectionNum;
extern CONNECTION_INFO g_ConnectionInfo[];
extern GAME_TABLE_INFO g_GameTableInfo[];


// ���������繦��
BOOL StartNetService(void);
void StopNetService(void);

void CloseConnectionThreadHandle(HANDLE hThread);

static void ReleaseNetServiceRes(void);
static BOOL InitNetServiceRes(void);
static BOOL InitWinSocket(void);

static BOOL InitServerIP(void);
static BOOL InitServerPort(void);
static void WriteServerPortToIni(void);

void GetHostIP(DWORD ipArray[], int num, int* count);

// IP address selection dialog procedure
INT_PTR CALLBACK IpSelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR FinishIpSel(HWND hDlg, int iItem);
INT_PTR IpListNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);
INT_PTR InitIpSelDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);

// �ȴ������̵߳���ں���
DWORD WaitForConnectionThreadProc(LPVOID lpData);

// �����̵߳���ں���
DWORD ConnectionThreadProc(LPVOID lpData);

// ��ʱ�㲥�̵߳���ں���
DWORD OnTimeBroadcastThreadProc(LPVOID lpData);

// ��ͻ��˷������ݵ��ⲿ�ӿ�
int SendData(int id, char* buf, int len);

int AllocID(void);
void DeallocID(int id);

void AddUserStateChangeEvent(int id, STATE_CHANGE_EVENT evt, PLAYER_STATE newState, BOOL bContainStat);

static void SendRoomInfo(int nAllocID);
static void ExchangeInfoInTable(int myid, int table, int seat, BYTE msgcode);
static void BroadcastInfoInTable(int myid, int table, int seat, BYTE msgcode);

// ��ʱ�������״̬�仯����Ϣ�����㲥��ȥ
static void OnTimeBuildMessageAndBroadcast(char* mem, int len);

// ��Ӧ���յ�����Ϣ
static void OnMessage(int id, char* msg, int len);

static void OnMsgReqRoomInfo(int id, char* msg, int len);
static void OnMsgReqGamerTakeSeat(int id, char* msg, int len);
static void OnMsgReqGamerLeaveSeat(int id, char* msg, int len);
static void OnMsgReqLookonTakeSeat(int id, char* msg, int len);
static void OnMsgReqLookonLeaveSeat(int id, char* msg, int len);
static void OnMsgReqGamerReady(int id, char* msg, int len);

// �����յ�����Ϣת������Ϸ�����߳�
static void PostMsgDataToGameThread(int id, char* buf, int len);

