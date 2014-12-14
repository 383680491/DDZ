//
// pokerhint.h
//
//      �����ϼ��������ƣ���ʾ��ҿ�����ʲô��ȥ�ӡ�
//      ʹ��������ʡ�ռ䣬���ǵ��������㣬����ʹ�þ�̬����ṹ��
//

#ifndef __POKER_HINT_H_
#define __POKER_HINT_H_

// �������Ͷ����Ƶ� PokerLib.h �ļ���

//// ����
//typedef struct HT_SINGLE_t {
//    int     idx;
//    int     value;
//} HT_SINGLE;
//
//// ����
//typedef struct HT_PAIR_t {
//    int     idx0;
//    int     idx1;
//    int     value;
//} HT_PAIR;
//
//// ����
//typedef struct HT_TRIANGLE_t {
//    int     idx0;
//    int     idx1;
//    int     idx2;
//    int     value;
//} HT_TRIANGLE;
//
//// ����
//typedef struct HT_FOUR_t {
//    int     idx0;
//    int     idx1;
//    int     idx2;
//    int     idx3;
//    int     value;
//} HT_FOUR;
//
//// ˳��
//typedef struct HT_SERIES_t {
//    int     idx[12];
//    int     num;
//    int     value;
//} HT_SERIES;
//
//// ����
//typedef struct HT_SERIES_PAIR_t {
//    int     idx[20];
//    int     num;
//    int     value;
//} HT_SERIES_PAIR;
//
//// ����
//typedef struct HT_SERIES_TRIANGLE_t {
//    int     idx[18];
//    int     num;
//    int     value;
//} HT_SERIES_TRIANGLE;
//
//// ����
//typedef struct HT_SERIES_FOUR_t {
//    int     idx[20];
//    int     num;
//    int     value;
//} HT_SERIES_FOUR;
//
//// ˫��ը��
//typedef struct HT_KINGBOMB_t {
//    int     idx0;
//    int     idx1;
//    int     value;
//} HT_KINGBOMB;
//
//// �˿��Ʒ����
//typedef struct POKER_CLASS_TABLE_t {
//    int             builded;
//    int             count;
//
//    HT_SINGLE       one[14];
//    int             num1;
//
//    HT_PAIR         two[10];
//    int             num2;
//
//    HT_TRIANGLE     three[6];
//    int             num3;
//
//    HT_FOUR         four[5];
//    int             num4;
//
//    HT_KINGBOMB     kingbomb;
//    int             has_king_bomb;
//
//    HT_SERIES       sone[2];
//    int             num11;
//
//    HT_SERIES_PAIR    stwo[3];
//    int                 num22;
//
//    HT_SERIES_TRIANGLE  sthree[2];
//    int                 num33;
//
//    HT_SERIES_FOUR      sfour[2];
//    int                 num44;
//} POKER_CLASS_TABLE;


// ����һ��ը�������ڽӷ�ը�����κ���
#define RETRIEVE_BOMB()\
{\
    for (i = 0; i < pct->num4; i++) {\
        if (count >= times) {\
            out->type   = BOMB;\
            out->num    = 4;\
            out->value  = pct->four[i].value;\
            vec[0]      = pct->four[i].idx0;\
            vec[1]      = pct->four[i].idx1;\
            vec[2]      = pct->four[i].idx2;\
            vec[3]      = pct->four[i].idx3;\
            return true;\
        } else {\
            count++;\
        }\
    }\
    if (pct->has_king_bomb != 0) {\
        if (count >= times) {\
            out->type   = BOMB;\
            out->num    = 2;\
            out->value  = pct->kingbomb.value;\
            vec[0]      = pct->kingbomb.idx0;\
            vec[1]      = pct->kingbomb.idx1;\
            return true;\
        } else {\
            count++;\
        }\
    }\
}

#endif
