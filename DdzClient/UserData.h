//
// File: UserData.h
//
#pragma once

// ��һ�����Ϣ
typedef struct _USER_INFO_t {
    int             id;
    TCHAR           name[MAX_USER_NAME_LEN];
    PLAYER_GENDER   gender;
    int             imageIdx;
    PLAYER_STATE    state;
    BOOL            bAllowLookon;
    int             table;
    int             seat;

    int             nScore;         // ����
    int             nTotalGames;    // ����Ϸ����
    int             nWinGames;      // Ӯ�ľ���
    int             nRunawayTimes;  // ���ܴ���
} USER_INFO, *LPUSER_INFO, PLAYER_INFO, *LPPLAYER_INFO;

// ��һ�����Ϣ������ valid ������ָʾ�������Ϣ�Ƿ���Ч
typedef struct _USER_DATA_t {
    BOOL            valid;
    PLAYER_INFO     playerInfo;
} USER_DATA, *LPUSER_DATA, PLAYER_DATA, *LPPLAYER_DATA;

// ����״̬�仯�������Ϣ
typedef struct _USER_CHG_DATA_t {
    STATE_CHANGE_EVENT  event;
    PLAYER_INFO     playerInfo;
    BOOL            bContainStatistics; // �Ƿ�Я��������ͳ����Ϣ���������� EVT_CHGSTATE
} PLAYER_CHG_DATA, *LPPLAYER_CHG_DATA;

// ���������ͻ��˷���
typedef struct _DISTRIBUTE_POKER_DATA_t {
    int numSeat0;
    int pokerSeat0[PLAYER_MAX_CARDS_NUM];

    int numSeat1;
    int pokerSeat1[PLAYER_MAX_CARDS_NUM];

    int numSeat2;
    int pokerSeat2[PLAYER_MAX_CARDS_NUM];

    int underCards[UNDER_CARDS_NUM];
} DISTRIBUTE_POKER, *LPDISTRIBUTE_POKER;

// ��¼����
typedef struct _OUTPUT_POKER_DATA_t {
    //int num;
    int poker[PLAYER_MAX_CARDS_NUM];
} OUTPUT_POKER, *LPOUTPUT_POKER;

// ��¼�����Ϸ���÷�
typedef struct _GAME_RESULT_t {
    int nScore[GAME_SEAT_NUM_PER_TABLE];
} GAME_RESULT, *LPGAME_RESULT;


extern PLAYER_DATA  g_PlayerData[];
extern int          g_nLocalPlayerId;


BOOL ResetPlayerDataTable(void);

int GetUserImageIndex(int nUserId);
int GetUserStatusImageIndex(int nUserId);

LPCTSTR GetUserNameStr(int nUserId);
LPCTSTR GetUserLevelStr(int nUserId);

int GetUserScore(int nUserId);
int GetUserWinGames(int nUserId);
int GetUserTotalGames(int nUserId);
int GetUserRunawayTimes(int nUserId);

int GetGamerId(int table, int seat);
int* GetLookonIdsArrayPtr(int table, int seat);

PLAYER_GENDER GetUserGender(int nUserId);
PLAYER_GENDER GetGamerGender(int table, int seat);

int GetLocalUserId(void);
PLAYER_STATE GetLocalUserState(void);
int GetLocalUserTableId(void);
int GetLocalUserSeatId(void);

BOOL IsGamerAllowLookon(int nUserId);
BOOL IsGamerAllowLookon(int table, int seat);
