//
// GameProc.cpp
//
//  ��������Ϸ�������߼�
//

#include "stdafx.h"
#include "QuickSort.h"
#include "PokerAlgor.h"
#include "Crc.h"
#include "NetProc.h"
#include "GameProc.h"

extern POKER_ALOGRITHM g_PokerAlgorithm[];
extern volatile int g_nCurPokerAlgorithm;

// ����һ��TLS����
DWORD g_dwGameTlsIndex = 0;

// ��Ϸ�����߳̾����ID��һ����Ϸ����Ӧһ����Ϸ�����߳�
HANDLE g_hGameThreads[GAME_TABLE_NUM] = { 0 };
DWORD g_nGameThreadIds[GAME_TABLE_NUM] = { 0 };

// ��Ϸ������Ϣ��ÿ����Ϸ�����߳�ʹ������һ��
GAME_INFO g_GameInfo[GAME_TABLE_NUM] = { 0 };

#define M_GAME_STARTED(table)                   g_GameInfo[table].bStartGame
#define M_GAME_OUTPUT_CARD_STARTED(table)       g_GameInfo[table].bStartOuputCard
#define M_GAME_MULTIPLE(table)                  g_GameInfo[table].nMultiple
#define M_GAME_LORD_SEAT(table)                 g_GameInfo[table].nLordSeat
#define M_GAME_LORD_SCORE(table)                g_GameInfo[table].nLordScore
#define M_GAME_WINNER_SEAT(table)               g_GameInfo[table].nWinnerSeat
#define M_GAME_UP_NOT_FOLLOW(table)             g_GameInfo[table].bUpPlayerNotFollow
#define M_GAME_LORD_OUTPUT_TIMES(table)         g_GameInfo[table].nLordOutputTimes
#define M_GAME_VOTE_LORD_FAIL_TIMES(table)      g_GameInfo[table].nVoteLordFailTimes
#define M_GAME_ACTIVE_SEAT(table)               g_GameInfo[table].nActiveSeat
#define M_GAME_CONNECTION_LOST(table, seat)     g_GameInfo[table].bConnectionLost[seat]
#define M_GAME_READY(table, seat)               g_GameInfo[table].bReady[seat]
#define M_GAME_VOTED(table, seat)               g_GameInfo[table].bVoted[seat]
#define M_GAME_DELEGATED(table,seat)            g_GameInfo[table].bDelegated[seat]
#define M_GAME_UNDER_POKER_CARDS(table)         g_GameInfo[table].underPokerCards
#define M_GAME_PLAYER_POKER_NUM(table, seat)    g_GameInfo[table].pokerCards[seat].num
#define M_GAME_PLAYER_POKERS(table, seat)       g_GameInfo[table].pokerCards[seat].pokers
#define M_GAME_PLAYER_ID(table, seat)           g_GameInfo[table].seatInfo[seat].playerid
#define M_GAME_LOOKON_ID(table, seat)           g_GameInfo[table].seatInfo[seat].lookonid
#define M_GAME_OUTPUT_POKER_TYPE(table)         g_GameInfo[table].outputPokerProperty.type
#define M_GAME_OUTPUT_POKER_VALUE(table)        g_GameInfo[table].outputPokerProperty.value
#define M_GAME_OUTPUT_POKER_NUM(table)          g_GameInfo[table].outputPokerProperty.num
#define M_GAME_SAVE_POKER_PROPERTY(table, _t, _v, _n) \
{\
    M_GAME_OUTPUT_POKER_TYPE(table) = (POKER_TYPE)_t;\
    M_GAME_OUTPUT_POKER_VALUE(table) = _v;\
    M_GAME_OUTPUT_POKER_NUM(table) = _n;\
}


// ������Ϸ�����߳�
BOOL BeginGameThreads(void)
{
    // Set Low-Fragmentation Heap
    ULONG  HeapFragValue = 2;
    HeapSetInformation(GetProcessHeap(),
        HeapCompatibilityInformation,
        &HeapFragValue,
        sizeof(HeapFragValue));

    // ��ʼ����Ϸ������
    ZeroMemory(&g_GameInfo, sizeof(GAME_INFO) * GAME_TABLE_NUM);

    for (int table = 0; table < GAME_TABLE_NUM; table++) {
        M_GAME_LORD_SEAT(table) = INVALID_SEAT;
        M_GAME_WINNER_SEAT(table) = INVALID_SEAT;
        M_GAME_ACTIVE_SEAT(table) = INVALID_SEAT;

        for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
            M_GAME_PLAYER_ID(table, seat) = INVALID_USER_ID;

            for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
                M_GAME_LOOKON_ID(table, seat)[i] = INVALID_USER_ID;
            }
        }
    }

    // ����һ��TLS������������Ϸ�����̱߳������������ţ��Ա��߳�ʶ���Լ��ǵڼ����Ĺ����߳�
    if ((g_dwGameTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
        return FALSE;
    }

    // ������Ϸ�����̣߳����ݲ���i���̣߳���ʾ���߳��ǵڼ�������Ϸ�����߳�
    for (int i = 0; i < sizeof(g_nGameThreadIds) / sizeof(g_nGameThreadIds[0]); i++) {
        g_hGameThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GameThreadProc,
            (LPVOID)i, 0, &g_nGameThreadIds[i]);

        if (g_hGameThreads[i] == NULL) {
            for (int j = 0; j < i; j++) {
                PostThreadMessage(g_nGameThreadIds[j], TM_QUIT, 0, 0);
            }

            for (int j = 0; j < i; j++) {
                WaitForSingleObject(g_hGameThreads[j], INFINITE);
                CloseHandle(g_hGameThreads[j]);
            }

            TlsFree(g_dwGameTlsIndex);

            return FALSE;
        }
    }

    return TRUE;
}

// ������Ϸ�߳�
BOOL EndGameThreads(void)
{
    for (int i = 0;
        i < sizeof(g_nGameThreadIds) / sizeof(g_nGameThreadIds[0]);
        i++) {
            PostThreadMessage(g_nGameThreadIds[i], TM_QUIT, 0, 0);
    }

    for (int i = 0;
        i < sizeof(g_hGameThreads) / sizeof(g_hGameThreads[0]);
        i++) {
            WaitForSingleObject(g_hGameThreads[i], INFINITE);
            CloseHandle(g_hGameThreads[i]);
    }

    TlsFree(g_dwGameTlsIndex);

    return TRUE;
}

// ��Ϸ�����̵߳���ں���
// ���̵߳Ĺ����Ǹ�����Ϸ���������߼��ͻỰ
DWORD GameThreadProc(LPVOID lpData)
{
    UINT nMsg;
    WPARAM wParam;
    LPARAM lParam;
    int id;
    int seat;
    int len;
    BYTE* data;

    // �����߳�ʱ�����ݵĲ�������Ϸ���������ţ�����ʾ���߳��ǵڼ�������Ϸ�����̡߳�
    // ���䱣�����̵߳�TLS�У��Ա���ʱ����ȡ����ʹ��

    int table = int(lpData);
    if (TlsSetValue(g_dwGameTlsIndex, (LPVOID)table) == 0) {
        return 1;
    }

    int ret;
    MSG msg;

    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (ret == -1) { continue; }

        nMsg = msg.message;
        wParam = msg.wParam;
        lParam = msg.lParam;

        switch (nMsg) {
            case TM_GAMER_ENTER:
                id = (int)wParam;
                seat = (int)lParam;
                OnGamerEnter(id, table, seat);
                break;

            case TM_GAMER_LEAVE:
                id = (int)wParam;
                seat = (int)lParam;
                OnGamerLeave(id, table, seat);
                break;

            case TM_LOOKON_ENTER:
                id = (int)wParam;
                seat = (int)lParam;
                OnLookonEnter(id, table, seat);
                break;

            case TM_LOOKON_LEAVE:
                id = (int)wParam;
                seat = (int)lParam;
                OnLookonLeave(id, table, seat);
                break;

            case TM_GAMER_READY:
                id = (int)wParam;
                seat = (int)lParam;
                OnGamerReady(id, table, seat);
                break;

            case TM_CONNECTION_LOST:
                id = (int)wParam;
                seat = (int)lParam;
                OnConnectionLost(id, table, seat);
                break;

            case TM_RECEIVE_DATA:
                id = HIWORD(wParam);
                len = LOWORD(wParam);
                data = (BYTE*)lParam;
                OnReceiveData(id, len , data);
                HeapFree(GetProcessHeap(), 0, (LPVOID)lParam);
                break;

            case TM_QUIT:
                //PostQuitMessage(0);
                //break;
                return 0;

                //
                // ������ϢΪ���߳��Լ����͸��Լ���
                // ����������Ϊ�˱�����Ϸ������������Ҷ��й���Ϸ�����ֵݹ���ƣ�
                // ���޷���ʱ����������Ϣ
                //
            case TM_START_GAME:
                StartGame(table);
                break;

            case TM_REQ_VOTE_LORD:
                ReqVoteLord(table, (int)wParam, (int)lParam);
                break;

            case TM_REQ_OUTPUT_CARD:
                {
                    POKER_PROPERTY pp;
                    int req_seat = LOWORD(wParam);
                    pp.type = (POKER_TYPE)HIWORD(wParam);
                    pp.value = LOWORD(lParam);
                    pp.num = HIWORD(lParam);
                    ReqOutputCard(table, req_seat, &pp);
                }
                break;
        }
    }

    return 0;
}

// ������������յ�������
static void OnReceiveData(int id, int len, BYTE* buf)
{
    UNREFERENCED_PARAMETER(id);

    // ���򵽴ˣ��Ѿ�ʶ���һ���Ϸ�����Ϣ��������Ϣ���ͣ�ֻ������Ϣ�塣
    int nMsgCode = buf[0];
    int nMsgBodyLen = len - 3;
    BYTE* pMsgBody = buf + 3;

    // ��ȡ���߳����ڵ���Ϸ����
    int table = (int)TlsGetValue(g_dwGameTlsIndex);

    switch (nMsgCode) {
        case NMSG_ACK_VOTE_LORD:
            OnAckVoteLord(table, pMsgBody, nMsgBodyLen);
            break;

        case NMSG_ACK_OUTPUT_CARD:
            OnAckOutputCard(table, pMsgBody, nMsgBodyLen);
            break;

        case NMSG_REQ_STOP_GAMING:
            OnReqStopGaming(table, pMsgBody, nMsgBodyLen);
            break;

        case NMSG_ACK_STOP_GAMING:
            OnAckStopGaming(table, pMsgBody, nMsgBodyLen);
            break;

        case NMSG_REQ_DELEGATE:
            OnReqDelegate(table, pMsgBody, nMsgBodyLen);
            break;

        case NMSG_REQ_FORCE_STOP_GAMING:
            OnReqForceStopGaming(table, pMsgBody, nMsgBodyLen);
            break;

        case NMSG_ALLOW_LOOKON:
            OnAllowLookon(table, pMsgBody, nMsgBodyLen);
            break;

        case NMSG_CHATTING:
            OnChatMessage(table, pMsgBody, nMsgBodyLen);
            break;

        default:
            break;
    }
}

// ��ҽ�����Ϸ��
static void OnGamerEnter(int id, int table, int seat)
{
    M_GAME_PLAYER_ID(table, seat) = id;

    // ��ո���λ�Թ���ID���Թ������������֮����ܽ����Թ�
    for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
        M_GAME_LOOKON_ID(table, seat)[i] = INVALID_USER_ID;
    }

    M_GAME_READY(table, seat) = FALSE;
    M_GAME_VOTED(table, seat) = FALSE;
    M_GAME_DELEGATED(table, seat) = FALSE;
    M_GAME_PLAYER_POKER_NUM(table, seat) = 0;
    M_GAME_CONNECTION_LOST(table, seat) = FALSE;
    M_GAME_WINNER_SEAT(table) = INVALID_SEAT;
}

// ����뿪��Ϸ��
static void OnGamerLeave(int id, int table, int seat)
{
    UNREFERENCED_PARAMETER(id);

    M_GAME_PLAYER_ID(table, seat) = INVALID_USER_ID;

    M_GAME_READY(table, seat) = FALSE;
    M_GAME_VOTED(table, seat) = FALSE;
    M_GAME_DELEGATED(table, seat) = FALSE;
    M_GAME_PLAYER_POKER_NUM(table, seat) = 0;
    M_GAME_CONNECTION_LOST(table, seat) = FALSE;
    M_GAME_WINNER_SEAT(table) = INVALID_SEAT;
}

// �Թ��߽�����Ϸ��
static void OnLookonEnter(int id, int table, int seat)
{
    // First, distribute poker to him if game started
    if (M_GAME_STARTED(table) == TRUE) {
        
        for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) { // who's delegated.
            if (M_GAME_DELEGATED(table, i) == TRUE) {
                BYTE data[16] = { 0 };
                data[0] = NMSG_ACK_DELEGATE;
                data[1] = 0;
                data[2] = 2;

                data[3] = (BYTE)i;
                data[4] = 1;

                WORD crc = CRC16(data, 5);
                data[5] = (BYTE)(crc >> 8);
                data[6] = (BYTE)(crc >> 0);

                SendData(id, (char*)data, 7);
            }
        }

        if (M_GAME_OUTPUT_CARD_STARTED(table) == TRUE) { // who's the lord.
            BYTE data[16] = { 0 };
            data[0] = NMSG_VOTE_LORD_FINISH;
            data[1] = 0;
            data[2] = 3;

            data[3] = (BYTE)TRUE;
            data[4] = (BYTE)M_GAME_LORD_SEAT(table);
            data[5] = (BYTE)M_GAME_LORD_SCORE(table);

            WORD crc = CRC16(data, 6);
            data[6] = (BYTE)(crc >> 8);
            data[7] = (BYTE)(crc >> 0);

            SendData(id, (char*)data, 8);
        }

        // distribute poker
        int ids[1];
        ids[0] = id;
        DistributePoker(table, ids, 1);
    }

    // Second, add user id to GAME data structure so that it can receive messages
    // within the game table
    for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
        if (IS_INVALID_USER_ID(M_GAME_LOOKON_ID(table, seat)[i])) {
            M_GAME_LOOKON_ID(table, seat)[i] = id;
            break;
        }
    }
}

// �Թ����뿪��Ϸ��
static void OnLookonLeave(int id, int table, int seat)
{
    for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
        if (M_GAME_LOOKON_ID(table, seat)[i] == id) {
            M_GAME_LOOKON_ID(table, seat)[i] = INVALID_USER_ID;
            break;
        }
    }
}

// ���׼����Ϸ
static void OnGamerReady(int id, int table, int seat)
{
    UNREFERENCED_PARAMETER(id);

    M_GAME_READY(table, seat) = TRUE;

    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        if (M_GAME_READY(table, i) != TRUE) {
            return;
        }
    }

    // TODO:
    int playerid;
    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        playerid = M_GAME_PLAYER_ID(table, i);
        M_CONNECT_STATE(playerid) = STATE_GAMING;
        AddUserStateChangeEvent(playerid, EVT_CHGSTATE, STATE_GAMING, FALSE);
    }

    PostMessage(NULL, TM_START_GAME, 0, 0);
}

// ��һ��Թ����������
// ��Ϸ�����߳��յ�����Ϣ������Ϊ�������ʱ���ͻ���������Ϸ���С�
// ���ͻ������Թ��ߣ�������Ϸ���ڹ㲥֪ͨ���뿪��
// ���ͻ�������Ϸ�ߣ��������ڽ�����Ϸ�������йܴ�����֪ͨ��Ϸ����Ա������ҵ��ߣ�
// ��������û��ʼ��Ϸ�������֪�����ˣ���������뿪��
static void OnConnectionLost(int id, int table, int seat)
{
    // �Թ��ߵ��ߡ���֪ͨ��Ϸ����Ա�����Թ����뿪
    if (M_CONNECT_STATE(id) == STATE_LOOKON) {
        for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
            if (id == M_GAME_LOOKON_ID(table, seat)[i]) {
                M_GAME_LOOKON_ID(table, seat)[i] = INVALID_USER_ID;
                break;
            }
        }

        for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
            if (id == M_TABLE_LOOKON_ID(table, seat)[i]) {
                M_TABLE_LOOKON_ID(table, seat)[i] = INVALID_USER_ID;
                break;
            }
        }

        BYTE data[16];
        data[0] = NMSG_ACK_LOOKON_LEAVE_SEAT;

        data[1] = 0;
        data[2] = 4;

        data[3] = (BYTE)table;
        data[4] = (BYTE)seat;
        data[5] = (BYTE)(id >> 8);
        data[6] = (BYTE)(id >> 0);

        WORD crc = CRC16(data, 7);
        data[7] = (BYTE)(crc >> 8);
        data[8] = (BYTE)(crc >> 0);
        BroadcastInTableExceptId(table, (char*)data, 9, id);

        // �뿪����
        WriteLog(LOG_INFO, _T("ID: %d �������ա�[�Թ��ߵ���]"), id);

        AddUserStateChangeEvent(id, EVT_LEAVE, (PLAYER_STATE)0, FALSE);
    } else {
        if (M_GAME_STARTED(table) == FALSE) { // ��ҵ��ߣ�����Ϸû�п�ʼ����֪������Ա��������뿪��λ
            M_GAME_PLAYER_ID(table, seat) = INVALID_USER_ID;

            M_TABLE_PLAYER_ID(table, seat) = INVALID_USER_ID;

            BYTE data[8];
            data[0] = NMSG_ACK_GAMER_LEAVE_SEAT;
            data[1] = 0;
            data[2] = 2;

            data[3] = (BYTE)table;
            data[4] = (BYTE)seat;

            WORD crc = CRC16(data, 5);
            data[5] = (BYTE)(crc >> 8);
            data[6] = (BYTE)(crc >> 0);

            BroadcastInTableExceptId(table, (char*)data, 7, id);

            // �뿪����
            WriteLog(LOG_INFO, _T("ID: %d �������ա�[��ҵ��ߣ�������Ϸ��δ��ʼ]"), id);

            AddUserStateChangeEvent(id, EVT_LEAVE, (PLAYER_STATE)0, FALSE);
        } else {
            // ��ҵ��ߣ�������Ϸ�Ѿ���ʼ��֪ͨ������Ա������ҵ���
            BYTE data[8];
            data[0] = NMSG_CONNECTION_LOST;
            data[1] = 0;
            data[2] = 1;

            data[3] = (BYTE)seat;

            WORD crc = CRC16(data, 4);
            data[4] = (BYTE)(crc >> 8);
            data[5] = (BYTE)(crc >> 0);

            BroadcastInTableExceptId(table, (char*)data, 6, id);

            // ��¼������Ѿ����ߣ�����Ϸ����ʱ����Ҫ�������������
            M_GAME_CONNECTION_LOST(table, seat) = TRUE;

            // ���������Ҷ����ߣ���ֱ�ӽ�����Ϸ
            BOOL bAllGamerLost = TRUE;
            for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
                if (M_GAME_CONNECTION_LOST(table, i) == FALSE) {
                    bAllGamerLost = FALSE;
                    break;
                }
            }

            if (bAllGamerLost == TRUE) {
                EndGame(table);
                return;
            }

            // û���йܣ������й�
            if (M_GAME_DELEGATED(table, seat) == FALSE) {
                M_GAME_DELEGATED(table, seat) = TRUE;

                BYTE data[8];
                data[0] = NMSG_ACK_DELEGATE;
                data[1] = 0;
                data[2] = 2;

                data[3] = (BYTE)seat;
                data[4] = 1; // TRUE

                WORD crc = CRC16(data, 5);
                data[5] = (BYTE)(crc >> 8);
                data[6] = (BYTE)(crc >> 0);

                BroadcastInTable(table, (char*)data, 7);

                // ���ߵ�����ǵ�ǰ�������ȴ��ظ���Ϣ�����
                if (seat == M_GAME_ACTIVE_SEAT(table)) {
                    //
                    // ������������Ϣ���Լ�������Ӧ�����򣬷������ͻ���������
                    //
                    if (M_GAME_OUTPUT_CARD_STARTED(table) == FALSE) { // ��Ϸ�Ѿ���ʼ������û��ʼ���ƣ����з���
                        PostMessage(NULL, TM_REQ_VOTE_LORD, (WPARAM)seat, (LPARAM)(M_GAME_LORD_SCORE(table)));
                    } else { // �Ѿ���ʼ����
                        WPARAM wp = MAKEWPARAM(seat, M_GAME_OUTPUT_POKER_TYPE(table));
                        LPARAM lp = MAKELPARAM(M_GAME_OUTPUT_POKER_VALUE(table), M_GAME_OUTPUT_POKER_NUM(table));
                        PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);
                    }
                }
            }
        }
    }
}

// ��Ϸ�����󣬸�λ��Ϸ���������
static void EndGame(int table)
{
    int id;

    for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
        id = M_GAME_PLAYER_ID(table, seat);

        if (M_GAME_CONNECTION_LOST(table, seat) == TRUE) {
            // ��������뿪��������Ͼ���ϷӮ�ҵ����Ӻ�
            M_GAME_WINNER_SEAT(table) = INVALID_SEAT;

            M_GAME_PLAYER_ID(table, seat) = INVALID_USER_ID;

            M_TABLE_PLAYER_ID(table, seat) = INVALID_USER_ID;

            BYTE data[8];
            data[0] = NMSG_ACK_GAMER_LEAVE_SEAT;
            data[1] = 0;
            data[2] = 2;

            data[3] = (BYTE)table;
            data[4] = (BYTE)seat;

            WORD crc = CRC16(data, 5);
            data[5] = (BYTE)(crc >> 8);
            data[6] = (BYTE)(crc >> 0);

            BroadcastInTableExceptId(table, (char*)data, 7, id);

            // �뿪����
            WriteLog(LOG_INFO, _T("ID: %d �������ա�[��ҵ��ߣ�������Ϸ�Ѿ�����]"), id);

            AddUserStateChangeEvent(id, EVT_LEAVE, (PLAYER_STATE)0, FALSE);
        } else {
            M_CONNECT_STATE(id) = STATE_SIT;
            AddUserStateChangeEvent(id, EVT_CHGSTATE, STATE_SIT, TRUE);
        }
    }

    M_GAME_STARTED(table) = FALSE;
    M_GAME_OUTPUT_CARD_STARTED(table) = FALSE;
    M_GAME_MULTIPLE(table) = 0;
    M_GAME_ACTIVE_SEAT(table) = INVALID_SEAT;
    M_GAME_LORD_SEAT(table) = INVALID_SEAT;
    M_GAME_LORD_SCORE(table) = 0;
    M_GAME_UP_NOT_FOLLOW(table) = FALSE;
    M_GAME_SAVE_POKER_PROPERTY(table, 0, 0, 0);
    M_GAME_LORD_OUTPUT_TIMES(table) = 0;
    M_GAME_VOTE_LORD_FAIL_TIMES(table) = 0;

    for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
        M_GAME_READY(table, seat) = FALSE;
        M_GAME_VOTED(table, seat) = FALSE;
        M_GAME_DELEGATED(table, seat) = FALSE;
        M_GAME_CONNECTION_LOST(table, seat) = FALSE;
        M_GAME_PLAYER_POKER_NUM(table, seat) = 0;
    }
}

// ������������ʼ��Ϸ
static void StartGame(int table)
{
    WriteLog(LOG_DEBUG, _T("�� %d ����ʼ��Ϸ"), table);

    // ��ʼ����ҵ��˿��ƣ��ڷ������ϱ������ݣ�
    InitPlayerPokerData(table);

    // �ռ���Ҫ�������ݵĿͻ���ID
    int count = 0;
    int ids[(1 + MAX_LOOKON_NUM_PER_SEAT) * GAME_SEAT_NUM_PER_TABLE];

    for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
        ids[count++] = M_GAME_PLAYER_ID(table, seat);

        for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
            if (M_GAME_LOOKON_ID(table, seat)[i] != INVALID_USER_ID) {
                ids[count++] = M_GAME_LOOKON_ID(table, seat)[i];
            }
        }
    }

    // ���ƣ����͵��ͻ��ˣ�
    DistributePoker(table, ids, count);

    // ����ѡ����
    PostMessage(NULL, TM_REQ_VOTE_LORD, (WPARAM)INVALID_SEAT, (LPARAM)0);

    M_GAME_STARTED(table) = TRUE;
}

// ��ʼ����Ҫ���͸���ҵ���
static void InitPlayerPokerData(int table)
{
    int poker[TOTAL_CARDS_NUM];

    // ϴ��
    (*g_PokerAlgorithm[g_nCurPokerAlgorithm].algorithm)(poker, TOTAL_CARDS_NUM);

    {//Debug...
        TCHAR text[256] = { 0 };
        int len = 0;
        for (int i = 0; i < TOTAL_CARDS_NUM; i++) {
            if (i % 17 == 0) {
                len += _stprintf_s(text + len, sizeof(text) / sizeof(text[0]) - len, _T("\r\n"));
            }
            len += _stprintf_s(text + len, sizeof(text) / sizeof(text[0]) - len, _T("%d "), poker[i]); 
        }
        WriteLog(LOG_DEBUG, text);
    }

    // ���ѡ��ͷ���Ż�ĩβ������Ϊ���ƣ������ѡ���ȸ��ĸ���ҷ���
    srand((unsigned)time(NULL));
    int random_underpoker = rand() % 2;
    int random_player = rand() % GAME_SEAT_NUM_PER_TABLE;

    if (random_underpoker == 0) {
        for (int i = 0; i < UNDER_CARDS_NUM; i++) { // ȡ��ͷ������Ϊ����
            M_GAME_UNDER_POKER_CARDS(table)[i] = poker[i];
        }

        for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
            M_GAME_PLAYER_POKER_NUM(table, seat) = PLAYER_INIT_CARDS_NUM;

            int start = UNDER_CARDS_NUM + random_player * PLAYER_INIT_CARDS_NUM;
            for (int i = 0; i < PLAYER_INIT_CARDS_NUM; i++) {
                M_GAME_PLAYER_POKERS(table, seat)[i] = poker[start + i];
            }

            random_player = (random_player + 1) % GAME_SEAT_NUM_PER_TABLE;
        }
    } else {
        for (int i = TOTAL_CARDS_NUM - UNDER_CARDS_NUM; i < TOTAL_CARDS_NUM; i++) { // ȡĩβ������Ϊ����
            M_GAME_UNDER_POKER_CARDS(table)[i - (TOTAL_CARDS_NUM - UNDER_CARDS_NUM)] = poker[i];
        }

        for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
            M_GAME_PLAYER_POKER_NUM(table, seat) = PLAYER_INIT_CARDS_NUM;

            int start = random_player * PLAYER_INIT_CARDS_NUM;
            for (int i = 0; i < PLAYER_INIT_CARDS_NUM; i++) {
                M_GAME_PLAYER_POKERS(table, seat)[i] = poker[start + i];
            }

            random_player = (random_player + 1) % GAME_SEAT_NUM_PER_TABLE;
        }
    }

    // ����ҷֺ��ƺ󣬶�������
    quick_sort(M_GAME_UNDER_POKER_CARDS(table), 0, UNDER_CARDS_NUM - 1);

    for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
        quick_sort(M_GAME_PLAYER_POKERS(table, seat), 0, PLAYER_INIT_CARDS_NUM - 1);
    }
}

// ���������������ҵ��Ʒ��͵��ͻ������
static void DistributePoker(int table, int ids[], int idcount)
{
    BYTE data[80];
    int offset = 0;

    data[0] = NMSG_DISTRIBUTE_CARD;

    offset = 3;

    // �����ҵ���
    for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
        data[offset] = (BYTE)M_GAME_PLAYER_POKER_NUM(table, seat); // �˿�������
        offset++;

        for (int i = 0; i < M_GAME_PLAYER_POKER_NUM(table, seat); i++) { // �˿�������
            data[offset] = (BYTE)M_GAME_PLAYER_POKERS(table, seat)[i];
            offset++;
        }
    }

    // ������
    data[offset] = (BYTE)UNDER_CARDS_NUM;
    offset++;
    for (int i = 0; i < UNDER_CARDS_NUM; i++) {
        data[offset] = (BYTE)M_GAME_UNDER_POKER_CARDS(table)[i];
        offset++;
    }

    data[1] = (BYTE)((offset - 3) >> 8);
    data[2] = (BYTE)((offset - 3) >> 0);

    // Append CRC
    WORD crc = CRC16(data, offset);
    data[offset] = (BYTE)(crc >> 8);
    data[offset + 1] = (BYTE)(crc >> 0);
    offset += 2;

    for (int i = 0; i < idcount; i++) {
        SendData(ids[i], (char*)data, offset);
    }
}

// ����ͻ��˾�ѡ����
static void ReqVoteLord(int table, int seat, int score)
{
    int req_seat = seat;
    int req_score = score;

    if (IS_INVALID_SEAT(req_seat)) { // �Ƿ�ֵ��ʾ�״νе���
        if (IS_INVALID_SEAT(M_GAME_WINNER_SEAT(table))) { // ��û��ʼ����Ϸ����û���Ͼ���Ϸ��Ӯ��
            req_seat = (rand() % 10) % 3;
            //req_seat = (int)((double)(rand()) / (double)(RAND_MAX + 1) * 3);

            WriteLog(LOG_DEBUG, _T("�� %d ����ѡ������ʼ�����ѡ�� %d �����Ƚе���"), table, req_seat);
        } else {
            req_seat = M_GAME_WINNER_SEAT(table);

            WriteLog(LOG_DEBUG, _T("�� %d ����ѡ������ʼ�����Ͼ���Ϸ��Ӯ�� [%d] �����Ƚе���"), table, req_seat);
        }

        req_score = 0;
    }

    // ��¼����������зֵ����Ӻ�
    M_GAME_ACTIVE_SEAT(table) = req_seat;

    if (M_GAME_DELEGATED(table, req_seat) == TRUE) {
        VoteLordDelegated(table, req_seat);
    } else {
        BYTE data[8];
        data[0] = NMSG_REQ_VOTE_LORD;
        data[1] = 0;
        data[2] = 2;
        data[3] = (BYTE)req_seat;
        data[4] = (BYTE)req_score;

        WORD crc;
        crc = CRC16(data, 5);
        data[5] = (BYTE)(crc >> 8);
        data[6] = (BYTE)(crc >> 0);

        BroadcastInTable(table, (char*)data, 7);
    }
}

// �й���Ϸ�������Ӧ�е������¼�
static void VoteLordDelegated(int table, int seat)
{
    // ��¼��ǰ����Ѿ��й���
    M_GAME_VOTED(table, seat) = TRUE;

    BYTE data[8];

    data[0] = NMSG_VOTE_LORD;
    data[1] = 0;
    data[2] = 2;
    data[3] = (BYTE)seat;
    data[4] = 0; // ���з�

    WORD crc = CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    // ֪ͨ������ҹ��ڸ��й���Ҳ��з�
    BroadcastInTable(table, (char*)data, 7);

    int next_seat = (seat + 1) % GAME_SEAT_NUM_PER_TABLE;
    int cur_score = M_GAME_LORD_SCORE(table);

    WriteLog(LOG_DEBUG, _T("�� %d �� %d ��������Ѿ��й���Ϸ�����з֡���ǰ�Ѿ��з���: %d ��"),
        table, seat, cur_score);

    if (M_GAME_VOTED(table, next_seat) == FALSE) { // ��һ����һ�û�з֣���֪ͨ���з�
        PostMessage(NULL, TM_REQ_VOTE_LORD, (WPARAM)next_seat, (LPARAM)cur_score);
    } else { // ������Ҷ��й�����
        if (cur_score == 0) { // ������Ҷ���0��
            VoteLordFailed(table);
        } else {
            VoteLordSucceed(table, M_GAME_LORD_SEAT(table), M_GAME_LORD_SCORE(table));
        }
    }
}

// Ӧ��ͻ��˾�ѡ����
static void OnAckVoteLord(int table, BYTE* pMsgBody, int nMsgBodyLen)
{
    //
    // �ͻ��������Ҫ���п��ƣ����ֻ�ܽб��ϼҸ��ߵķֻ򲻽з�
    //

    if (M_GAME_STARTED(table) == FALSE) {
        return;
    }

    if (nMsgBodyLen != 2) {
        WriteLog(LOG_ERROR, _T("NMSG_ACK_VOTE_LORD ��Ϣ�ṹ����ȷ"));
        return;
    }

    int seat = pMsgBody[0];
    int score = pMsgBody[1];

    // ��¼��ǰ����Ѿ��й���
    M_GAME_VOTED(table, seat) = TRUE;

    // ��λ�������ȴ���Ӧ��������Ӻ�
    M_GAME_ACTIVE_SEAT(table) = INVALID_SEAT;

    // �������ҽз֣���������ݣ��ݶ������Ϊ����
    if (M_GAME_LORD_SCORE(table) < score) {
        M_GAME_LORD_SCORE(table) = score;
        M_GAME_LORD_SEAT(table) = seat;
    }

    // ֪ͨ������ң�����ҽм���
    BYTE data[8];
    data[0] = NMSG_VOTE_LORD;
    data[1] = 0;
    data[2] = 2;
    data[3] = (BYTE)seat;
    data[4] = (BYTE)score;

    WORD crc = CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    BroadcastInTable(table, (char*)data, 7);

    if (score == 3) { // ĳ��ҽ�3�֣�������Ѿ�ѡ��
        VoteLordSucceed(table, seat, score);
        return;
    }

    int next_seat = (seat + 1) % GAME_SEAT_NUM_PER_TABLE;

    if (M_GAME_VOTED(table, next_seat) == TRUE) { // ����ҵ��¼��Ѿ��й�������˵��3����Ҷ��й�����
        if (M_GAME_LORD_SCORE(table) == 0) { // ������Ҷ���0��
            VoteLordFailed(table);
        } else {
            VoteLordSucceed(table, M_GAME_LORD_SEAT(table), M_GAME_LORD_SCORE(table));
        }
    } else { // �¼һ�û�й��֣�������һ����ҽз�
        PostMessage(NULL, TM_REQ_VOTE_LORD, (WPARAM)next_seat, (LPARAM)M_GAME_LORD_SCORE(table));
    }
}

// ����û�в���
static void VoteLordFailed(int table)
{
    BYTE data[8];

    data[0] = NMSG_VOTE_LORD_FINISH;

    data[1] = 0;
    data[2] = 1;

    data[3] = (BYTE)FALSE;

    WORD crc = CRC16(data, 4);
    data[4] = (BYTE)(crc >> 8);
    data[5] = (BYTE)(crc >> 0);

    BroadcastInTable(table, (char*)data, 6);

    WriteLog(LOG_DEBUG, _T("�� %d ����ѡ����������û����ҽе���"), table);

    // ����ϴ�ƣ����ƣ��з�
    {//Debug...
        Sleep(1000);
    }

    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        M_GAME_VOTED(table, i) = FALSE;
    }

    // ����û�˽е����������ѡ���Ƚе��������
    M_GAME_WINNER_SEAT(table) = INVALID_SEAT;

    // ����Ƿ�����ҵ��ߣ����û�У�����෢�����ƣ������û���˽е�����
    // �������Ϸ����������ΪSIT״̬
    BOOL bHasLost = FALSE;
    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        if (M_GAME_CONNECTION_LOST(table, i) == TRUE) {
            bHasLost = TRUE;
            break;
        }
    }

    if (bHasLost == TRUE) {
        WriteLog(LOG_DEBUG, _T("û����ҽе�����������ҵ��ߡ�������Ϸ��"));
        EndGame(table);
    } else {
        M_GAME_VOTE_LORD_FAIL_TIMES(table) += 1;
        if (M_GAME_VOTE_LORD_FAIL_TIMES(table) > SERVER_REDISTR_POKER_TIMES) {
            WriteLog(LOG_DEBUG, _T("�Ѿ����� %d ��û����ҽе������������������·��ƣ���ҽ������״̬"), 
                M_GAME_VOTE_LORD_FAIL_TIMES(table));
            EndGame(table);
        } else {
            WriteLog(LOG_DEBUG, _T("�� %d ��û����ҽе����������������·���"), 
                M_GAME_VOTE_LORD_FAIL_TIMES(table));
            PostMessage(NULL, TM_START_GAME, 0, 0);
        }
    }
}

// ��������
static void VoteLordSucceed(int table, int seat, int score)
{
    BYTE data[8];

    data[0] = NMSG_VOTE_LORD_FINISH;

    data[1] = 0;
    data[2] = 3;

    data[3] = (BYTE)TRUE;
    data[4] = (BYTE)seat;
    data[5] = (BYTE)score;

    WORD crc = CRC16(data, 6);
    data[6] = (BYTE)(crc >> 8);
    data[7] = (BYTE)(crc >> 0);

    // ֪ͨ��Ϸ�����г�Ա�������Ѿ�����
    BroadcastInTable(table, (char*)data, 8);

    WriteLog(LOG_DEBUG, _T("�� %d ����ѡ������������ %d ������ҽе���: %d ��"), table, seat, score);

    // �����Ʋ������������
    for (int i = 0; i < UNDER_CARDS_NUM; i++) {
        M_GAME_PLAYER_POKERS(table, seat)[PLAYER_INIT_CARDS_NUM + i] = M_GAME_UNDER_POKER_CARDS(table)[i];
    }

    quick_sort(M_GAME_PLAYER_POKERS(table, seat), 0, PLAYER_MAX_CARDS_NUM - 1);

    M_GAME_PLAYER_POKER_NUM(table, seat) = PLAYER_MAX_CARDS_NUM;

    { //Debug...
        int len;
        TCHAR buf[128];

        len = 0;
        for (int i = 0; i < M_GAME_PLAYER_POKER_NUM(table, 0); i++) {
            len += _stprintf_s(buf + len, sizeof(buf) / sizeof(buf[0]) - len, _T("%c "),
                poker_index_to_char(M_GAME_PLAYER_POKERS(table, 0)[i]));
        }
        WriteLog(LOG_DEBUG, buf);

        len = 0;
        for (int i = 0; i < M_GAME_PLAYER_POKER_NUM(table, 1); i++) {
            len += _stprintf_s(buf + len, sizeof(buf) / sizeof(buf[0]) - len, _T("%c "),
                poker_index_to_char(M_GAME_PLAYER_POKERS(table, 1)[i]));
        }
        WriteLog(LOG_DEBUG, buf);

        len = 0;
        for (int i = 0; i < M_GAME_PLAYER_POKER_NUM(table, 2); i++) {
            len += _stprintf_s(buf + len, sizeof(buf) / sizeof(buf[0]) - len, _T("%c "),
                poker_index_to_char(M_GAME_PLAYER_POKERS(table, 2)[i]));
        }
        WriteLog(LOG_DEBUG, buf);
    }

    // ��ʼ���ƣ�����������״γ���
    M_GAME_SAVE_POKER_PROPERTY(table, 0, 0, 0);

    WPARAM wp = MAKEWPARAM(M_GAME_LORD_SEAT(table), 0);
    LPARAM lp = MAKELPARAM(0, 0);
    PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);

    M_GAME_OUTPUT_CARD_STARTED(table) = TRUE;
}

// �㲥��Ϣ��������Ϸ����Ա
static void BroadcastInTable(int table, char* buf, int len)
{
    for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
        SendData(M_GAME_PLAYER_ID(table, seat), buf, len);

        for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
            if (M_GAME_LOOKON_ID(table, seat)[i] != INVALID_USER_ID) {
                SendData(M_GAME_LOOKON_ID(table, seat)[i], buf, len);
            }
        }
    }
}

// ��ĳ������⣬�㲥��Ϣ������Ϸ������������Ա
static void BroadcastInTableExceptId(int table, char* buf, int len, int id)
{
    for (int seat = 0; seat < GAME_SEAT_NUM_PER_TABLE; seat++) {
        if (id != M_GAME_PLAYER_ID(table, seat)) {
            SendData(M_GAME_PLAYER_ID(table, seat), buf, len);
        }

        for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
            if (M_GAME_LOOKON_ID(table, seat)[i] != INVALID_USER_ID) {
                if (id != M_GAME_LOOKON_ID(table, seat)[i]) {
                    SendData(M_GAME_LOOKON_ID(table, seat)[i], buf, len);
                }
            }
        }
    }
}

// ������ҳ���
static void ReqOutputCard(int table, int seat, POKER_PROPERTY* req)
{
    // ��¼������������Ƶ����Ӻ�
    M_GAME_ACTIVE_SEAT(table) = seat;

    if (M_GAME_DELEGATED(table, seat) == TRUE) {
        {//Debug...
            Sleep(100);
        }

        OutputCardDelegated(table, seat, req);
    } else {
        BYTE data[10];
        data[0] = NMSG_REQ_OUTPUT_CARD;
        data[1] = 0;
        data[2] = 4;

        data[3] = (BYTE)seat;
        data[4] = (BYTE)req->type;
        data[5] = (BYTE)req->value;
        data[6] = (BYTE)req->num;

        WORD crc = CRC16(data, 7);
        data[7] = (BYTE)(crc >> 8);
        data[8] = (BYTE)(crc >> 0);

        BroadcastInTable(table, (char*)data, 9);
    }
}

// �������й���Ϸ����
static void OutputCardDelegated(int table, int seat, POKER_PROPERTY* req)
{
    BYTE data[24];
    BOOL ret = FALSE;
    POKER_PROPERTY pp;
    ZeroMemory(&pp, sizeof(POKER_PROPERTY));

    int next_seat = (seat + 1) % GAME_SEAT_NUM_PER_TABLE;

    if ((req->value == 0) && (req->num == 0)) { // �״γ���
        data[0] = NMSG_OUTPUT_CARD;
        data[1] = 0;
        data[2] = 3;

        data[3] = (BYTE)seat;
        data[4] = (BYTE)1; // ��һ����С����

        int poker_elem = M_GAME_PLAYER_POKERS(table, seat)[0];
        data[5] = (BYTE)(poker_elem); // ��һ����С����

        WORD crc = CRC16(data, 6);
        data[6] = (BYTE)(crc >> 8);
        data[7] = (BYTE)(crc >> 0);

        BroadcastInTable(table, (char*)data, 8);

        // �����������Ƶ����ԣ�����¼�Ҫ���ƣ�������ݴ����ͳ���
        pp.num = 1;
        pp.type = SINGLE;
        pp.value = poker_index_to_value(poker_elem);

        WriteLog(LOG_DEBUG, _T("[�й�] �� %d �� %d ������ҳ��� %d �� [%s]: %c "), table, seat,
            pp.num, poker_type_to_string(pp.type), poker_index_to_char(poker_elem));

        int indexes[1] = { 0 }; // ����һ���ƣ�����ǰ��С�������ƣ�������Ϊ0
        RemoveOutputPokersByIndex(table, seat, indexes, 1);

        if (seat == M_GAME_LORD_SEAT(table)) { // �Ƿ�Ϊ��������
            M_GAME_LORD_OUTPUT_TIMES(table) += 1;
        }

        if (M_GAME_PLAYER_POKER_NUM(table, seat) <= 0) {
            GameOver(table, seat);
        } else {
            M_GAME_UP_NOT_FOLLOW(table) = FALSE; // ����ҳ��ƣ����ñ�־λΪFALSE
            M_GAME_SAVE_POKER_PROPERTY(table, pp.type, pp.value, pp.num);

            WPARAM wp = MAKEWPARAM(next_seat, pp.type);
            LPARAM lp = MAKELPARAM(pp.value, pp.num);
            PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);
        }
    } else { // ���ϼ���������
        int indexes[PLAYER_MAX_CARDS_NUM];
        POKER_CLASS_TABLE* pct;

        pct = (POKER_CLASS_TABLE*)LocalAlloc(LMEM_FIXED, sizeof(POKER_CLASS_TABLE));
        if (pct == NULL) {
            WriteLog(LOG_ERROR, _T("[�й�] �����ڴ�ʧ�ܣ�OutputCardDelegated: [POKER_CLASS_TABLE]"));
            ret = FALSE;
            goto FOLLOW_OR_NOT;
        }

        reset_poker_class_table(pct);

        // ������ʾ������һ�����Խӵ����������
        build_poker_class_table(pct, M_GAME_PLAYER_POKERS(table, seat), M_GAME_PLAYER_POKER_NUM(table, seat));
        ret = get_poker_hint(0, pct, req, &pp, indexes);

        LocalFree(pct);

FOLLOW_OR_NOT:
        if (ret == TRUE) { // ���Խ�����
            { //Debug...
                int len = 0;
                TCHAR outpokers[PLAYER_MAX_CARDS_NUM * 2 + 1] = { 0 }; // 1���ֽڼ�1���ո�
                for (int i = 0; i < pp.num; i++) {
                    len += _stprintf_s(outpokers + len, sizeof(outpokers) / sizeof(outpokers[0]) - len, _T("%c "), 
                        poker_index_to_char(M_GAME_PLAYER_POKERS(table, seat)[indexes[i]]));
                }

                WriteLog(LOG_DEBUG, _T("[�й�] �� %d �� %d ������ҳ��� %d �� [%s]: %s"), table, seat,
                    pp.num, poker_type_to_string(pp.type), outpokers);
            }

            data[0] = NMSG_OUTPUT_CARD;
            data[1] = (BYTE)((1 + 1 + pp.num) >> 8);
            data[2] = (BYTE)((1 + 1 + pp.num) >> 0);

            data[3] = (BYTE)seat;
            data[4] = (BYTE)pp.num;

            int offset = 5;
            for (int i = 0; i < pp.num; i++) {
                data[offset] = (BYTE)(M_GAME_PLAYER_POKERS(table, seat)[indexes[i]]);
                offset++;
            }

            WORD crc = CRC16(data, offset);
            data[offset] = (BYTE)(crc >> 8);
            data[offset + 1] = (BYTE)(crc >> 0);
            offset += 2;

            // �㲥��Ϸ����֪ͨ�������
            BroadcastInTable(table, (char*)data, offset);

            // ���·������ϵ�����
            RemoveOutputPokersByIndex(table, seat, indexes, pp.num);

            if (pp.type == BOMB) { // �������Ƿ�Ϊը��
                M_GAME_MULTIPLE(table)++;
            }

            if (seat == M_GAME_LORD_SEAT(table)) { // ���Ƿ�Ϊ��������
                M_GAME_LORD_OUTPUT_TIMES(table) += 1;
            }

            if (M_GAME_PLAYER_POKER_NUM(table, seat) <= 0) {
                GameOver(table, seat);
            } else {
                M_GAME_UP_NOT_FOLLOW(table) = FALSE; // ����ҳ��ƣ����ñ�־λΪFALSE
                M_GAME_SAVE_POKER_PROPERTY(table, pp.type, pp.value, pp.num);

                WPARAM wp = MAKEWPARAM(next_seat, pp.type);
                LPARAM lp = MAKELPARAM(pp.value, pp.num);
                PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);
            }
        } else { // ���ܽ�����
            WriteLog(LOG_DEBUG, _T("[�й�] �� %d �� %d ������Ҳ�����"), table, seat);

            data[0] = NMSG_OUTPUT_CARD;
            data[1] = 0;
            data[2] = 2;

            data[3] = (BYTE)seat;
            data[4] = (BYTE)0;      // ��ʾ������

            WORD crc = CRC16(data, 5);
            data[5] = (BYTE)(crc >> 8);
            data[6] = (BYTE)(crc >> 0);

            BroadcastInTable(table, (char*)data, 7);

            if (M_GAME_UP_NOT_FOLLOW(table) == TRUE) {
                // �ϼҽӲ��𣬱����Ҳ�Ӳ����ƣ����¼ҿ������³���
                M_GAME_SAVE_POKER_PROPERTY(table, 0, 0, 0);

                WPARAM wp = MAKEWPARAM(next_seat, 0);
                LPARAM lp = MAKELPARAM(0, 0);
                PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);
            } else {
                // ��ǰ��ҽӲ����ƣ�Ҫ���¼ҽӴ���
                M_GAME_UP_NOT_FOLLOW(table) = TRUE;
                M_GAME_SAVE_POKER_PROPERTY(table, req->type, req->value, req->num);

                WPARAM wp = MAKEWPARAM(next_seat, req->type);
                LPARAM lp = MAKELPARAM(req->value, req->num);
                PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);
            }
        }
    }
}

// ɾ��������ָʾ���˿��ƣ�����������ȫ���˿��Ƶ�����������������е�ǰ�Ƶ�����
static void RemoveOutputPokersByIndex(int table, int seat, int indexes[], int num)
{
    // ����һ�����飬��ʣ����ƿ�����ȥ���ٿ�������

    int rem_poker[PLAYER_MAX_CARDS_NUM] = { 0 };
    BOOL output[PLAYER_MAX_CARDS_NUM] = { 0 };

    for (int i = 0; i < num; i++) {
        output[indexes[i]] = TRUE;
    }

    int rem_count = 0;
    for (int i = 0; i < M_GAME_PLAYER_POKER_NUM(table, seat); i++) {
        if (output[i] != TRUE) {
            rem_poker[rem_count] = M_GAME_PLAYER_POKERS(table, seat)[i];
            rem_count++;
        }
    }

    M_GAME_PLAYER_POKER_NUM(table, seat) = rem_count;

    for (int i = 0; i < rem_count; i++) {
        M_GAME_PLAYER_POKERS(table, seat)[i] = rem_poker[i];
    }
}

// ������ҷ��͵��������ĳ�����Ϣ���ӷ�������������ɾ�������������
// ע�⣺pokers��BYTE�͵����飬��Ԫ��ֵ��ȫ���˿�������
static void RemoveOutputPokers(int table, int seat, BYTE pokers[], int num)
{
    int old_num = M_GAME_PLAYER_POKER_NUM(table, seat);

    BOOL output[PLAYER_MAX_CARDS_NUM] = { 0 };

    // ������������ԭ���������ñ�־���˿��ƾ�Ϊ����
    int j = 0;
    for (int i = 0; i < old_num; i++) {
        if (M_GAME_PLAYER_POKERS(table, seat)[i] == (int)(pokers[j])) {
            output[i] = TRUE;
            if (++j >= num) {
                break;
            }
        }
    }

    // ��ʣ����ƿ�����ȥ���ٿ�������������ʵ��ɾ����������
    int rem_count = 0;
    int rem_pokers[PLAYER_MAX_CARDS_NUM];

    for (int i = 0; i < old_num; i++) {
        if (output[i] != TRUE) {
            rem_pokers[rem_count++] = M_GAME_PLAYER_POKERS(table, seat)[i];
        }
    }

    for (int i = 0; i < rem_count; i++) {
        M_GAME_PLAYER_POKERS(table, seat)[i] = rem_pokers[i];
    }

    M_GAME_PLAYER_POKER_NUM(table, seat) = rem_count;
}

// ������Ϸ�󣬼������÷�
static void GameOver(int table, int nWinnerSeat)
{
    BYTE data[24];

    // ��¼�˴���ϷӮ�ҵ����Ӻ�
    M_GAME_WINNER_SEAT(table) = nWinnerSeat;

    int nLordScore = M_GAME_LORD_SCORE(table);  //�����еĵ׷�
    int nLordSeat = M_GAME_LORD_SEAT(table);    //��������λ��
    int nMultiple = M_GAME_MULTIPLE(table);     //����

    int nFinalLordScore = 0;        // �������÷�
    int nFinalFarmerScore = 0;      // ũ�����÷�
    int nFinalScore[GAME_SEAT_NUM_PER_TABLE] = { 0 }; // ����������÷֣�����λ�ţ�

    if (nLordSeat == nWinnerSeat) { // ����Ӯ
        if (M_GAME_PLAYER_POKER_NUM(table, (nLordSeat + 1) % GAME_SEAT_NUM_PER_TABLE) == PLAYER_INIT_CARDS_NUM) {
            if (M_GAME_PLAYER_POKER_NUM(table, (nLordSeat + 2) % GAME_SEAT_NUM_PER_TABLE) == PLAYER_INIT_CARDS_NUM) {
                // ����ũ��û���ƣ����1��
                nMultiple++;
            }
        }

        nFinalLordScore = 2 * nLordScore; // ˫���׷�
        for (int i = 0; i < nMultiple; i++) {
            nFinalLordScore *= 2;
        }
        nFinalFarmerScore = -1 * nFinalLordScore / 2;
    } else { // ũ��Ӯ
        if (M_GAME_LORD_OUTPUT_TIMES(table) <= 1) {
            // ����ֻ����һ���ƣ����1��
            nMultiple++;
        }

        nFinalFarmerScore = nLordScore; // �׷�
        for (int i = 0; i < nMultiple; i++) {
            nFinalFarmerScore *= 2;
        }
        nFinalLordScore = -1 * nFinalFarmerScore * 2;
    }

    // ��¼�÷�
    nFinalScore[nLordSeat] = nFinalLordScore;
    nFinalScore[(nLordSeat + 1) % GAME_SEAT_NUM_PER_TABLE] = nFinalFarmerScore;
    nFinalScore[(nLordSeat + 2) % GAME_SEAT_NUM_PER_TABLE] = nFinalFarmerScore;

    // ������������һ��ֵ�ͳ��
    M_CONNECT_PLAYER_TOTALGAMES(M_GAME_PLAYER_ID(table, 0)) += 1;
    M_CONNECT_PLAYER_TOTALGAMES(M_GAME_PLAYER_ID(table, 1)) += 1;
    M_CONNECT_PLAYER_TOTALGAMES(M_GAME_PLAYER_ID(table, 2)) += 1;

    if (nWinnerSeat == nLordSeat) {
        M_CONNECT_PLAYER_WINGAMES(M_GAME_PLAYER_ID(table, nLordSeat)) += 1;
    } else {
        M_CONNECT_PLAYER_WINGAMES(M_GAME_PLAYER_ID(table, (nLordSeat + 1) % 3)) += 1;
        M_CONNECT_PLAYER_WINGAMES(M_GAME_PLAYER_ID(table, (nLordSeat + 2) % 3)) += 1;
    }

    M_CONNECT_PLAYER_SCORE(M_GAME_PLAYER_ID(table, nLordSeat)) += nFinalLordScore;
    M_CONNECT_PLAYER_SCORE(M_GAME_PLAYER_ID(table, (nLordSeat + 1) % 3)) += nFinalFarmerScore;
    M_CONNECT_PLAYER_SCORE(M_GAME_PLAYER_ID(table, (nLordSeat + 2) % 3)) += nFinalFarmerScore;

    // ������Ϸ������Ϣ
    int offset = 0;

    data[offset] = NMSG_GAME_OVER;
    offset++;

    data[offset + 0] = 0;
    data[offset + 1] = 12;
    offset += 2;

    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        data[offset + 0] = (BYTE)(nFinalScore[i] >> 24);
        data[offset + 1] = (BYTE)(nFinalScore[i] >> 16);
        data[offset + 2] = (BYTE)(nFinalScore[i] >> 8);
        data[offset + 3] = (BYTE)(nFinalScore[i] >> 0);
        offset += 4;
    }

    WORD crc = CRC16(data, offset);
    data[offset + 0] = (BYTE)(crc >> 8);
    data[offset + 1] = (BYTE)(crc >> 0);
    offset += 2;

    // ֪ͨ��������й���Ϸ������ĵ÷����
    BroadcastInTable(table, (char*)data, offset);

    WriteLog(LOG_DEBUG, _T("��Ϸ�������� %d �� %d ���ӵ����Ӯ�ñ�����Ϸ����ҵ÷֣�[%d, %d, %d]"),
        table, nWinnerSeat, nFinalScore[0], nFinalScore[1], nFinalScore[2]);

    // ��λ�����Ϸ������
    EndGame(table);
}

// ��Ӧ��ҳ���
static void OnAckOutputCard(int table, BYTE* pMsgBody, int nMsgBodyLen)
{
    if (M_GAME_STARTED(table) == FALSE) {
        return;
    }

    // ��λ�������ȴ���Ӧ��������Ӻ�
    M_GAME_ACTIVE_SEAT(table) = INVALID_SEAT;

    BYTE data[PLAYER_MAX_CARDS_NUM + 8]; // ���ڹ�����Ƶ���Ϣ
    int seat = pMsgBody[0]; // ������ҵ����Ӻ�
    POKER_TYPE type = (POKER_TYPE)pMsgBody[1]; // ���Ƶ�����
    int value = pMsgBody[2]; // ���Ƶ�ֵ
    int num = pMsgBody[3]; // ���Ƶ�����
    BYTE* pokers = pMsgBody + 4; // �˿������п�ʼ��ַ

    if ((4 + num) != nMsgBodyLen) {
        WriteLog(LOG_ERROR, _T("NMSG_ACK_OUTPUT_CARD ��Ϣ�ṹ����ȷ"));
        return;
    }

    int next_seat = (seat + 1) % GAME_SEAT_NUM_PER_TABLE;

    if ((value == 0) && (num == 0)) { // ��Ҳ�����
        WriteLog(LOG_DEBUG, _T("�� %d �� %d ������Ҳ�����"), table, seat);

        data[0] = NMSG_OUTPUT_CARD;
        data[1] = 0;
        data[2] = 2;

        data[3] = (BYTE)seat;
        data[4] = (BYTE)0;      // ��ʾ������

        WORD crc = CRC16(data, 5);
        data[5] = (BYTE)(crc >> 8);
        data[6] = (BYTE)(crc >> 0);

        // �㲥֪ͨ�ͻ�����ң������ӵ���Ҳ�����
        BroadcastInTable(table, (char*)data, 7);

        if (M_GAME_UP_NOT_FOLLOW(table) == TRUE) {
            // �ϼҽӲ����ƣ������Ҳ�Ӳ����ƣ����¼ҿ������³���
            M_GAME_SAVE_POKER_PROPERTY(table, 0, 0, 0);

            WPARAM wp = MAKEWPARAM(next_seat, 0);
            LPARAM lp = MAKELPARAM(0, 0);
            PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);
        } else {
            // �ϼҳ����ƣ���ǰ��Ҳ����ƣ�Ҫ���¼ҽ��ϼ���������
            M_GAME_UP_NOT_FOLLOW(table) = TRUE;

            WPARAM wp = MAKEWPARAM(next_seat, M_GAME_OUTPUT_POKER_TYPE(table));
            LPARAM lp = MAKELPARAM(M_GAME_OUTPUT_POKER_VALUE(table), M_GAME_OUTPUT_POKER_NUM(table));
            PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);
        }
    } else { // ��ҳ���
        {//Debug...
            int len = 0;
            TCHAR outpokers[PLAYER_MAX_CARDS_NUM * 2 + 1] = { 0 }; // 1���ֽڼ�1���ո�
            for (int i = 0; i < num; i++) {
                len += _stprintf_s(outpokers + len, sizeof(outpokers) / sizeof(outpokers[0]) - len,
                    _T("%c "), poker_index_to_char(pokers[i]));
            }

            WriteLog(LOG_DEBUG, _T("�� %d �� %d ������ҳ��� %d �� [%s]: %s"),
                table, seat, num, poker_type_to_string(type), outpokers);
        }

        data[0] = NMSG_OUTPUT_CARD;
        data[1] = (BYTE)((1 + 1 + num) >> 8);
        data[2] = (BYTE)((1 + 1 + num) >> 0);

        data[3] = (BYTE)seat;
        data[4] = (BYTE)num;

        int offset = 5;
        for (int i = 0; i < num; i++) {
            data[offset] = pokers[i];
            offset++;
        }

        WORD crc = CRC16(data, offset);
        data[offset] = (BYTE)(crc >> 8);
        data[offset + 1] = (BYTE)(crc >> 0);
        offset += 2;

        // �㲥��Ϸ��
        BroadcastInTable(table, (char*)data, offset);

        // ���·������ϵ�����
        RemoveOutputPokers(table, seat, pokers, num);

        if (type == BOMB) { // �������Ƿ�Ϊը��
            M_GAME_MULTIPLE(table)++;
        }

        if (seat == M_GAME_LORD_SEAT(table)) { // ���Ƿ�Ϊ��������
            M_GAME_LORD_OUTPUT_TIMES(table) += 1;
        }

        if (M_GAME_PLAYER_POKER_NUM(table, seat) <= 0) {
            GameOver(table, seat);
        } else {
            M_GAME_UP_NOT_FOLLOW(table) = FALSE; // ����ҳ��ƣ����ñ�־λΪFALSE
            M_GAME_SAVE_POKER_PROPERTY(table, type, value, num);

            WPARAM wp = MAKEWPARAM(next_seat, type);
            LPARAM lp = MAKELPARAM(value, num);
            PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);
        }
    }
}

// ��Ӧ�����;�˳���Ϸ������
static void OnReqStopGaming(int table, BYTE* pMsgBody, int nMsgBodyLen)
{
    int seat = pMsgBody[0];

    BYTE* data = (BYTE*)LocalAlloc(LMEM_FIXED, nMsgBodyLen + 5);
    if (data == NULL) {
        WriteLog(LOG_ERROR, _T("�����ڴ�ʧ�ܣ�Ӧ�������;�˳���Ϸ"));
        return;
    }

    data[0] = NMSG_REQ_STOP_GAMING;

    data[1] = (BYTE)(nMsgBodyLen >> 8);
    data[2] = (BYTE)(nMsgBodyLen >> 0);

    for (int i = 0; i < nMsgBodyLen; i++) {
        data[3 + i] = pMsgBody[i];
    }

    WORD crc = CRC16(data, 3 + nMsgBodyLen);
    data[3 + nMsgBodyLen + 0] = (BYTE)(crc >> 8);
    data[3 + nMsgBodyLen + 1] = (BYTE)(crc >> 0);

    int next_seat = (seat + 1) % GAME_SEAT_NUM_PER_TABLE; // �¼�

    // ת����Ϣ����һ����ң������������
    SendData(M_GAME_PLAYER_ID(table, next_seat), (char*)data, 3 + nMsgBodyLen + 2);

    LocalFree(data);
}

// ��Ӧ����Ƿ�ͬ��ĳ�����;�˳���Ϸ
static void OnAckStopGaming(int table, BYTE* pMsgBody, int nMsgBodyLen)
{
    int ack_exit_seat   = pMsgBody[0];
    BOOL bPermit        = (BOOL)pMsgBody[1];
    int req_exit_seat   = pMsgBody[2];
    int nReasonTextLen  = pMsgBody[3];
    BYTE* szStopReason  = pMsgBody + 4;

    if (M_GAME_STARTED(table) == FALSE) {
        return;
    }

    if ((4 + nReasonTextLen) != nMsgBodyLen) {
        WriteLog(LOG_ERROR, _T("NMSG_ACK_STOP_GAMING ��Ϣ�ṹ����ȷ"));
        return;
    }

    WORD crc;
    BYTE* data = (BYTE*)LocalAlloc(LMEM_FIXED, 5 + nMsgBodyLen);
    if (data == NULL) {
        WriteLog(LOG_ERROR, _T("�����ڴ�ʧ�ܣ���Ӧĳ����Ƿ�ͬ����;�˳�"));
        return;
    }

    if (bPermit == FALSE) { // ĳ��Ҳ�ͬ�⣬��ظ������˳������Ϊ��ͬ���˳�
        data[0] = NMSG_ACK_STOP_GAMING;
        data[1] = 0;
        data[2] = 2;

        data[3] = (BYTE)FALSE;
        data[4] = (BYTE)ack_exit_seat;

        crc = CRC16(data, 5);
        data[5] = (BYTE)(crc >> 8);
        data[6] = (BYTE)(crc >> 0);

        SendData(M_GAME_PLAYER_ID(table, req_exit_seat), (char*)data, 7);
    } else { // ĳ���ͬ��
        //
        // �����˳����������Ӧ���ߵ��¼ң���˵��������Ҷ�������ͬ���˳��������
        // �����������Ӧ���ߵ��¼ҵ����
        //
        if (req_exit_seat == ((ack_exit_seat + 1) % GAME_SEAT_NUM_PER_TABLE)) { // ������Ҷ�ͬ���˳�
            data[0] = NMSG_ACK_STOP_GAMING;
            data[1] = 0;
            data[2] = 1;

            data[3] = (BYTE)TRUE;

            crc = CRC16(data, 4);
            data[4] = (BYTE)(crc >> 8);
            data[5] = (BYTE)(crc >> 0);

            BroadcastInTable(table, (char*)data, 6);

            EndGame(table);

            // ���·��������ݣ������״̬����
            int id = M_GAME_PLAYER_ID(table, req_exit_seat);

            M_GAME_PLAYER_ID(table, req_exit_seat) = INVALID_USER_ID;

            M_TABLE_PLAYER_ID(table, req_exit_seat) = INVALID_USER_ID;

            M_CONNECT_STATE(id) = STATE_IDLE;

            AddUserStateChangeEvent(id, EVT_CHGSTATE, STATE_IDLE, FALSE);
        } else { // ��һ�����ͬ�⣬������ڶ�����ҵ����
            int next_seat = (ack_exit_seat + 1) % GAME_SEAT_NUM_PER_TABLE;

            data[0] = NMSG_REQ_STOP_GAMING;
            data[1] = (BYTE)((nReasonTextLen + 2) >> 8);
            data[2] = (BYTE)((nReasonTextLen + 2) >> 0);

            data[3] = (BYTE)req_exit_seat;
            data[4] = (BYTE)nReasonTextLen;
            for (int i = 0; i < nReasonTextLen; i++) {
                data[5 + i] = szStopReason[i];
            }

            crc = CRC16(data, 5 + nReasonTextLen);
            data[5 + nReasonTextLen + 0] = (BYTE)(crc >> 8);
            data[5 + nReasonTextLen + 1] = (BYTE)(crc >> 0);

            SendData(M_GAME_PLAYER_ID(table, next_seat), (char*)data, 5 + nReasonTextLen + 2);
        }
    }

    LocalFree(data);
}

// ��Ӧ����й���Ϸ������
static void OnReqDelegate(int table, BYTE* pMsgBody, int nMsgBodyLen)
{
    if (M_GAME_STARTED(table) == FALSE) {
        return;
    }

    if (nMsgBodyLen != 2) {
        WriteLog(LOG_ERROR, _T("NMSG_REQ_DELEGATE ��Ϣ�ṹ����ȷ"));
        return;
    }

    int seat = pMsgBody[0];
    BOOL bDelegate = (BOOL)pMsgBody[1];

    if (M_GAME_DELEGATED(table, seat) == bDelegate) {
        return;
    }

    M_GAME_DELEGATED(table, seat) = bDelegate;

    BYTE data[8];
    data[0] = NMSG_ACK_DELEGATE;
    data[1] = 0;
    data[2] = 2;

    data[3] = pMsgBody[0];
    data[4] = pMsgBody[1];

    WORD crc = CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    BroadcastInTable(table, (char*)data, 7);

    if (M_GAME_DELEGATED(table, seat) == TRUE) {
        if (seat == M_GAME_ACTIVE_SEAT(table)) { // �����й���Ϸ������Ƿ��������ȴ��ظ���Ϣ�����
            if (M_GAME_OUTPUT_CARD_STARTED(table) == FALSE) { // ��Ϸ�Ѿ���ʼ������û��ʼ���ƣ����з���
                PostMessage(NULL, TM_REQ_VOTE_LORD, (WPARAM)seat, (LPARAM)(M_GAME_LORD_SCORE(table)));
            } else { // �Ѿ���ʼ����
                WPARAM wp = MAKEWPARAM(seat, M_GAME_OUTPUT_POKER_TYPE(table));
                LPARAM lp = MAKELPARAM(M_GAME_OUTPUT_POKER_VALUE(table), M_GAME_OUTPUT_POKER_NUM(table));
                PostMessage(NULL, TM_REQ_OUTPUT_CARD, wp, lp);
            }
        }
    }

    if (bDelegate == TRUE) {
        WriteLog(LOG_INFO, _T("�� %d �� %d ���ӵ���ң�ѡ���й���Ϸ"), table, seat);
    } else {
        WriteLog(LOG_INFO, _T("�� %d �� %d ���ӵ���ң�ȡ���й���Ϸ"), table, seat);
    }
}

// ��Ӧ��������Ƿ������Թ۵�ѡ��
// �ͻ�����Ҫ���ƣ�ֻ������ҷ��ʹ���Ϣ���Թ��߲��ܷ��ʹ���Ϣ
static void OnAllowLookon(int table, BYTE* pMsgBody, int nMsgBodyLen)
{
    if (nMsgBodyLen != 2) {
        WriteLog(LOG_ERROR, _T("NMSG_ALLOW_LOOKON ��Ϣ�ṹ����ȷ"));
        return;
    }

    int seat = pMsgBody[0];
    BOOL bAllow = (BOOL)pMsgBody[1];

    int id = M_GAME_PLAYER_ID(table, seat);
    M_CONNECT_ALLOW_LOOKON(id) = bAllow;

    // ֪ͨ�����ӵ��Թ���
    BYTE data[8];
    data[0] = NMSG_ALLOW_LOOKON;
    data[1] = 0;
    data[2] = 2;

    data[3] = (BYTE)seat;
    data[4] = (BYTE)bAllow;

    WORD crc = CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    for (int i = 0; i < MAX_LOOKON_NUM_PER_SEAT; i++) {
        if (M_GAME_LOOKON_ID(table, seat)[i] != INVALID_USER_ID) {
            SendData(M_GAME_LOOKON_ID(table, seat)[i], (char*)data, 7);
        }
    }

    // ʹ��״̬�����¼�ʹ�ÿͻ���֪������ҵ�����
    AddUserStateChangeEvent(M_GAME_PLAYER_ID(table, seat), EVT_CHGSTATE, M_CONNECT_STATE(id), FALSE);
}

// ��Ϸ�����У����ǿ���˳���Ϸ
static void OnReqForceStopGaming(int table, BYTE* pMsgBody, int nMsgBodyLen)
{
    if (nMsgBodyLen != 1) {
        WriteLog(LOG_ERROR, _T("NMSG_REQ_FORCE_STOP_GAMING ��Ϣ�ṹ����ȷ"));
        return;
    }

    BYTE data[24];
    WORD crc;
    int seat = pMsgBody[0];
    int id = M_GAME_PLAYER_ID(table, seat);

    //
    // ֪ͨ��Ϸ��������Ա��ĳ���ǿ���˳�
    //
    data[0] = NMSG_ACK_FORCE_STOP_GAMING;
    data[1] = 0;
    data[2] = 1;

    data[3] = (BYTE)seat;

    crc = CRC16(data, 4);
    data[4] = (BYTE)(crc >> 8);
    data[5] = (BYTE)(crc >> 0);

    BroadcastInTableExceptId(table, (char*)data, 6, id);

    //
    // �������ܺ�÷����
    //
    int results[GAME_SEAT_NUM_PER_TABLE];
    int multiple = M_GAME_MULTIPLE(table) + RUN_AWAY_MULTIPLE;
    int score = M_GAME_LORD_SCORE(table);

    for (int i = 0; i < multiple; i++) {
        score *= 2;
    }

    results[seat] = -1 * score;
    results[(seat + 1) % GAME_SEAT_NUM_PER_TABLE] = score / 2;
    results[(seat + 2) % GAME_SEAT_NUM_PER_TABLE] = score / 2;

    // ������Ϸ������Ϣ
    int offset = 0;

    data[offset] = NMSG_GAME_OVER;
    offset++;

    data[offset + 0] = 0;
    data[offset + 1] = 12;
    offset += 2;

    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        data[offset + 0] = (BYTE)(results[i] >> 24);
        data[offset + 1] = (BYTE)(results[i] >> 16);
        data[offset + 2] = (BYTE)(results[i] >> 8);
        data[offset + 3] = (BYTE)(results[i] >> 0);
        offset += 4;
    }

    crc = CRC16(data, offset);
    data[offset + 0] = (BYTE)(crc >> 8);
    data[offset + 1] = (BYTE)(crc >> 0);
    offset += 2;

    // ֪ͨ����������÷����
    BroadcastInTableExceptId(table, (char*)data, offset, id);

    // ��Ϸ����
    M_GAME_WINNER_SEAT(table) = INVALID_SEAT;
    EndGame(table);

    WriteLog(LOG_INFO, _T("�� %d �� %d ���ӵ����ǿ���˳���Ϸ"), table, seat);

    // ���·��������ݣ������״̬����
    M_GAME_PLAYER_ID(table, seat) = INVALID_USER_ID;

    M_TABLE_PLAYER_ID(table, seat) = INVALID_USER_ID;

    M_CONNECT_STATE(id) = STATE_IDLE;

    AddUserStateChangeEvent(id, EVT_CHGSTATE, STATE_IDLE, FALSE);
}

// ת��������Ϣ����Ϸ���������˶��ɼ���������˽�ģ�
static void OnChatMessage(int table, BYTE* pMsgBody, int nMsgBodyLen)
{
    BYTE data[140]; // ������Ϣ���127���ֽ�

    data[0] = NMSG_CHATTING;
    data[1] = 0;
    data[2] = (BYTE)nMsgBodyLen;

    for (int i = 0; i < nMsgBodyLen; i++) {
        data[3 + i] = pMsgBody[i];
    }

    WORD crc = CRC16(data, 3 + nMsgBodyLen);
    data[3 + nMsgBodyLen + 0] = (BYTE)(crc >> 8);
    data[3 + nMsgBodyLen + 1] = (BYTE)(crc >> 0);

    BroadcastInTable(table, (char*)data, 3 + nMsgBodyLen + 2);
}
