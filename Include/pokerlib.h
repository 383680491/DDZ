//
// PokerLib.h
//      POKER DLL ���������ͼ���������
//

#pragma once

// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵ĺ�ı�׼������
// �� DLL �е������ļ��������������϶���� POKERLIB_EXPORTS ���ű���ġ�
// ��ʹ�ô� DLL ���κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ���
// �κ�������Ŀ���Ὣ POKER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô�
// �궨��ķ�����Ϊ�Ǳ������ġ�
#ifdef POKERLIB_EXPORTS
#define POKER_API __declspec(dllexport)
#else
#define POKER_API __declspec(dllimport)
#endif

// ȡ��ϢASSERT��ʹ��
#ifndef _DEBUG
#ifdef assert
#undef assert
#endif
#define assert(arg)     void(0)
#endif

//
// �ṩ������ģ��ĳ��������Ͷ��壬�Լ� PokerLib.DLL �����ķ���
//

// һ���˿���
#define  POKER_ITEM_COUNT                   54


// poker unit
typedef enum POKER_UNIT_t {
    P3,
    P4,
    P5,
    P6,
    P7,
    P8,
    P9,
    P10,
    PJ,
    PQ,
    PK,
    PA,
    P2,
    PJOKER0,
    PJOKER1
} POKER_UNIT;

// the color of poker
typedef enum POKER_CLR_t {
    SPADE,
    HEART,
    CLUB,
    DIAMOND,
    JOKER0,
    JOKER1
} POKER_CLR;

// the poker card element
typedef struct POKER_t {
    POKER_UNIT      unit;
    POKER_CLR       color;
    int             value;
} POKER;


// ���Ƶ�����
typedef enum {
    INVALID_TYPE = 0,
    SINGLE,                 // ����
    SERIES,                 // ˳��
    PAIR,                   // ����
    SERIES_PAIR,            // ����
    TRIANGLE,               // ����
    SERIES_TRIANGLE,        // ������
    THREE_PLUS_ONE,         // ����һ
    SERIES_THREE_PLUS_ONE,  // ������һ
    THREE_PLUS_TWO,         // ������
    SERIES_THREE_PLUS_TWO,  // ��������
    SERIES_FOUR,            // ���ģ�����ը�����ˣ�
    FOUR_PLUS_TWO,          // �Ĵ���
    SERIES_FOUR_PLUS_TWO,   // ���Ĵ���
    FOUR_PLUS_FOUR,         // �Ĵ���
    SERIES_FOUR_PLUS_FOUR,  // ���Ĵ���
    BOMB,                   // ը��
} POKER_TYPE;

// ���Ƶ���ϸ���
typedef struct POKER_PROPERTY_s {
    POKER_TYPE      type;       // ���Ƶ�����
    int             value;      // ���Ƶ�ֵ
    int             num;        // ���Ƹ���
} POKER_PROPERTY;

// ����
typedef struct HT_SINGLE_t {
    int     idx;
    int     value;
} HT_SINGLE;

// ����
typedef struct HT_PAIR_t {
    int     idx0;
    int     idx1;
    int     value;
} HT_PAIR;

// ����
typedef struct HT_TRIANGLE_t {
    int     idx0;
    int     idx1;
    int     idx2;
    int     value;
} HT_TRIANGLE;

// ����
typedef struct HT_FOUR_t {
    int     idx0;
    int     idx1;
    int     idx2;
    int     idx3;
    int     value;
} HT_FOUR;

// ˳��
typedef struct HT_SERIES_t {
    int     idx[12];
    int     num;
    int     value;
} HT_SERIES;

// ����
typedef struct HT_SERIES_PAIR_t {
    int     idx[20];
    int     num;
    int     value;
} HT_SERIES_PAIR;

// ����
typedef struct HT_SERIES_TRIANGLE_t {
    int     idx[18];
    int     num;
    int     value;
} HT_SERIES_TRIANGLE;

// ����
typedef struct HT_SERIES_FOUR_t {
    int     idx[20];
    int     num;
    int     value;
} HT_SERIES_FOUR;

// ˫��ը��
typedef struct HT_KINGBOMB_t {
    int     idx0;
    int     idx1;
    int     value;
} HT_KINGBOMB;

// �˿��Ʒ����
typedef struct POKER_CLASS_TABLE_t {
    int             builded;        // whether the PCT has been builded
    int             count;          // current poker cards num

    HT_SINGLE       one[14];        // single (13 singles at most)
    int             num1;           // [single] COUNT

    HT_PAIR         two[10];        // pair (10 pairs at most)
    int             num2;           // [pair] COUNT

    HT_TRIANGLE     three[6];       // triangle (6 triangles at most)
    int             num3;           // [triangle] COUNT

    HT_FOUR         four[5];        // four (5 fours at most)
    int             num4;           // [four] COUNT

    HT_KINGBOMB     kingbomb;       // king bomb
    int             has_king_bomb;  // indicate whether exists king bomb

    HT_SERIES       sone[2];        // series one (2 separate series at most)
    int             num11;          // [series one] COUNT

    HT_SERIES_PAIR      stwo[3];    // series two ( 3 separate series two at most)
    int                 num22;      // [series two] COUNT

    HT_SERIES_TRIANGLE  sthree[3];  // series three ( 3 separate series three at most)
    int                 num33;      // [series three] COUNT

    HT_SERIES_FOUR      sfour[2];   // series four ( 2 separate series four at most)
    int                 num44;      // [series four] COUNT
} POKER_CLASS_TABLE;

// ��ȡ��ʾ�˿��Ƶ��ַ�������DEBUG�������CONSOLE���������
POKER_API 
TCHAR poker_unit_to_char(POKER_UNIT unit);

// ��ȡ��ʾ�˿��ƻ�ɫ���ַ�������DEBUG�������CONSOLE���������
POKER_API 
TCHAR poker_clr_to_char(POKER_CLR clr);

// ���˿���������ȡ��ʾ�˿��Ƶ��ַ�������DEBUG�������CONSOLE���������
POKER_API 
TCHAR poker_index_to_char(int index);

// ���˿���������ȡ�仨ɫ���ַ�������DEBUG�������CONSOLE���������
POKER_API 
TCHAR poker_index_to_clr_char(int index);

// ���˿���������ȡ�˿��Ƶ�ֵ������DEBUG�������CONSOLE���������
POKER_API 
int poker_index_to_value(int index);

// ��ȡ�˿������Ͷ�Ӧ���ַ���������DEBUG�������CONSOLE���������
POKER_API 
TCHAR* poker_type_to_string(POKER_TYPE pt);

//
// Function:
//      can_play_poker
// Description:
//      ���Ҫ�������Ƿ�Ϸ�������Ϸ������Ƶ�������Ա�����pp���������״γ��ƣ�
// Parameter:
//      pp  - ���ڱ����˿������е�����
//      vec - �˿��Ƶ��������飬��Ԫ��Ϊָ��POKER��������ȡֵ��ΧΪ0��53
//      num - ��������ĳ���
// Return:
//      �����ɹ�����true�����򷵻�false.
// Remark:
//      ���������жϸú����ķ���ֵ��������false����ʾ�޷�ʶ���Ƶ����ԣ���������
//      Ϊ���Ϸ����С�
//      ����Ĳ��� vec Ϊȫ���˿��Ƶ�������0��53������������ҵ�ǰ�Ƶ�������
//
POKER_API 
bool can_play_poker(POKER_PROPERTY* pp, int vec[], int num);

//
// Function:
//      can_follow_poker
// Description:
//      �������󣬼��Ҫ�������Ƿ�Ϸ�������Ϸ������Ƶ����Ա�����pp�������ڽ��ƣ�
// Parameter:
//      pp  - ���ڱ����˿������е�����
//      vec - �˿��Ƶ��������飬��Ԫ��Ϊָ��POKER��������ȡֵ��ΧΪ0��53
//      num - ��������ĳ���
//      req - ������Ƶ�����
// Return:
//      �����ɹ�����true�����򷵻�false.
// Remark:
//      ���������жϸú����ķ���ֵ��������false����ʾ�޷�ʶ���Ƶ����ԣ���������
//      Ϊ���Ϸ����С�
//      ����Ĳ��� vec Ϊȫ���˿��Ƶ�������0��53������������ҵ�ǰ�Ƶ�������
//
POKER_API 
bool can_follow_poker(POKER_PROPERTY* pp, int vec[], int num, POKER_PROPERTY* req);

//
// Function:
//      build_poker_class_table
// Description:
//      �����˿��Ʒ����
// Parameter:
//      pct     - [out] �����˿��Ʒ�������Ϣ
//      vec     - ��ҵ�ǰ���е��˿������У���Ԫ��Ϊȫ���˿���������0��53����
//      num     - ��ҵ�ǰ���е��˿�������
// Return:
//      Always be TRUE
//
POKER_API 
bool build_poker_class_table(POKER_CLASS_TABLE* pct, int vec[], int num);

//
// Function:
//      reset_poker_class_table
// Description:
//      ��λ�˿��Ʒ����
// Parameter:
//      pct - �˿��Ʒ����
// Return:
//      Always be TRUE
//
POKER_API 
bool reset_poker_class_table(POKER_CLASS_TABLE* pct);

//
// Function:
//      get_poker_hint
//
// Description:
//      �����ϼ��������ƣ���������е�ǰ���У���ʾ���Գ����ơ�
//
// Parameter:
//      times   - ��ʾ�ڼ���������ʾ
//      pct     - �Ѿ������õ��˿��Ʒ����
//      req     - ������Ƶ����ԣ����ϼ���������Ƶ�����
//      out     - [out]���ڷ��ز��ҵ�����ʾ�Ƶ�����
//      out_vec - [out]���ڷ��ز��ҵ�����ʾ�Ƶ��������飬�䳤�ȿ��ɷ��ص����Ի�á�
//
// Return:
//      TRUE if get a proper hint, else FALSE.
//
// Remark:
//      Finding PROPER pokers from [pct] according to the [req] poker property,
//      if operation succeeds, [out] poker property will contain info about the
//      pokers which were found, and the out_vec[] for corresponding indexes.
//      The [times] parameter indicates how many times the caller had queried,
//      0 means first call.
// 
// ATTENTION:
//      ����Ĳ���vec���˿��Ƶ�ȫ���������飬����ȡֵ��ΧΪ��0��53�ݣ�
//      ���ص���������out_vec����Ԫ��Ϊ��ҵ�ǰ�Ƶ�����������ȡֵ��ΧΪ��0��19�ݣ�
//      ������ʾʱ�����ȹ�����˿��Ʒ�������˿��������������仯������Ҫ���¹����˿˷����
//
POKER_API 
bool get_poker_hint(
    int times,
    POKER_CLASS_TABLE* pct,
    POKER_PROPERTY* req,
    POKER_PROPERTY* out,
    int out_vec[]
);

