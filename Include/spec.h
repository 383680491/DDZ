//
// File: spec.h
//
//  ��������Ϸ�����񡣷�������ͻ��˾�����ѭ��Щ����Ĳ�����
//

// ������Ĭ�������������ӵĶ˿ں�
#define DDZSERVER_PORT                      26008

// ���������������������
#define MAX_CONNECTION_NUM                  300

// ������֧�ֵ���Ϸ������
#define GAME_TABLE_NUM                      100

// ÿ����Ϸ���е���������
#define GAME_SEAT_NUM_PER_TABLE             3

// ÿ��������������Թ�������
#define MAX_LOOKON_NUM_PER_SEAT             2

// ����������������ֵ�����ַ�����
#define MAX_USER_NAME_LEN                   32

// �˿�����ز���
#define TOTAL_CARDS_NUM                     54
#define PLAYER_MAX_CARDS_NUM                20
#define PLAYER_INIT_CARDS_NUM               17
#define UNDER_CARDS_NUM                     3

// ��Ϸ��ʼ����û����ҽе���������������·��ơ�
// ����Ҷ����е����Ĵ�����������ֵ��������������·��ƣ�
// ������Ϸ���������Ϊ����״̬����������µ����ʼ���ܽ���׼��״̬��
// �ö���Ҳ��Ϊ�˱��⣬��������Ҷ��й���Ϸʱ�����е��������������᲻ͣ�����·��ơ�
#define SERVER_REDISTR_POKER_TIMES          2

#define INVALID_USER_ID                     -1
#define IS_INVALID_USER_ID(id)              (((id) < 0) || ((id) >= MAX_CONNECTION_NUM))

#define INVALID_TABLE                       -1
#define IS_INVALID_TABLE(table)             (((table) < 0) || ((table) >= GAME_TABLE_NUM))

#define INVALID_SEAT                        -1
#define IS_INVALID_SEAT(seat)               (((seat)  < 0) || ((seat)  >= GAME_SEAT_NUM_PER_TABLE))

