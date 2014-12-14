//
// File: GameTableWnd.h
//
#pragma once

#define GAME_TABLE_WND_CLASS_NAME       _T("_GAME_TABLE_WND_")

// ��Ϸ��λͼ�ߴ�
#define CX_TABLE_BMP                    152
#define CY_TABLE_BMP                    150

// ��ʾ��Ϸ���ŵľ��θ߶�
#define TABLE_NUMBER_RECT_HT            24

// �Ӵ�����Ϊ�����ڵĿؼ�ID
#define GAME_SEAT0_ID                   0
#define GAME_SEAT1_ID                   1
#define GAME_SEAT2_ID                   2

// �������Ӵ��ڵ�������ߴ�
#define X_SEAT0                         93
#define Y_SEAT0                         30
#define X_SEAT1                         58
#define Y_SEAT1                         98
#define X_SEAT2                         20
#define Y_SEAT2                         29

#define CX_SEAT                         32
#define CY_SEAT                         32

// ��ʾ����ǳƵľ���������ߴ�
#define X_SEAT0_NAME                    126
#define Y_SEAT0_NAME                    12
#define X_SEAT1_NAME                    92
#define Y_SEAT1_NAME                    100
#define X_SEAT2_NAME                    4
#define Y_SEAT2_NAME                    4

#define CX_SEAT0_NAME                   24
#define CY_SEAT0_NAME                   60
#define CX_SEAT1_NAME                   48
#define CY_SEAT1_NAME                   48
#define CX_SEAT2_NAME                   72
#define CY_SEAT2_NAME                   24

// ��ʾ׼����Ϸͼ���������ߴ�
#define X_SEAT0_READY                   80
#define Y_SEAT0_READY                   56
#define X_SEAT1_READY                   67
#define Y_SEAT1_READY                   80
#define X_SEAT2_READY                   52
#define Y_SEAT2_READY                   56

#define CX_SEAT_READY                   16
#define CY_SEAT_READY                   16

// ����ǰ������ڵ���Ϸ����һ�߿�
#define LOCAL_USER_TABLE_FRAME_CLR      RGB(255,255,255)

#define TABLE_ID_TEXT_COLOR             RGB(255,255,255)

class GameTableWnd
{
    HWND        m_hWnd;
    HWND        m_hWndParent;
    HINSTANCE   m_hInstance;
    int         m_nTableId;

    GameSeatWnd     m_SeatWnd[GAME_SEAT_NUM_PER_TABLE];

    int  m_nGamerId[GAME_SEAT_NUM_PER_TABLE];
    BOOL m_bReady[GAME_SEAT_NUM_PER_TABLE];
    BOOL m_bGameStarted;

public:
    GameTableWnd(void);
    ~GameTableWnd(void);
    static ATOM GameTableWndRegister(HINSTANCE hInstance);
    static LRESULT CALLBACK GameTableWndProc(HWND, UINT, WPARAM, LPARAM);

protected:
    void OnCommand(WPARAM wParam, LPARAM lParam);
    void OnSize(WPARAM wParam, LPARAM lParam);

public:
    HWND Create(int x, int y, int cx, int cy, UINT nID, HWND hWndParent, HINSTANCE hInstance);
    void SetWindowRect(int x, int y, int cx, int cy);

    int  GetGamerId(int seat);
    int* GetLookonIdsArrayPtr(int seat);

    void GamerTakeSeat(int nUserId, int seat);
    void GamerLeaveSeat(int nUserId, int seat);

    void LookonTakeSeat(int nUserId, int seat);
    void LookonLeaveSeat(int nUserId, int seat);

    BOOL IsGamerReady(int seat);

    void GamerGetReady(int seat, BOOL bReady = TRUE);
    void GameStarted(BOOL bStart = TRUE);

    void SetAllowLookon(int seat, BOOL bAllow = TRUE);
    void ResetGameTable(void);

    void PaintTableWnd(HDC hdc);
};