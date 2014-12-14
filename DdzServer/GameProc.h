//
// GameProc.h
//
//  ��������Ϸ�߼�����
//
#pragma once

// ��Ϸ�����߳���Ϣ���ṩ�������̷߳�����Ϣ����Ϸ�����߳�
#define TM_RECEIVE_DATA                     (WM_USER + 101)
#define TM_GAMER_ENTER                      (WM_USER + 102)
#define TM_LOOKON_ENTER                     (WM_USER + 103)
#define TM_GAMER_LEAVE                      (WM_USER + 104)
#define TM_LOOKON_LEAVE                     (WM_USER + 105)
#define TM_GAMER_READY                      (WM_USER + 106)
#define TM_CONNECTION_LOST                  (WM_USER + 107)
#define TM_QUIT                             (WM_USER + 108)

// ��Ϸ�����߳��Լ����Լ�������Ϣ
#define TM_START_GAME                       (WM_USER + 201)
#define TM_REQ_VOTE_LORD                    (WM_USER + 202)
#define TM_REQ_OUTPUT_CARD                  (WM_USER + 203)


// ��Ϸ��;�������һ���۷�
#define RUN_AWAY_MULTIPLE                   1

// ������е���
typedef struct PLAYER_POKER_INFO_t {
    int         num;
    int         pokers[PLAYER_MAX_CARDS_NUM];
} PLAYER_POKER_INFO, *LPPLAYER_POKER_INFO;

// ��Ϸ������Ϣ
typedef struct GAME_INFO_t {
    BOOL    bStartGame;         //��ʶ��Ϸ�Ѿ���ʼ
    BOOL    bStartOuputCard;    //��ʶ��ʼ����
    int     nLordSeat;          //���������Ӻ�
    int     nWinnerSeat;        //���Ӯ�ҵ����Ӻ�
    int     nLordScore;         //�����еķ�������Ϸ�׷֣�
    int     nLordOutputTimes;   //�������ƵĴ��������Ϊ1���������ʱ��Ҫ��һ���䣨���أ�
    int     nMultiple;          //����
    int     nActiveSeat;        //�������ȴ��е�������Ƶ����Ӻ�
    BOOL    bUpPlayerNotFollow; //�ϼ�û����
    int     nVoteLordFailTimes; //û�˽е����Ĵ������������2�������·��ƣ�����ҵ�����״̬    
    BOOL    bReady[GAME_SEAT_NUM_PER_TABLE]; //׼������
    BOOL    bVoted[GAME_SEAT_NUM_PER_TABLE]; //�Ƿ�й�����
    BOOL    bDelegated[GAME_SEAT_NUM_PER_TABLE]; //����Ƿ�ѡ�����й���Ϸ
    BOOL    bConnectionLost[GAME_SEAT_NUM_PER_TABLE]; //�����ӵ�����Ƿ��Ѿ�����
    int     underPokerCards[UNDER_CARDS_NUM]; // ����
    PLAYER_POKER_INFO   pokerCards[GAME_SEAT_NUM_PER_TABLE]; //������е���
    GAME_SEAT_INFO      seatInfo[GAME_SEAT_NUM_PER_TABLE]; //���Ӷ�Ӧ����Ҽ����Թ���
    POKER_PROPERTY      outputPokerProperty; // �������������Ƶ�����
} GAME_INFO, *LPGAME_INFO;


// ������Ϸ�����߳�
BOOL BeginGameThreads(void);

// ������Ϸ�߳�
BOOL EndGameThreads(void);

// ��Ϸ�����̵߳���ں���
DWORD GameThreadProc(LPVOID lpData);

// ����ͻ��˷��͹�������Ϣ����
static void OnReceiveData(int id, int len, BYTE* buf);

static void OnGamerEnter(int id, int table, int seat);
static void OnGamerLeave(int id, int table, int seat);
static void OnLookonEnter(int id, int table, int seat);
static void OnLookonLeave(int id, int table, int seat);
static void OnGamerReady(int id, int table, int seat);
static void OnConnectionLost(int id, int table, int seat);

static void StartGame(int table);
static void EndGame(int table);

static void InitPlayerPokerData(int table);
static void DistributePoker(int table, int ids[], int idcount);

static void ReqVoteLord(int table, int seat, int score);
static void VoteLordDelegated(int table, int seat);

static void OnAckVoteLord(int table, BYTE* pMsgBody, int nMsgBodyLen);
static void VoteLordFailed(int table);
static void VoteLordSucceed(int table, int seat, int score);

static void BroadcastInTable(int table, char* buf, int len);
static void BroadcastInTableExceptId(int table, char* buf, int len, int id);

static void ReqOutputCard(int table, int seat, POKER_PROPERTY* pp);
static void OutputCardDelegated(int table, int seat, POKER_PROPERTY* req);

static void RemoveOutputPokersByIndex(int table, int seat, int indexes[], int num);
static void RemoveOutputPokers(int table, int seat, BYTE pokers[], int num);

static void GameOver(int table, int nWinnerSeat);
static void OnAckOutputCard(int table, BYTE* pMsgBody, int nMsgBodyLen);
static void OnReqStopGaming(int table, BYTE* pMsgBody, int nMsgBodyLen);
static void OnAckStopGaming(int table, BYTE* pMsgBody, int nMsgBodyLen);
static void OnReqDelegate(int table, BYTE* pMsgBody, int nMsgBodyLen);
static void OnReqForceStopGaming(int table, BYTE* pMsgBody, int nMsgBodyLen);
static void OnAllowLookon(int table, BYTE* pMsgBody, int nMsgBodyLen);
static void OnChatMessage(int table, BYTE* pMsgBody, int nMsgBodyLen);

