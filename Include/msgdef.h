//
// MsgDef.h
//
//  ��������Ϸ��������ͻ���ͨ����Ϣ���壬�Լ�������������ͻ�����ͬ���������Ͷ��塣
//  ��ͬ����Ϣ�룬����Ϣ��ĽṹҲ����ͬ������ο�����ĵ���
//
#pragma once


// ������Ϣ��
//#define NMSG_ACK_SVR_CONNECTION_INFO        0x00    // S->C ������Ӧ��ǰ�����������ǰ�����������������������
//#define NMSG_REQ_SVR_CONNECTION_INFO        0x01    // C->S �ͻ����������ǰ����״�����Ƿ�������Ϊ��

#define NMSG_REQ_ROOM_INFO                  0x10    // C->S �ͻ��˵�½�����������Ϣ���ͻ���Я���Լ�����Ϣ��
#define NMSG_ACK_ROOM_INFO                  0x11    // S->C ������Ӧ�������Ϣ

#define NMSG_PLAYER_CHANGE_STATE            0x20    // S->C ������֪ͨ�ͻ�������״̬�����仯�������Ϣ����ʱ�㲥��


#define NMSG_REQ_GAMER_TAKE_SEAT            0x40    // C->S �ͻ��ˣ���ҽ�ɫ����������λ��
#define NMSG_ACK_GAMER_TAKE_SEAT            0x41    // S->C ������Ӧ����Ϸ���ڹ㲥����������յ���Ӧ����Ϣ���Լ�������ģ���ʾ��������
#define NMSG_REQ_LOOKON_TAKE_SEAT           0x42    // C->S �ͻ��ˣ��Թ��߽�ɫ�������Թ�ĳ�����
#define NMSG_ACK_LOOKON_TAKE_SEAT           0x43    // S->C ������Ӧ����Ϸ���ڹ㲥����������յ���Ӧ����Ϣ���Լ�������ģ���ʾ��������
#define NMSG_REQ_GAMER_LEAVE_SEAT           0x44    // C->S �ͻ��ˣ���ҽ�ɫ�����߷��������Ѿ��뿪ĳ����λ
#define NMSG_ACK_GAMER_LEAVE_SEAT           0x45    // S->C ������������Ϸ������������ң�ĳ����Ϸ����뿪ĳ����λ
#define NMSG_REQ_LOOKON_LEAVE_SEAT          0x46    // C->S �ͻ��ˣ��Թ��߽�ɫ�����߷��������Ѿ��뿪ĳ����λ
#define NMSG_ACK_LOOKON_LEAVE_SEAT          0x47    // S->C ������������Ϸ������������ң�ĳ���Թ����Ѿ��뿪ĳ����λ
#define NMSG_REQ_GAMER_READY                0x48    // C->S �ͻ���֪ͨ���������Ѿ�׼����ʼ��Ϸ
#define NMSG_ACK_GAMER_READY                0x49    // S->C ������֪ͨ��Ϸ�������ͻ��ˣ�ĳ���׼������

// ��Ϸ����Ϣ��
#define NMSG_DISTRIBUTE_CARD                0x80    // S->C ���������ͻ��˷���
#define NMSG_REQ_VOTE_LORD                  0x81    // S->C ����������ͻ��˽з�
#define NMSG_ACK_VOTE_LORD                  0x82    // C->S �ͻ���Ӧ��������м���
#define NMSG_VOTE_LORD                      0x83    // S->C ������֪ͨ�ͻ��ˣ�ĳ��ҽм���
#define NMSG_VOTE_LORD_FINISH               0x84    // S->C ������֪ͨ�ͻ��ˣ������Ѿ��������е��Ǽ���
#define NMSG_REQ_OUTPUT_CARD                0x85    // S->C ����������ͻ��˳���
#define NMSG_ACK_OUTPUT_CARD                0x86    // C->S �ͻ���Ӧ�����������
#define NMSG_OUTPUT_CARD                    0x87    // S->C ������֪ͨ�ͻ��ˣ��ĸ���ҳ���ʲô��
#define NMSG_GAME_OVER                      0x88    // S->C ������֪ͨ�ͻ�����Ϸ����

#define NMSG_CHATTING                       0x6A    // C->S | S->C ������Ϣ
#define NMSG_CONNECTION_LOST                0x6B    // S->C ������֪ͨ����Ϸ�������ͻ��ˣ�ĳ��ҵ���
#define NMSG_REQ_DELEGATE                   0x6C    // C->S �ͻ��������й���Ϸ
#define NMSG_ACK_DELEGATE                   0x6D    // S->C ������֪ͨ����Ϸ�������ͻ��ˣ�ĳ������й���Ϸ
#define NMSG_REQ_STOP_GAMING                0x6E    // C->S �ͻ���������;�˳���Ϸ
                                                    // S->C ��������ת��������������������������ĳ���Ҫ��;�˳�
#define NMSG_ACK_STOP_GAMING                0x6F    // S->C ������Ӧ��ͻ�����;�˳���Ϸ������

#define NMSG_REQ_FORCE_STOP_GAMING          0x70    // C->S �ͻ���֪ͨ��������ǿ���ж���Ϸ
#define NMSG_ACK_FORCE_STOP_GAMING          0x71    // S->C ������֪ͨ��Ϸ����Ա��ĳ���ǿ���ж���Ϸ

// ��Ϸ������Ϣ��
#define NMSG_ALLOW_LOOKON                   0xA0    // C->S | S->C �Ƿ������Թ�
//#define NMSG_TABLE_PASSWORD                 0xA1    // C->S �ͻ��ˣ���һ����λ����ң�Ҫ��������Ϸ������

// ϵͳ��Ϣ��
#define NMSG_BROADCASTING                   0xFF    // S->C �������㲥һЩ��Ϣ


// ����Ա�
typedef enum PLAYER_GENDER_t {
    MALE,
    FEMALE
} PLAYER_GENDER;

// ���״̬
typedef enum {
    STATE_IDLE = 0,
    STATE_SIT,
    STATE_READY,
    STATE_GAMING,
    STATE_LOOKON
} PLAYER_STATE;

// �����ͻ�����ҽ��뷿�䡢�뿪���䡢�ı�״̬���¼�
// ��������ʱ����Щ�¼����͸����߿ͻ��ˣ��������ͻ��˲��ܻ�ȡ���µĴ����ĳ�Ա��Ϣ
typedef enum {
    EVT_ENTER,
    EVT_LEAVE,
    EVT_CHGSTATE
} STATE_CHANGE_EVENT;

