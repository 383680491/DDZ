//
// File: GameMainWnd.h
//
#pragma once

#define GAME_MAIN_WND_CLASS_NAME        _T("_GMW_CLASS_")

#define ID_BTN_SCORE0                   1000
#define ID_BTN_SCORE1                   1001
#define ID_BTN_SCORE2                   1002
#define ID_BTN_SCORE3                   1003
#define ID_BTN_START                    1004
#define ID_BTN_OPTION                   1005
#define ID_BTN_DELEGATE                 1006
#define ID_BTN_PREVROUND                1007
#define ID_BTN_PASS                     1008
#define ID_BTN_FOLLOW                   1009
#define ID_BTN_HINT                     1010

#define ID_CUR_POKER_WND                1011
#define ID_CUR_POKER_WND_UP             1012
#define ID_CUR_POKER_WND_DOWN           1013

#define GMW_BTN_WIDTH                   59
#define GMW_BTN_HEIGHT                  25

#define GMW_START_BTN_WIDTH             60
#define GMW_START_BTN_HEIGHT            32

#define GMW_BTN_H_GAP                   8
#define GMW_BTN_V_GAP                   4

#define GMW_DEF_BTN_MASK_CLR            RGB(255,255,255)
#define GMW_TEXT_CLR                    RGB(255,255,255)

#define GMW_LOGCLR_INVALID_OP           RGB(255,0,0)
#define GMW_LOGCLR_GAME_RESULT          RGB(0,255,0)
#define GMW_LOGCLR_NORMAL               RGB(255,255,255)
#define GMW_LOGCLR_SPECIAL              RGB(0,255,128)
#define GMW_LOGCLR_DELEGATE             RGB(255,192,0)
#define GMW_LOGCLR_DISCONNECT           RGB(255,0,0)
#define GMW_LOGCLR_CHAT                 RGB(0,0,0)

#define GMW_CX_LORD_SCORE               60
#define GMW_CY_LORD_SCORE               20

// ��ʾ��һ�ֳ��ƵĶ�ʱ��
#define ID_TIMER_SHOW_PREVROUND         100
#define SHOW_PREVROUND_TIMEOUT          1500

// ��ʾʱ��
#define ID_TIMER_SHOW_CLOCK             101
#define SHOW_CLOCK_TIMEOUT              1000

// ʱ�Ӷ�ʱ����
#define GMW_TIMEOUT_SECOND              30

// ʱ�ӱ���ʱ������
#define GMW_TIMEOUT_SECOND_ALARM        10

// ����������ʱ�Ĵ����������ù��ν��й���Ϸ
#define GMW_MAX_TIMEOUT_TIMES           2

// ����ʱ��������ʼ�㣨ʱ��ICONΪ64x64�ģ�����Ϊ13x23�ģ�
#define GMW_X_CLOCK_DIGITAL             20
#define GMW_Y_CLOCK_DIGITAL             23


class GameMainWnd
{
    HWND        m_hWnd;
    HWND        m_hWndParent;
    HINSTANCE   m_hInstance;

    HBITMAP     m_hBackBitmap;
    int         m_cxBackBitmap;
    int         m_cyBackBitmap;

    BOOL        m_bShow;

    // �Ӵ��ڽ���Ԫ��
    MyButton    m_btnStart;     // ׼����������ʼ��Ϸ��

    MyButton    m_btnScore0;    // ����
    MyButton    m_btnScore1;    // 1��
    MyButton    m_btnScore2;    // 2��
    MyButton    m_btnScore3;    // 3��

    MyButton    m_btnPass;      // ����
    MyButton    m_btnFollow;    // ����
    MyButton    m_btnHint;      // ��ʾ

    MyButton    m_btnOption;    // ����ѡ��
    MyButton    m_btnDelegate;  // �й���Ϸ
    MyButton    m_btnPrevRound; // ��һ�ֳ���

    UnderCardWnd    m_UnderCardWnd;     // ���ƴ���

    CurPokerWnd     m_CurPokerWnd;      // ��ǰ��ҵ��˿��ƴ���
    CurPokerWnd     m_CurPokerWndUp;    // �ϼ��˿��ƴ���
    CurPokerWnd     m_CurPokerWndDown;  // �¼��˿��ƴ���

    OutCardWnd      m_OutCardWndCur;    // ��ǰ��ҵĳ��ƴ���
    OutCardWnd      m_OutCardWndUp;     // �ϼҳ��ƴ���
    OutCardWnd      m_OutCardWndDown;   // �¼ҳ��ƴ���

    GamerVisualWnd  m_GamerVisualWndUp;     // �ϼ���Ҵ�ͷ��
    GamerVisualWnd  m_GamerVisualWndCur;    // ��ǰ��Ҵ�ͷ��
    GamerVisualWnd  m_GamerVisualWndDown;   // �¼���Ҵ�ͷ��

    BOOL            m_bIsLookon;    // ��¼�Լ��Ƿ�Ϊ�Թ��ߡ�������Ϸ������ʱ��Init()����
#define I_AM_LOOKON (m_bIsLookon == TRUE)

    int             m_nCurUserSeat; // ��ǰ��һ��Թ������ڵ����Ӻ�
#define CUR_SEAT_NUM    m_nCurUserSeat
#define DOWN_SEAT_NUM   ((m_nCurUserSeat + 1) % GAME_SEAT_NUM_PER_TABLE)
#define UP_SEAT_NUM     ((m_nCurUserSeat + 2) % GAME_SEAT_NUM_PER_TABLE)

#define IS_CUR_SEAT(seat)       ((seat) == CUR_SEAT_NUM)
#define IS_DOWN_SEAT(seat)      ((seat) == DOWN_SEAT_NUM)
#define IS_UP_SEAT(seat)        ((seat) == UP_SEAT_NUM)

    // ��ʶ�Ƿ��Ѿ���ʼ��Ϸ��
    // ���巢�ƺ�Ϊ��Ϸ��ʼ����ΪTRUE��GameOver�����˽е�������������;��ֹ
    // ��Ϸ�ľ���Ϊ��Ϸδ��ʼ����ΪFALSE��
    BOOL    m_bGameStarted;

    // ��ʶ�Ƿ��Ѿ���ʼ���ơ�
    // �����������֮��Ϊ��ʼ���ƣ���ΪTRUE��GameOver֮��ΪFALSE��
    BOOL    m_bOutputCardStarted;

    BOOL    m_bNoGamerVoteLord;         // ��¼������Ϸ�Ƿ�û���˽е���
    int     m_nNoGamerVoteLordTimes;    // ��¼���������ڼ���û����ҽе���

    int     m_nBombNum; // ��Ϸ�����г��ֵ�ը������

    BOOL    m_bReqOutputCard;       // �����������ҳ���
    POKER_PROPERTY  m_PokerProp;    // ��¼�����������ҳ��Ƶ�����

    int     m_nHintTimes;       // ��¼��ǰ�ǵڼ���������ʾ�����ƺ�򲻳�����λΪ0
    POKER_CLASS_TABLE m_pct;    // �˿��Ʒ�������ڹ�����ʾ

    int     m_nLordSeat;    // ��¼���������Ӻ�
    int     m_nLordScore;   // ��¼�����еķ�

    BOOL    m_bGamerDelegated[GAME_SEAT_NUM_PER_TABLE]; // ����Ƿ��й���Ϸ
    BOOL    m_bGamerConnLost[GAME_SEAT_NUM_PER_TABLE];  // ����Ƿ����

    // ����ͷ��ľ���
    RECT    m_rcLordCur;
    RECT    m_rcLordUp;
    RECT    m_rcLordDown;
    
    // �й�ͼ��ľ���
    RECT    m_rcDelegatedCur;
    RECT    m_rcDelegatedUp;
    RECT    m_rcDelegatedDown;
    
    // ����ͼ��ľ���
    RECT    m_rcConnLostCur;
    RECT    m_rcConnLostUp;
    RECT    m_rcConnLostDown;

    // ��ʱ�ӱ�ľ���
    RECT    m_rcClockCur;
    RECT    m_rcClockUp;
    RECT    m_rcClockDown;

    int     m_nTimeOutCur;
    int     m_nTimeOutUp;
    int     m_nTimeOutDown;

    BOOL    m_bShowClockCur;
    BOOL    m_bShowClockUp;
    BOOL    m_bShowClockDown;

    // ��ǰ����ڽе���������ڼ䣬������ʱ�Ĵ���
    int     m_nTimeOutTimes;

    // �ж��ϡ��¼��Ƿ��Ѿ�׼��������������Ϸ����֮���Ƿ��������ƴ�������
    BOOL    m_bGamerReadyUp;
    BOOL    m_bGamerReadyDown;

public:
    GameMainWnd(void);
    ~GameMainWnd(void);

    static ATOM GameMainWndRegister(HINSTANCE hInstance);
    static LRESULT CALLBACK GameMainWndProc(HWND, UINT, WPARAM, LPARAM);

protected:
    void CreateChildWindows(void);

    void DrawIconLord(HDC hdc);
    void DrawIconConnLost(HDC hdc);
    void DrawIconDelegated(HDC hdc);
    void DrawIconBomb(HDC hdc);
    void DrawLordScoreString(HDC hdc);
    void DrawClock(HDC hdc);

    void OnPaint(HDC hdc);
    void OnSize(WPARAM wParam, LPARAM lParam);

    void OnTimer(WPARAM wParam, LPARAM lParam);
    void OnLButtonDblClk(WPARAM wParam, LPARAM lParam);
    void OnRButtonDown(WPARAM wParam, LPARAM lParam);

    void OnTimerClock(void);

    void OnCommand(UINT nCtrlId);
    void On_ID_BTN_START(void);
    void On_ID_BTN_SCORE0(void);
    void On_ID_BTN_SCORE1(void);
    void On_ID_BTN_SCORE2(void);
    void On_ID_BTN_SCORE3(void);
    void On_ID_BTN_PASS(void);
    void On_ID_BTN_FOLLOW(void);
    void On_ID_BTN_HINT(void);
    void On_ID_BTN_OPTION(void);
    void On_ID_BTN_DELEGATE(void);
    void On_ID_BTN_PREVROUND(void);

    void ShowVoteLordButtons(int nCurScore, BOOL bShow = TRUE);
    void ShowOutCardButtons(BOOL bFirstOutput, BOOL bShow = TRUE);

    void WriteLog(LPCTSTR lpszText, COLORREF clrText = RGB(0,0,0));
    void RedrawLordScoreRect(void);
    void ClearLostAndDelegatedIcon(void);

public:
    HWND Create(int x, int y, int cx, int cy, HWND hWndParent, HINSTANCE hInstance);
    BOOL SetBackBitmap(HBITMAP hBmp);
    void SetWindowRect(int x, int y, int cx, int cy);
    void Show(BOOL bShow = TRUE);
    BOOL IsVisible(void);

    void Init(BOOL bIsLookon);
    void SetCurrentUserSeat(int seat);

    void GamerTakeSeat(int id, int seat);
    void GamerLeaveSeat(int id, int seat);
    void GamerReady(int id, int seat);

    void GamerCanStart(void);

    void DistributePokerCards(int seat, int poker[], int num);
    void SetUnderPokerCards(int poker[], int num);

    void ReqVoteLord(int seat, int score); // ����������ͻ��˽з�
    void GamerVoteLord(int seat, int score); // ������֪ͨ�ͻ��ˣ�ĳ��ҽм���
    void VoteLordFinish(BOOL bVoteSucceed, int nLordSeat, int nLordScore);
    void ReqOutputCard(int seat, POKER_PROPERTY* pp);
    void OutputCard(int seat, int num, int poker[]);
    void GameOver(GAME_RESULT* lpResult);

    void OutputChatText(int seat, int nMsgIndex);
    void OutputChatText(int seat, LPCTSTR lpszChatText);

    void GamerDelegate(int seat, BOOL bDelegated);
    void GamerDisconnected(int seat);
};