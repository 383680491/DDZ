//
// File: UserData.cpp
//
//  ������� User(Player) �Ļ�����Ϣ
//
#include "stdafx.h"
#include "GameSeatWnd.h"
#include "GameTableWnd.h"
#include "GameRoomWnd.h"
#include "UserData.h"

extern GameRoomWnd g_GameRoomWnd;


// ������ҵ���Ϣ�����������Ϊ��ҵ�ID
PLAYER_DATA     g_PlayerData[MAX_CONNECTION_NUM] = { 0 };
int             g_nLocalPlayerId = INVALID_USER_ID;


// ��λ�����Ϣ�����飩�����������������ǰ���ý��г�ʼ����
BOOL ResetPlayerDataTable(void)
{
    ZeroMemory(&g_PlayerData, sizeof(PLAYER_DATA) * MAX_CONNECTION_NUM);
    for (int i = 0; i < MAX_CONNECTION_NUM; i++) {
        g_PlayerData[i].playerInfo.bAllowLookon = TRUE;
        g_PlayerData[i].playerInfo.state = STATE_IDLE;
        g_PlayerData[i].playerInfo.table = INVALID_TABLE;
        g_PlayerData[i].playerInfo.seat = INVALID_SEAT;
    }

    g_nLocalPlayerId = INVALID_USER_ID;
    return TRUE;
}

// �û�ͷ��
int GetUserImageIndex(int nUserId)
{
    assert(!IS_INVALID_USER_ID(nUserId));
    return g_PlayerData[nUserId].playerInfo.imageIdx;
}

// ��ʾ�û�״̬��ͼ��
int GetUserStatusImageIndex(int nUserId)
{
    assert(!IS_INVALID_USER_ID(nUserId));
    PLAYER_STATE state = g_PlayerData[nUserId].playerInfo.state;

    //
    // ���״̬ͼ������Сͷ��ͼ��ĩβ
    //
    return (int)(state) - (int)(STATE_IDLE) + (IDB_USERHEAD_16_LAST - IDB_USERHEAD_16_FIRST + 1);
}

// �û��ǳ�
LPCTSTR GetUserNameStr(int nUserId)
{
    assert(!IS_INVALID_USER_ID(nUserId));
    return g_PlayerData[nUserId].playerInfo.name;
}

// ����
LPCTSTR GetUserLevelStr(int nUserId)
{
    UNREFERENCED_PARAMETER(nUserId);
    int score = GetUserScore(nUserId);

    if (score < 10) {
        return _T("����");
    } else if (score < 25) {
        return _T("�̹�");
    } else if (score < 40) {
        return _T("����");
    } else if (score < 80) {
        return _T("�軧");
    } else if (score < 140) {
        return _T("ƶũ");
    } else if (score < 230) {
        return _T("���");
    } else if (score < 365) {
        return _T("����");
    } else if (score < 500) {
        return _T("��ũ");
    } else if (score < 700) {
        return _T("��ũ");
    } else if (score < 1000) {
        return _T("�ƹ�");
    } else if (score < 1500) {
        return _T("����");
    } else if (score < 2200) {
        return _T("����");
    } else if (score < 3000) {
        return _T("С����");
    } else if (score < 4000) {
        return _T("�����");
    } else if (score < 5500) {
        return _T("С����");
    } else if (score < 7700) {
        return _T("�����");
    } else if (score < 10000) {
        return _T("֪��");
    } else if (score < 14000) {
        return _T("ͨ��");
    } else if (score < 20000) {
        return _T("֪��");
    } else if (score < 30000) {
        return _T("�ܶ�");
    } else if (score < 45000) {
        return _T("Ѳ��");
    } else if (score < 70000) {
        return _T("ة��");
    } else {
        return _T("����");
    }
}

// ����
int GetUserScore(int nUserId)
{
    assert(!IS_INVALID_USER_ID(nUserId));
    return g_PlayerData[nUserId].playerInfo.nScore;
}

// Ӯ����
int GetUserWinGames(int nUserId)
{
    assert(!IS_INVALID_USER_ID(nUserId));
    return g_PlayerData[nUserId].playerInfo.nWinGames;
}

// ��Ϸ�ܾ���
int GetUserTotalGames(int nUserId)
{
    assert(!IS_INVALID_USER_ID(nUserId));
    return g_PlayerData[nUserId].playerInfo.nTotalGames;
}

// ���ܴ���
int GetUserRunawayTimes(int nUserId)
{
    assert(!IS_INVALID_USER_ID(nUserId));
    return g_PlayerData[nUserId].playerInfo.nRunawayTimes;
}

// �ж�ĳ����Ƿ������Թ�
BOOL IsGamerAllowLookon(int nUserId)
{
    assert(!IS_INVALID_USER_ID(nUserId));
    return g_PlayerData[nUserId].playerInfo.bAllowLookon;
}

// �ж�ĳ���ӵ�����Ƿ������Թ�
BOOL IsGamerAllowLookon(int table, int seat)
{
    int nUserId = GetGamerId(table, seat);
    if (IS_INVALID_USER_ID(nUserId)) {
        return TRUE; // No gamer sit yet. default is TRUE.
    }
    return IsGamerAllowLookon(nUserId);
}

// ��ȡ��Ϸ����ҵ�ID
int GetGamerId(int table, int seat)
{
    return g_GameRoomWnd.GetGamerId(table, seat);
}

// ��ȡ��Ϸ���Թ��ߵ�ID����ָ��
int* GetLookonIdsArrayPtr(int table, int seat)
{
    return g_GameRoomWnd.GetLookonIdsArrayPtr(table, seat);
}

// ������ҵ�ID
int GetLocalUserId(void)
{
    return g_nLocalPlayerId;
}

// �������״̬
PLAYER_STATE GetLocalUserState(void)
{
    if (IS_INVALID_USER_ID(g_nLocalPlayerId)) {
        return STATE_IDLE;
    }

    return g_PlayerData[g_nLocalPlayerId].playerInfo.state;
}

// �������������Ϸ����
int GetLocalUserTableId(void)
{
    if (IS_INVALID_USER_ID(g_nLocalPlayerId)) {
        return INVALID_TABLE;
    }

    return g_PlayerData[g_nLocalPlayerId].playerInfo.table;
}

// ��������������Ӻ�
int GetLocalUserSeatId(void)
{
    if (IS_INVALID_USER_ID(g_nLocalPlayerId)) {
        return INVALID_SEAT;
    }

    return g_PlayerData[g_nLocalPlayerId].playerInfo.seat;
}

// �û��Ա�
PLAYER_GENDER GetUserGender(int nUserId)
{
    assert(!IS_INVALID_USER_ID(nUserId));
    return g_PlayerData[nUserId].playerInfo.gender;
}

// ����Ա�
PLAYER_GENDER GetGamerGender(int table, int seat)
{
    assert(!IS_INVALID_TABLE(table));
    assert(!IS_INVALID_SEAT(seat));

    int nGamerId = GetGamerId(table, seat);
    return GetUserGender(nGamerId);
}