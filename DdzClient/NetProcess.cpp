//
// File: NetProc.cpp
//
//  ��������ͨ�š���������������ӣ����ա��������������ݡ�
//
#include "stdafx.h"
#include "DdzClient.h"
#include "NetProcess.h"

extern USER_LOGON_INFO g_UserLogonInfo;
extern SOCKET g_ConSocket;
extern HWND g_hMainWnd;
extern DWORD g_nMainThreadId;
extern BOOL g_bShutdownConnection;

// ��ʼ��Winsocket
BOOL InitWinSocket(void)
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

    return TRUE;
}

// ����WinSock
void CloseWinSock(void)
{
    WSACleanup();
}

// �ر���������
void CloseConnection(SOCKET sock)
{
    if (sock != INVALID_SOCKET) {
        shutdown(sock, SD_BOTH);
        closesocket(sock);
    }
}

// ���ӷ�����������IP��PortΪ������ģ�
SOCKET EstablishConnection(DWORD ip, USHORT port)
{
    SOCKET sock = INVALID_SOCKET;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET; 
    }

    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = htonl(ip);
    clientService.sin_port = htons(port);

    if (connect(sock, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
        return INVALID_SOCKET; 
    }

    return sock;
}

// ���ڽ������ݵ��߳���ں���
DWORD ConnectionReceiveDataProc(LPVOID lpData)
{
    SOCKET sock = (SOCKET)lpData;

    //
    // ��������������Ͽ����ӣ�����������̹߳رո������߳̾������ˣ�������Ϣ
    // TM_CONNECTION_LOST ���뽻�����̴߳���������Ϣ�������̴߳���Ҳ���ɱ��߳�
    // �����ɺ� MAIN_THREAD_PROCESS_NMSG ���Ƿ���������
    //
    // ��������� MAIN_THREAD_PROCESS_NMSG�������߳��ڴ�����Ϣʱ�����ܵ����˵���
    // ģʽ�Ի�����Ϊ����������ֹ���߳̽��ղ������������ʱ���͵���Ϣ��������Ϣ��
    // �Ӷ����¿ͻ������������ά���������Ϣ��һ�¡�
    //
    int nExitReason = -1;
    int nRecv = 0;
    char* data = (char*)LocalAlloc(LMEM_FIXED, RECV_DATA_BUF_SZ * sizeof(char));
    if (data == NULL) {
        PostThreadMessage(g_nMainThreadId, TM_CONNECTION_LOST, nExitReason, 0);
        return (DWORD)-1;
    }

    for (;;) {
        if (g_bShutdownConnection == TRUE) {
            break;
        }

        nRecv = recv(sock, data, RECV_DATA_BUF_SZ, 0);

        if (nRecv > 0) {
            OnNetMessage((BYTE*)data, nRecv);
        } else if (nRecv == 0) {
            nExitReason = 1;
            break;
        } else {
            nExitReason = 2;
            break;
        }
    }

    LocalFree(data);

    if (g_bShutdownConnection != TRUE) {
        PostThreadMessage(g_nMainThreadId, TM_CONNECTION_LOST, nExitReason, 0);
    }

    return 0;
}

// ������յ�����Ϣ����
// �����Ϣ�ĺϷ��ԣ����ַ���������Ϣ������
void OnNetMessage(BYTE* buf, int len)
{
    BYTE* data = buf;

    int nParsedLen = 0;     // ���������ݵĳ���
    int nMsgCode = 0;       // ��Ϣ��
    int nMsgBodyLen = 0;    // ��Ϣ�峤��
    BYTE* pMsgBody = NULL;  // ��Ϣ��������ʼָ��
    int   msgcount = 0;

    WORD myCrc, yourCrc;

    while (nParsedLen < len) {
        if (nParsedLen + 2 >= len) {
            TRACE1(_T("[1] ��Ϣ�ṹ���Ϸ����ѽ��� %d ���Ϸ���Ϣ"), msgcount);
            return;
        }

        nMsgBodyLen = (data[nParsedLen + 1] << 8) | data[nParsedLen + 2];

        // 1���ֽ���Ϣ�룬2���ֽ���Ϣ�峤�ȣ�nMsgBodyLen���ֽ���Ϣ�壬2���ֽ�CRC
        if (nParsedLen + 1 + 2 + nMsgBodyLen + 2 > len) {
            TRACE1(_T("[2] ��Ϣ�ṹ���Ϸ����ѽ��� %d ���Ϸ���Ϣ"), msgcount);
            return;
        }

        myCrc = CommonUtil::CRC16((data + nParsedLen), 1 + 2 + nMsgBodyLen);
        yourCrc = (data[nParsedLen + 1 + 2 + nMsgBodyLen] << 8) | 
            data[nParsedLen + 1 + 2 + nMsgBodyLen + 1];
        if (myCrc != yourCrc) {
            TRACE1(_T("���յ�����CRC�ֶβ���ȷ���ѽ��� %d ���Ϸ���Ϣ"), msgcount);
            return;
        }

        nMsgCode = data[nParsedLen];
        pMsgBody = data + nParsedLen + 1 + 2;

        nParsedLen += (1 + 2 + nMsgBodyLen + 2);
        msgcount++;

        //
        // �ַ���Ϣʱ������Ϣ�弰�䳤�ȴ��ݸ���Ӧ������
        // �������Ѿ�����CRCУ�飬��Ϣ��������ֱ�Ӵ�����Ϣ
        //

#define ON_NMSG_BREAK(NMSG_NAME) \
case NMSG_NAME: { On_ ## NMSG_NAME (pMsgBody, nMsgBodyLen); break; }

        switch (nMsgCode) {
            ON_NMSG_BREAK(  NMSG_ACK_ROOM_INFO)
            ON_NMSG_BREAK(  NMSG_PLAYER_CHANGE_STATE);
            ON_NMSG_BREAK(  NMSG_ACK_GAMER_TAKE_SEAT);
            ON_NMSG_BREAK(  NMSG_ACK_LOOKON_TAKE_SEAT);
            ON_NMSG_BREAK(  NMSG_ACK_GAMER_LEAVE_SEAT);
            ON_NMSG_BREAK(  NMSG_ACK_LOOKON_LEAVE_SEAT);
            ON_NMSG_BREAK(  NMSG_ACK_GAMER_READY);
            ON_NMSG_BREAK(  NMSG_DISTRIBUTE_CARD);
            ON_NMSG_BREAK(  NMSG_REQ_VOTE_LORD);
            ON_NMSG_BREAK(  NMSG_VOTE_LORD);
            ON_NMSG_BREAK(  NMSG_VOTE_LORD_FINISH);
            ON_NMSG_BREAK(  NMSG_OUTPUT_CARD);
            ON_NMSG_BREAK(  NMSG_REQ_OUTPUT_CARD);
            ON_NMSG_BREAK(  NMSG_GAME_OVER);
            ON_NMSG_BREAK(  NMSG_CHATTING);
            ON_NMSG_BREAK(  NMSG_CONNECTION_LOST);
            ON_NMSG_BREAK(  NMSG_ACK_DELEGATE);
            ON_NMSG_BREAK(  NMSG_REQ_STOP_GAMING);
            ON_NMSG_BREAK(  NMSG_ACK_STOP_GAMING);
            ON_NMSG_BREAK(  NMSG_ACK_FORCE_STOP_GAMING);
            ON_NMSG_BREAK(  NMSG_ALLOW_LOOKON);
            ON_NMSG_BREAK(  NMSG_BROADCASTING);

            default:
                TRACE(_T("Unknown Message. What the hell!\n"));
                break;
        }
    }
}

void On_NMSG_ACK_ROOM_INFO(BYTE* pMsgBody, int nMsgBodyLen)
{
    // !!!
    // Analyze the message and save user info to LocalAlloc() memory. Then 
    // PostMessage to Main thread with the memory as a param. 
    // Requirement: Main thread will be responsible for freeing the memory.
    //
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int nClientID = 0;
    int nPlayerCount = 0;
    int nPlayerNum = 0;

    nClientID = (data[offset] << 8) | data[offset + 1];
    offset += 2;
    
    if ((nClientID < 0) || (nClientID >= MAX_CONNECTION_NUM)) {
        TRACE(_T("Impossible! The message is not valid or some error occured during transmission.\n"));
        return;
    }

    nPlayerCount = (data[offset] << 8) | data[offset + 1];
    offset += 2;

    if ((nPlayerCount < 0) || (nPlayerCount >= MAX_CONNECTION_NUM)) {
        TRACE(_T("Impossible! The message is not valid or some error occured during transmission.\n"));
        return;
    }

    if (nPlayerCount == 0) { // It's the first user connected to the server, no other users yet.
        WPARAM wp = MAKEWPARAM(nClientID, nPlayerCount);
        LPARAM lp = 0;
        PostMessageToMain(TM_RECV_SVR_ROOM_INFO, wp, lp);
        return;
    }

    // no zero-init, since that may be faster.
    PLAYER_INFO* pUserInfo = (PLAYER_INFO*)LocalAlloc(LMEM_FIXED,// | LMEM_ZEROINIT,
        sizeof(PLAYER_INFO) * nPlayerCount);
    if (pUserInfo == NULL) {
        TRACE(_T("Alloc memory failed while processing ACK_ROOM_INFO\n"));
        return;
    }

    BYTE*   pName = NULL;
    int     nPlayerNameLen = 0;

    while ((offset < len) && (nPlayerNum < nPlayerCount)) {
        // ID
        pUserInfo[nPlayerNum].id = (data[offset] << 8) | data[offset + 1];
        offset += 2;

        // Name length
        nPlayerNameLen = data[offset];
        offset += 1;

        assert(nPlayerNameLen / sizeof(TCHAR) < MAX_USER_NAME_LEN);

        // Name
        pName = (BYTE*)pUserInfo[nPlayerNum].name;
        for (int i = 0; i < nPlayerNameLen; i++) {
            pName[i] = data[offset + i];
        }
        offset += nPlayerNameLen;

        // add null terminated char, since the memory allocated without zero-initialized.
        pUserInfo[nPlayerNum].name[nPlayerNameLen / sizeof(TCHAR)] = _T('\0');

        // Gender, state, imageIdx
        pUserInfo[nPlayerNum].gender    = (PLAYER_GENDER)(data[offset] >> 7);
        pUserInfo[nPlayerNum].state     = (PLAYER_STATE)((data[offset] >> 4) & 0x07);
        pUserInfo[nPlayerNum].imageIdx  =  data[offset] & 0x0F;
        offset += 1;

        if (pUserInfo[nPlayerNum].state != STATE_IDLE) {
            // AllowLookon
            pUserInfo[nPlayerNum].bAllowLookon = data[offset] >> 7;

            // Seat
            pUserInfo[nPlayerNum].seat = data[offset] & 0x0F;
            offset += 1;

            // Table
            pUserInfo[nPlayerNum].table = data[offset];
            offset += 1;
        } else {
            // AllowLookon
            pUserInfo[nPlayerNum].bAllowLookon = data[offset] >> 7;
            offset += 1;

            // Default seat and table
            pUserInfo[nPlayerNum].seat = INVALID_SEAT;
            pUserInfo[nPlayerNum].table = INVALID_TABLE;
        }

        // ���֡���Ϸ������Ӯ�������ܴ���
        pUserInfo[nPlayerNum].nScore = ((data[offset + 0] << 24) | (data[offset + 1] << 16) |
            (data[offset + 2] << 8) | (data[offset + 3] << 0));
        offset += 4;

        pUserInfo[nPlayerNum].nTotalGames = ((data[offset + 0] << 24) | (data[offset + 1] << 16) |
            (data[offset + 2] << 8) | (data[offset + 3] << 0));
        offset += 4;

        pUserInfo[nPlayerNum].nWinGames = ((data[offset + 0] << 24) | (data[offset + 1] << 16) |
            (data[offset + 2] << 8) | (data[offset + 3] << 0));
        offset += 4;

        pUserInfo[nPlayerNum].nRunawayTimes = ((data[offset + 0] << 24) | (data[offset + 1] << 16) |
            (data[offset + 2] << 8) | (data[offset + 3] << 0));
        offset += 4;

        nPlayerNum++;
    }

    assert(offset == len);
    assert(nPlayerNum == nPlayerCount);

    WPARAM wp = MAKEWPARAM(nClientID, nPlayerCount);
    LPARAM lp = (LPARAM)pUserInfo;
    PostMessageToMain(TM_RECV_SVR_ROOM_INFO, wp, lp);
}

void On_NMSG_PLAYER_CHANGE_STATE(BYTE* pMsgBody, int nMsgBodyLen)
{
    // !!!
    // Requirement: Main thread will be responsible for freeing the memory while
    // processing TM_RECV_PLAYER_STATE_CHANGE.
    //
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int nCurPlayers = (data[offset] << 8) | data[offset + 1];
    offset += 2;

    int nServerCapacity = (data[offset] << 8) | data[offset + 1];
    offset += 2;

    PostMessageToMain(TM_RECV_SVR_STATUS, nCurPlayers, nServerCapacity);

    int count = (data[offset] << 8) | data[offset + 1];
    offset += 2;

    if (count <= 0) {
        return;
    }

    PLAYER_CHG_DATA* pChangeData = (PLAYER_CHG_DATA*)LocalAlloc(LMEM_FIXED,
        sizeof(PLAYER_CHG_DATA) * count);
    if (pChangeData == NULL) {
        TRACE(_T("�յ����״̬�仯��Ϣ�����������ڽ������ڴ�ʧ��\n"));
        return;
    }

    int num = 0;
    while (num < count) {
        int id = (data[offset] << 8) | data[offset + 1];
        offset += 2;

        STATE_CHANGE_EVENT evtChange = (STATE_CHANGE_EVENT)data[offset];
        offset += 1;

        BYTE*   pName = NULL;
        int     nPlayerNameLen = 0;

        switch (evtChange) {
            case EVT_ENTER:
                pChangeData[num].event = EVT_ENTER;
                pChangeData[num].playerInfo.id = id;

                // Name length
                nPlayerNameLen = data[offset];
                offset += 1;

                assert(nPlayerNameLen / sizeof(TCHAR) < MAX_USER_NAME_LEN);

                // Name
                pName = (BYTE*)pChangeData[num].playerInfo.name;
                for (int i = 0; i < nPlayerNameLen; i++) {
                    pName[i] = data[offset + i];
                }
                offset += nPlayerNameLen;

                // add null terminated char, since the memory allocated without zero-initialized.
                pChangeData[num].playerInfo.name[nPlayerNameLen / sizeof(TCHAR)] = _T('\0');

                pChangeData[num].playerInfo.gender  = (PLAYER_GENDER)(data[offset] >> 7);
                pChangeData[num].playerInfo.state   = (PLAYER_STATE)((data[offset] & 0x70) >> 4);
                pChangeData[num].playerInfo.imageIdx = data[offset] & 0x0F;
                offset += 1;

                // init other fields as default values
                pChangeData[num].playerInfo.bAllowLookon = TRUE;
                pChangeData[num].playerInfo.table = INVALID_TABLE;
                pChangeData[num].playerInfo.seat = INVALID_SEAT;
                break;

            case EVT_LEAVE:
                pChangeData[num].event = EVT_LEAVE;
                pChangeData[num].playerInfo.id = id;
                break;

            case EVT_CHGSTATE:
                pChangeData[num].event = EVT_CHGSTATE;
                pChangeData[num].playerInfo.id = id;

                pChangeData[num].playerInfo.state = (PLAYER_STATE)data[offset];
                offset += 1;

                if (pChangeData[num].playerInfo.state == STATE_IDLE) {
                    pChangeData[num].playerInfo.bAllowLookon = data[offset] >> 7;
                    offset += 1;
                } else {
                    pChangeData[num].playerInfo.bAllowLookon = data[offset] >> 7;
                    pChangeData[num].playerInfo.seat = data[offset] & 0x0F;
                    offset += 1;

                    pChangeData[num].playerInfo.table = data[offset];
                    offset += 1;
                }

                if ((data[offset] & 0x80) == 0) { // Not contain STATISTICS
                    pChangeData[num].bContainStatistics = FALSE;
                    offset += 1;
                } else {
                    pChangeData[num].bContainStatistics = TRUE;
                    offset += 1;

                    // ���֡���Ϸ������Ӯ�������ܴ���
                    pChangeData[num].playerInfo.nScore = ((data[offset + 0] << 24) | (data[offset + 1] << 16) |
                        (data[offset + 2] << 8) | (data[offset + 3] << 0));
                    offset += 4;

                    pChangeData[num].playerInfo.nTotalGames = ((data[offset + 0] << 24) | (data[offset + 1] << 16) |
                        (data[offset + 2] << 8) | (data[offset + 3] << 0));
                    offset += 4;

                    pChangeData[num].playerInfo.nWinGames = ((data[offset + 0] << 24) | (data[offset + 1] << 16) |
                        (data[offset + 2] << 8) | (data[offset + 3] << 0));
                    offset += 4;

                    pChangeData[num].playerInfo.nRunawayTimes = ((data[offset + 0] << 24) | (data[offset + 1] << 16) |
                        (data[offset + 2] << 8) | (data[offset + 3] << 0));
                    offset += 4;
                }

                // Main thread may ignore other fields when process the CHANGE_DATA
                // which identified by EVT_CHGSTATE
                break;
        }

        num++;
    }

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    PostMessageToMain(TM_RECV_PLAYER_STATE_CHANGE, (WPARAM)count, (LPARAM)pChangeData);
}

void On_NMSG_ACK_GAMER_TAKE_SEAT(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    BOOL bCanSit = data[offset] == 0 ? FALSE : TRUE;
    offset += 1;

    // ����������£��򲻷�����Ϣ�����߳�
    if (bCanSit == TRUE) {
        int table = data[offset];
        offset += 1;

        int seat = data[offset];
        offset += 1;

        int id = (data[offset] << 8) | data[offset + 1];
        offset += 2;

        assert(len == offset);
        UNREFERENCED_PARAMETER(len);

        WPARAM wp = MAKEWPARAM(table, seat);
        LPARAM lp = (LPARAM)id;
        //PostMessageToMain(TM_GAMER_TAKE_SEAT, wp, lp);

        //
        // ��������������ӵ���Ϣ�����̴߳���Ҫ��Щ������������һ���̶��ϱ���
        // ���������⣺��ҿ��ٵ��������ͬ�����ӣ����������ܻ�ͬʱ����������
        // �������ӡ�
        //
        PostThreadMessage(g_nMainThreadId, TM_GAMER_TAKE_SEAT, wp, lp);
    }
}

void On_NMSG_ACK_LOOKON_TAKE_SEAT(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    BOOL bCanSit = data[offset] == 0 ? FALSE : TRUE;
    offset += 1;

    // ����������£��򲻷�����Ϣ�����߳�
    if (bCanSit == TRUE) {
        int table = data[offset];
        offset += 1;

        int seat = data[offset];
        offset += 1;

        int id = (data[offset] << 8) | data[offset + 1];
        offset += 2;

        assert(len == offset);
        UNREFERENCED_PARAMETER(len);

        WPARAM wp = MAKEWPARAM(table, seat);
        LPARAM lp = (LPARAM)id;
        //PostMessageToMain(TM_LOOKON_TAKE_SEAT, wp, lp);

        //
        // �Թ��������������ӵ���Ϣ�����̴߳���Ҫ��Щ������������һ���̶��ϱ���
        // ���������⣺��ҿ��ٵ��������ͬ�����ӣ����������ܻ�ͬʱ����������
        // �������ӡ�
        //
        PostThreadMessage(g_nMainThreadId, TM_LOOKON_TAKE_SEAT, wp, lp);
    }
}

void On_NMSG_ACK_GAMER_LEAVE_SEAT(BYTE* pMsgBody, int nMsgBodyLen)
{
    // ������뿪��λǰ����Ҫ����REQ_GAMER_LEAVE_SEAT֪ͨ��������Ȼ����Թر���Ϸ�����ڡ�
    // �������յ�����Ϣ�󣬽�֪ͨ��Ϸ��������Ա������������ң���ĳ������뿪��
    // Ȼ�󽫸����״̬��ΪIDLE������֪��ʱ�㲥�̡߳�
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int table = data[offset];
    offset++;

    int seat = data[offset];
    offset++;

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    WPARAM wp = MAKEWPARAM(table, seat);
    LPARAM lp = 0;

    PostMessageToMain(TM_GAMER_LEAVE_SEAT, wp, lp);
}

void On_NMSG_ACK_LOOKON_LEAVE_SEAT(BYTE* pMsgBody, int nMsgBodyLen)
{
    // ���Թ����뿪��λǰ����Ҫ����REQ_LOOKON_LEAVE_SEAT֪ͨ��������Ȼ����Թر���Ϸ�����ڡ�
    // �������յ�����Ϣ�󣬽�֪ͨ��Ϸ��������Ա������������ң���ĳ������뿪��
    // Ȼ�󽫸����״̬��ΪIDLE������֪��ʱ�㲥�̡߳�
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int table = data[offset];
    offset++;

    int seat = data[offset];
    offset++;

    int id = (data[offset] << 8) | data[offset + 1];
    offset += 2;

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    WPARAM wp = MAKEWPARAM(table, seat);
    LPARAM lp = (LPARAM)id;

    PostMessageToMain(TM_LOOKON_LEAVE_SEAT, wp, lp);
}

void On_NMSG_ACK_GAMER_READY(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int table = data[offset];
    offset++;

    int seat = data[offset];
    offset++;

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    WPARAM wp = MAKEWPARAM(table, seat);
    LPARAM lp = 0;

    PostMessageToMain(TM_GAMER_READY, wp, lp);
}

void On_NMSG_REQ_VOTE_LORD(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int seat = data[offset];
    offset++;

    int score = data[offset];
    offset++;

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    WPARAM wp = MAKEWPARAM(seat, score);
    PostMessageToMain(TM_SVR_REQ_VOTE_LORD, wp, 0);
}

void On_NMSG_VOTE_LORD(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int seat = data[offset];
    offset++;

    int score = data[offset];
    offset++;

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    WPARAM wp = MAKEWPARAM(seat, score);
    PostMessageToMain(TM_SVR_NOTIFY_VOTE_LORD, wp, 0);
}

void On_NMSG_VOTE_LORD_FINISH(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    BOOL    bVote   = FALSE;
    int     seat    = INVALID_SEAT;
    int     score   = 0;
    
    bVote = data[offset] == 0 ? FALSE : TRUE;
    offset++;

    if (bVote == TRUE) {
        seat = data[offset];
        offset++;

        score = data[offset];
        offset++;
    }

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    WPARAM wp = MAKEWPARAM(seat, score);
    LPARAM lp = (LPARAM)bVote;
    PostMessageToMain(TM_VOTE_LORD_FINISH, wp, lp);
}

void On_NMSG_DISTRIBUTE_CARD(BYTE* pMsgBody, int nMsgBodyLen)
{
    // !!!
    // Special Requirements:
    // Main thread should free the memory.
    //
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    DISTRIBUTE_POKER* lpDistrPoker = (DISTRIBUTE_POKER*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
        sizeof(DISTRIBUTE_POKER));
    if (lpDistrPoker == NULL) {
        TRACE(_T("�������ڴ��ݷ��������Ƶ��ڴ�ʧ��\n"));
        return;
    }

    int num;

    // seat 0
    num = data[offset];
    offset++;
    
    lpDistrPoker->numSeat0 = num;
    for (int i = 0; i < num; i++) {
        lpDistrPoker->pokerSeat0[i] = data[offset + i];
    }
    offset += num;

    // seat 1
    num = data[offset];
    offset++;
    
    lpDistrPoker->numSeat1 = num;
    for (int i = 0; i < num; i++) {
        lpDistrPoker->pokerSeat1[i] = data[offset + i];
    }
    offset += num;

    // seat 2
    num = data[offset];
    offset++;

    lpDistrPoker->numSeat2 = num;
    for (int i = 0; i < num; i++) {
        lpDistrPoker->pokerSeat2[i] = data[offset + i];
    }
    offset += num;

    // under cards
    num = data[offset];
    offset++;
    assert(num == 3);

    for (int i = 0; i < num; i++) {
        lpDistrPoker->underCards[i] = data[offset + i];
    }
    offset += num;

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    PostMessageToMain(TM_DISTRIBUTE_POKER, 0, (LPARAM)lpDistrPoker);
}

void On_NMSG_OUTPUT_CARD(BYTE* pMsgBody, int nMsgBodyLen)
{
    // !!!
    // Requirements:
    // While the main thread process TM_SVR_NOTIFY_OUTPUT_CARD message, it must
    // free the memory when not NULL.
    //
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    OUTPUT_POKER* lpOutputPoker = NULL;

    int seat = data[offset];
    offset++;

    int num = data[offset];
    offset++;

    if (num > 0) {
        assert(num <= PLAYER_MAX_CARDS_NUM);

        lpOutputPoker = (OUTPUT_POKER*)LocalAlloc(LMEM_FIXED, sizeof(OUTPUT_POKER));
        if (lpOutputPoker == NULL) {
            TRACE(_T("���ճ�������ʱ�������ڴ�ʧ�ܡ�\n"));
            return;
        }

        for (int i = 0; i < num; i++) {
            lpOutputPoker->poker[i] = data[offset];
            offset++;
        }
    }

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    WPARAM wp = MAKEWPARAM(seat, num);
    LPARAM lp = (LPARAM)lpOutputPoker;
    PostMessageToMain(TM_SVR_NOTIFY_OUTPUT_CARD, wp, lp);
}

void On_NMSG_REQ_OUTPUT_CARD(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int seat    = data[offset++];
    int type    = data[offset++];
    int value   = data[offset++];
    int num     = data[offset++];

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    WPARAM wp = MAKEWPARAM(seat, type);
    LPARAM lp = MAKELPARAM(value, num);
    PostMessageToMain(TM_SVR_REQ_OUTPUT_CARD, wp, lp);
}

void On_NMSG_GAME_OVER(BYTE* pMsgBody, int nMsgBodyLen)
{
    // !!!
    // Requirements:
    // Main thread must free the memory.
    //
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    GAME_RESULT* lpResult = (GAME_RESULT*)LocalAlloc(LMEM_FIXED, sizeof(GAME_RESULT));
    if (lpResult == NULL) {
        TRACE(_T("��������ʱ�����䱣����Ϸ������ڴ�ʧ�ܡ�\n"));
        return;
    }

    for (int i = 0; i < GAME_SEAT_NUM_PER_TABLE; i++) {
        lpResult->nScore[i] = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
        offset += 4;
    }

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    PostMessageToMain(TM_SVR_NOTIFY_GAME_OVER, 0, (LPARAM)lpResult);
}

void On_NMSG_CHATTING(BYTE* pMsgBody, int nMsgBodyLen)
{
    //!!!
    // Main thread must free memory if there is a valid memory pointer.
    //
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int seat = data[offset];
    offset++;

    int nMsgLen = 0;
    int nMsgIndex = 0;
    BOOL bPredefinedMsg = (data[offset] >> 7) == 0 ? TRUE : FALSE;

    if (bPredefinedMsg == TRUE) {
        nMsgIndex = data[offset] & 0x7F;
        offset++;

        assert(offset == len);

        WPARAM wp = MAKEWPARAM(seat, TRUE);
        LPARAM lp = (LPARAM)nMsgIndex;
        PostMessageToMain(TM_GAMER_CHATTING, wp, lp);
    } else {
        nMsgLen = data[offset] & 0x7F;
        assert(nMsgLen > 0);
        offset++;

        if (nMsgLen > 0) {
            BYTE* lpszMsg = (BYTE*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, nMsgLen + 2);
            if (lpszMsg == NULL) {
                TRACE(_T("����������Ϣʱ�������ڴ�ʧ�ܡ�\n"));
                return;
            }

            for (int i = 0; i < nMsgLen; i++) {
                lpszMsg[i] = data[offset++];
            }

            assert(len == offset);
            UNREFERENCED_PARAMETER(len);

            WPARAM wp = MAKEWPARAM(seat, FALSE);
            LPARAM lp = (LPARAM)lpszMsg;
            PostMessageToMain(TM_GAMER_CHATTING, wp, lp);
        }
    }
}

void On_NMSG_CONNECTION_LOST(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int seat = data[offset];
    offset++;

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    PostMessageToMain(TM_GAMER_DISCONNECTED, (WPARAM)seat, 0);
}

void On_NMSG_ACK_DELEGATE(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    int seat = data[offset];
    offset++;

    BOOL bDelegated = data[offset] > 0 ? TRUE : FALSE;
    offset++;

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);

    PostMessageToMain(TM_GAMER_DELEGATED, (WPARAM)seat, (LPARAM)bDelegated);
}

void On_NMSG_REQ_STOP_GAMING(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(data);

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);
}

void On_NMSG_ACK_STOP_GAMING(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(data);

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);
}

void On_NMSG_ACK_FORCE_STOP_GAMING(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(data);

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);
}

void On_NMSG_ALLOW_LOOKON(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(data);

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);
}

void On_NMSG_BROADCASTING(BYTE* pMsgBody, int nMsgBodyLen)
{
    int     offset  = 0;
    int     len     = nMsgBodyLen;
    BYTE*   data    = pMsgBody;

    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(data);

    assert(len == offset);
    UNREFERENCED_PARAMETER(len);
}

// ����������ӳɹ�����Ҫ���ȷ����Լ������ݵ�������
int SendReqRoomInfo(void)
{
    BYTE data[128] = { 0 };

    // msg code
    data[0] = NMSG_REQ_ROOM_INFO;

    // name length field
    int namelen = _tcslen(g_UserLogonInfo.szName) * sizeof(TCHAR);
    data[3] = (BYTE)namelen;

    // name field
    _tcscpy_s((TCHAR*)(data + 4), (128 - 4) / sizeof(TCHAR), g_UserLogonInfo.szName);

    // gender, state(omitted), image-index
    if (g_UserLogonInfo.bMale == FALSE) { // Female: 1, Male: 0
        data[4 + (BYTE)data[3]] = (BYTE)(0x80 | (g_UserLogonInfo.nImage & 0x0F));
    } else {
        data[4 + (BYTE)data[3]] = (BYTE)(0x00 | (g_UserLogonInfo.nImage & 0x0F));
    }

    // msg length field
    data[1] = (BYTE)((2 + namelen) >> 8);
    data[2] = (BYTE)((2 + namelen) >> 0);

    // crc field
    WORD crc = CommonUtil::CRC16(data, 1 + 2 + 1 + namelen + 1);
    data[5 + namelen + 0] = (BYTE)(crc >> 8);
    data[5 + namelen + 1] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 5 + namelen + 2);
}

int SendReqGamerTakeSeat(int table, int seat)
{
    BYTE data[16];

    data[0] = NMSG_REQ_GAMER_TAKE_SEAT;

    data[1] = 0;
    data[2] = 2;

    data[3] = (BYTE)table;
    data[4] = (BYTE)seat;

    WORD crc = CommonUtil::CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 7);
}

int SendReqLookonTakeSeat(int table, int seat)
{
    BYTE data[16];

    data[0] = NMSG_REQ_LOOKON_TAKE_SEAT;

    data[1] = 0;
    data[2] = 2;

    data[3] = (BYTE)table;
    data[4] = (BYTE)seat;

    WORD crc = CommonUtil::CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 7);
}

int SendReqGamerLeaveSeat(int table, int seat)
{
    BYTE data[16];

    data[0] = NMSG_REQ_GAMER_LEAVE_SEAT;

    data[1] = 0;
    data[2] = 2;

    data[3] = (BYTE)table;
    data[4] = (BYTE)seat;

    WORD crc = CommonUtil::CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 7);
}

int SendReqLookonLeaveSeat(int table, int seat)
{
    BYTE data[16];

    data[0] = NMSG_REQ_LOOKON_LEAVE_SEAT;

    data[1] = 0;
    data[2] = 2;

    data[3] = (BYTE)table;
    data[4] = (BYTE)seat;

    WORD crc = CommonUtil::CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 7);
}

int SendReqGamerReady(int table, int seat)
{
    BYTE data[16];

    data[0] = NMSG_REQ_GAMER_READY;

    data[1] = 0;
    data[2] = 2;

    data[3] = (BYTE)table;
    data[4] = (BYTE)seat;

    WORD crc = CommonUtil::CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 7);
}

int SendAckVoteLord(int seat, int score)
{
    BYTE data[16];

    data[0] = NMSG_ACK_VOTE_LORD;

    data[1] = 0;
    data[2] = 2;

    data[3] = (BYTE)seat;
    data[4] = (BYTE)score;

    WORD crc = CommonUtil::CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 7);
}

int SendAckOutputCard(int seat, POKER_PROPERTY* pp, int poker[])
{
    BYTE data[32];
    int offset = 0;

    data[0] = NMSG_ACK_OUTPUT_CARD;

    data[3] = (BYTE)seat;
    data[4] = (BYTE)pp->type;
    data[5] = (BYTE)pp->value;
    data[6] = (BYTE)pp->num;

    offset = 7;
    for (int i = 0; i < pp->num; i++) {
        data[offset + i] = (BYTE)poker[i];
    }

    offset += pp->num;

    // message body length
    data[1] = 0;
    data[2] = (BYTE)(4 + pp->num);

    WORD crc = CommonUtil::CRC16(data, offset);
    data[offset + 0] = (BYTE)(crc >> 8);
    data[offset + 1] = (BYTE)(crc >> 0);
    offset += 2;

    return SendDataToServer(data, offset);
}

int SendChatMessage(int seat, LPCTSTR lpszChatText)
{
    BYTE* pText = (BYTE*)lpszChatText;
    int nTextBytes = _tcslen(lpszChatText) * sizeof(TCHAR);
    int nValidBytes = (nTextBytes > 126) ? 126 : nTextBytes;

    BYTE data[136];
    int offset = 0;

    data[0] = NMSG_CHATTING;
    data[1] = (BYTE)((1 + 1 + nValidBytes) >> 8);
    data[2] = (BYTE)((1 + 1 + nValidBytes) >> 0);

    data[3] = (BYTE)seat;
    data[4] = (BYTE)nValidBytes | 0x80;

    offset = 5;
    for (int i = 0; i < nValidBytes; i++) {
        data[offset + i] = pText[i];
    }

    offset += nValidBytes;

    WORD crc = CommonUtil::CRC16(data, offset);
    data[offset + 0] = (BYTE)(crc >> 8);
    data[offset + 1] = (BYTE)(crc >> 0);
    offset += 2;

    return SendDataToServer(data, offset);
}

int SendChatMessage(int seat, int nMsgIndex)
{
    BYTE data[16];

    data[0] = NMSG_CHATTING;

    data[1] = 0;
    data[2] = 2;

    data[3] = (BYTE)seat;
    data[4] = (BYTE)nMsgIndex & 0x7F;

    WORD crc = CommonUtil::CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 7);
}

int SendReqDelegate(int seat, BOOL bDelegate)
{
    BYTE data[16];

    data[0] = NMSG_REQ_DELEGATE;

    data[1] = 0;
    data[2] = 2;

    data[3] = (BYTE)seat;
    data[4] = bDelegate ? 1 : 0;

    WORD crc = CommonUtil::CRC16(data, 5);
    data[5] = (BYTE)(crc >> 8);
    data[6] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 7);
}

int SendReqStopGaming(int seat, LPCTSTR lpszTextReason)
{
    //
    // 1. text limited to 126 bytes
    // 2. allow param Reason to be NULL
    //
    BYTE* pText = (BYTE*)lpszTextReason;

    int nTextBytes = 0;
    if (lpszTextReason != NULL) {
        nTextBytes = _tcslen(lpszTextReason) * sizeof(TCHAR);
    }

    int nValidBytes = (nTextBytes > 126) ? 126 : nTextBytes;

    BYTE data[136];
    int offset = 0;

    data[0] = NMSG_REQ_STOP_GAMING;
    data[1] = (BYTE)((1 + 1 + nValidBytes) >> 8);
    data[2] = (BYTE)((1 + 1 + nValidBytes) >> 0);

    data[3] = (BYTE)seat;
    data[4] = (BYTE)nValidBytes;

    offset = 5;
    for (int i = 0; i < nValidBytes; i++) {
        data[offset + i] = pText[i];
    }

    offset += nValidBytes;

    WORD crc = CommonUtil::CRC16(data, offset);
    data[offset + 0] = (BYTE)(crc >> 8);
    data[offset + 1] = (BYTE)(crc >> 0);
    offset += 2;

    return SendDataToServer(data, offset);
}

int SendAckStopGaming(int seat, int reqSeat, BOOL bPermit, LPCTSTR lpszTextReason)
{
    //
    // 1. text limited to 126 bytes
    // 2. allow param Reason to be NULL
    //
    BYTE* pText = (BYTE*)lpszTextReason;

    int nTextBytes = 0;
    if (lpszTextReason != NULL) {
        nTextBytes = _tcslen(lpszTextReason) * sizeof(TCHAR);
    }

    int nValidBytes = (nTextBytes > 126) ? 126 : nTextBytes;

    BYTE data[136];
    int offset = 0;

    data[0] = NMSG_ACK_STOP_GAMING;
    data[1] = (BYTE)((4 + nValidBytes) >> 8);
    data[2] = (BYTE)((4 + nValidBytes) >> 0);

    data[3] = (BYTE)seat;
    data[4] = bPermit ? 1 : 0;
    data[5] = (BYTE)reqSeat;
    data[6] = (BYTE)nValidBytes;

    offset = 7;
    for (int i = 0; i < nValidBytes; i++) {
        data[offset + i] = pText[i];
    }

    offset += nValidBytes;

    WORD crc = CommonUtil::CRC16(data, offset);
    data[offset + 0] = (BYTE)(crc >> 8);
    data[offset + 1] = (BYTE)(crc >> 0);
    offset += 2;

    return SendDataToServer(data, offset);
}

int SendReqForceStopGaming(int seat)
{
    BYTE data[8];

    data[0] = NMSG_REQ_FORCE_STOP_GAMING;
    
    data[1] = 0;
    data[2] = 1;

    data[3] = (BYTE)seat;

    WORD crc = CommonUtil::CRC16(data, 4);
    data[4] = (BYTE)(crc >> 8);
    data[5] = (BYTE)(crc >> 0);

    return SendDataToServer(data, 6);
}

int SendDataToServer(BYTE* buf, int len)
{
    int nSend = 0;

    if (g_ConSocket != INVALID_SOCKET) {
        nSend = send(g_ConSocket, (char*)buf, len, 0);
        if (nSend <= 0) {
            TRACE1(_T("Send data to server failed. [ERR: %d]\n"), WSAGetLastError());
        }
    }

    return nSend;
}

void PostMessageToMain(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef MAIN_THREAD_PROCESS_NMSG
    PostThreadMessage(g_nMainThreadId, nMsg, wParam, lParam);
#else
    MSG msg = { 0 };
    msg.message = nMsg;
    msg.wParam = wParam;
    msg.lParam = lParam;

    ProcessNetMessage(&msg);
#endif
}

