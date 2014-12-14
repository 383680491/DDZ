//
// AppDef.h
//
//

#pragma once

// �Զ��崰����Ϣ
#define WM_PAGE_SEL_CHANGING                (WM_USER + 20)
#define WM_PAGE_SEL_CHANGE                  (WM_USER + 21)
#define WM_PAGE_RETURN                      (WM_USER + 22)

#define WM_SHOW_HOME_PAGE                   (WM_USER + 30)
#define WM_CONNECT_SERVER                   (WM_USER + 31)
#define WM_USER_CLICK_SEAT                  (WM_USER + 32)

// ���ڴ���ػ��������ϢCOMBOBOX�༭��� ESC��RETURN �����¼�
#define WM_CHATMSG_VKEY                     (WM_USER + 40)

#define WM_WRITE_LOG                        (WM_USER + 41)
#define WM_GAME_OVER                        (WM_USER + 42)

// �߳���Ϣ
#define TM_CONNECTION_LOST                  (WM_APP + 101)
#define TM_RECV_SVR_ROOM_INFO               (WM_APP + 102)
#define TM_RECV_SVR_STATUS                  (WM_APP + 103)
#define TM_RECV_PLAYER_STATE_CHANGE         (WM_APP + 104)
#define TM_GAMER_TAKE_SEAT                  (WM_APP + 105)
#define TM_LOOKON_TAKE_SEAT                 (WM_APP + 106)
#define TM_GAMER_LEAVE_SEAT                 (WM_APP + 107)
#define TM_LOOKON_LEAVE_SEAT                (WM_APP + 108)
#define TM_GAMER_READY                      (WM_APP + 109)
#define TM_DISTRIBUTE_POKER                 (WM_APP + 110)
#define TM_SVR_REQ_VOTE_LORD                (WM_APP + 111)
#define TM_SVR_NOTIFY_VOTE_LORD             (WM_APP + 112)
#define TM_VOTE_LORD_FINISH                 (WM_APP + 113)
#define TM_SVR_REQ_OUTPUT_CARD              (WM_APP + 114)
#define TM_SVR_NOTIFY_OUTPUT_CARD           (WM_APP + 115)
#define TM_SVR_NOTIFY_GAME_OVER             (WM_APP + 116)
#define TM_GAMER_CHATTING                   (WM_APP + 117)
#define TM_GAMER_DELEGATED                  (WM_APP + 118)
#define TM_GAMER_DISCONNECTED               (WM_APP + 119)

//
// ������Ϊ��һʵ������
// Debug�汾������һ̨��������ж��ʵ����Release�汾����һ̨���������һ��ʵ��
//
#ifndef _DEBUG
#define RUN_AS_SINGLE_INSTANCE
#endif

//
// �����ڸ�������Ӵ���
//
// ��Ϸ���ڰ����ü����Ӵ��ڣ��統ǰ�ƴ��ڣ����ƴ��ڣ����ƴ��ڡ����ǵ����ڳߴ�仯
// ʱ������ʹ�������ʱ����˸����ʹ�øúꡣ��Щ���ڵĻ��ƹ��������丸���ڵ�����Ӧ
// �Ӵ��ڽӿ���ɡ��������������ڻ��ƽ���ʱ�����Ӵ���һ����Ƶ��ڴ�DC��Ȼ����ˢ��
// ��ʾ��Ļ�����ٴ��ڳߴ�䶯ʱ��ͼ����˸��
//
// ����ú�֮��CurPokerWnd, OutCardWnd, UnderCardWnd ԭ����ƵĻ�ͼ���뽫���ᱻ
// ���õ�����������Ҫ����ʱ�����丸������ɣ���������ǽ�����SaveParentBackground��
// GamerVisualWnd Ҳʹ�ô˻����ɸ����ڽ��л��ƣ�������ʹ�� PictureEx ����ͼ����
// ����Ҫ���游���ڱ������ݸ� PictureEx������Ӧ PictureEx ��ͼ����
//
// �ɸ����ڻ����Ӵ��ڵĻ��ƣ���Ϊ�˾������ⴰ�ڳߴ�䶯ʱ��˸������ġ���������
// �Ӵ�����CurPokerWnd��OutCardWnd�Ͳ����ٱ��游���ڱ���λͼ��ÿ���Ӵ�����Ҫ����
// ʱ�����ɸ�����ȥ��ɣ���Ȼ�����Լ�����ִ���˸���Ĵ��룬��Ϊ�����ڻ���ʱ������
// ���Ƹ��Ӵ����⣬�����������������Ӵ��ڡ�
//
// ��Ϊ��ҵ���ͼ�걻�����ڸ���ҵ�ǰ�ƴ������룬����ʹ�ú�PARETN_PAINT_CHILD����
// ��ҵ���ͼ�꽫����ǰ�ƴ����ڸ�ס��
//
#define PARENT_PAINT_CHILD

// ���������͸��ͻ��˵�������Ϣ���������̴߳������ǽ������ݵ��߳��Լ�����
//
// ��������� MAIN_THREAD_PROCESS_NMSG�������߳��ڴ�����Ϣʱ�����ܵ����˵���
// ģʽ�Ի�����Ϊ����������ֹ���߳̽��ղ������������ʱ���͵���Ϣ��������Ϣ��
// �Ӷ����¿ͻ������������ά���������Ϣ��һ�¡�
//
// û�������õĽ���취����ˣ��������úꡣ������Ϣ���������߳�ֱ�Ӵ���
//
//#define MAIN_THREAD_PROCESS_NMSG

// ʹ��CRC16�㷨
#define _USE_CRC16

// �˿���λͼ���
#define POKER_BACK_INDEX                    -1
#define LORD_POKER_BACK_INDEX               -2

#define POKER_BMP_UNIT_WIDTH                71
#define POKER_BMP_UNIT_HEIGHT               96

#define CPW_POKER_H_SPACE                   15
#define CPW_POKER_POPUP_SPACE               18
#define CPW_POKER_V_SPACE                   17

#define OCW_POKER_H_SPACE                   CPW_POKER_H_SPACE

#define UCW_POKER_H_SPACE                   8

// ��ҳ����
#ifdef USE_LOCAL_FILE_AS_HOMEPAGE
#define DEFAULT_HOME_PAGE                   _T("readme.txt")
#else
#define DEFAULT_HOME_PAGE                   _T("http://minigameimg.qq.com/help/rule81.html")
#endif

#define DDZ_GAME_SEC                        _T("GAME_ENV")
#define GAME_KEY_BG_IMAGE                   _T("bg_image")
#define GAME_BG_DEF_BMP_FILE                _T("game_bg.bmp")

#define DDZ_CLIENT_INI_FILE                 _T("DdzClient.ini")
#define SERVER_LIST_INI_FILE                _T("ServerList.ini")

// ��̬���ӿ�����
#define POIKER_LOGIC_DLL_NAME               _T("PokerLib.dll")
#define RES_IMAGE_DLL_NAME                  _T("ResImage.dll")
#define RES_POKER_DLL_NAME                  _T("ResPoker.dll")
#define RES_SOUND_DLL_NAME                  _T("ResSound.dll")
#define CWEBPAGE_DLL_NAME                   _T("CWebPage.dll")

// PageTabCtrl �Ĳ���
#define HOME_PAGE_TAB_TEXT                  _T("��Ϸ��ҳ")
#define ROOM_PAGE_TAB_TEXT                  _T("��Ϸ����")
#define GAME_PAGE_TAB_TEXT                  _T("������")

#define HOME_PAGE_TAB_PARAM                 0
#define ROOM_PAGE_TAB_PARAM                 1
#define GAME_PAGE_TAB_PARAM                 2

#define HOME_PAGE_TAB_INDEX                 0
#define ROOM_PAGE_TAB_INDEX                 1
#define GAME_PAGE_TAB_INDEX                 2

#define HOME_PAGE_TAB_IMAGE_INDEX           0
#define ROOM_PAGE_TAB_IMAGE_INDEX           1
#define GAME_PAGE_TAB_IMAGE_INDEX           2




