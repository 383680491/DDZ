//
// pokertype.cpp : ���Ƶ�����
//
// �����������Ƶ�����ʱ���������Ʊ����ǰ��������кõġ�
//
// ISSUE:
//      ����ϼҳ���Ϊ��333444555JQK�����¼ҳ���Ϊ��666777888999��Ϊ�Ϸ�����;
//      �������״γ���Ϊ��666777888999������Ϊ������������������һ��
//      Ϊ���������⣬�״γ�������Ʒ���������ʵ�֡�
//
// Remark:
//      ը�������ԭ��
//      ���磬�ϼҳ���Ϊ��3334447799�����¼ҳ��ơ�JJJJQQQKKK��Ϊ���Ϸ����ƣ�
//      ���⣬����ϼҳ���Ϊ��33344468�����¼ҳ��ơ�77778889��ҲΪ���Ϸ����ơ�
//
//      ������ը�������ƶ��������������������Ĵ��ģ�����Ϊ���Ϸ��ĳ��ƣ�
//      ��˫��ը�����Զ�ͬʱ�������������͵ĸ�����
//
#include "stdafx.h"
#include "error.h"
#include "poker.h"
#include "pokertype.h"
#include "PokerLib.h"


extern POKER poker[];

//
// used for diagnostics
//
POKER_API 
TCHAR* poker_type_to_string(POKER_TYPE pt)
{
    switch (pt) {
    case SINGLE:
        return _T("SINGLE");

    case SERIES:
        return _T("SERIES");

    case PAIR:
        return _T("PAIR");

    case SERIES_PAIR:
        return _T("SERIES_PAIR");

    case TRIANGLE:
        return _T("TRIANGLE");

    case SERIES_TRIANGLE:
        return _T("SERIES_TRIANGLE");

    case THREE_PLUS_ONE:
        return _T("THREE_PLUS_ONE");

    case SERIES_THREE_PLUS_ONE:
        return _T("SERIES_THREE_PLUS_ONE");

    case THREE_PLUS_TWO:
        return _T("THREE_PLUS_TWO");

    case SERIES_THREE_PLUS_TWO:
        return _T("SERIES_THREE_PLUS_TWO");

    case SERIES_FOUR:
        return _T("SERIES_FOUR");

    case FOUR_PLUS_TWO:
        return _T("FOUR_PLUS_TWO");

    case SERIES_FOUR_PLUS_TWO:
        return _T("SERIES_FOUR_PLUS_TWO");

    case FOUR_PLUS_FOUR:
        return _T("FOUR_PLUS_FOUR");

    case SERIES_FOUR_PLUS_FOUR:
        return _T("SERIES_FOUR_PLUS_FOUR");

    case BOMB:
        return _T("BOMB");
    }

    return _T("INVALID_POKER_TYPE");
}

// ����
bool is_single(int vec[], int num, int* val)
{
    if (num != 1) { return false; }

    *val = poker[vec[0]].value;

    return true;
}

// ����
bool is_pair(int vec[], int num, int* val)
{
    if (num != 2) { return false; }

    if (poker[vec[0]].value == poker[vec[1]].value) {
        *val = poker[vec[1]].value;
        return true;
    }

    return false;
}

// ˫��ը��
bool is_king_bomb(int vec[], int num, int* val)
{
    if (num != 2) { return false; }

    if ((poker[vec[0]].unit == PJOKER0)
        && (poker[vec[1]].unit == PJOKER1)) {
            *val = poker[vec[1]].value;
            return true;
    }

    return false;
}

// ����
bool is_triangle(int vec[], int num, int* val)
{
    if (num != 3) { return false; }

    if ((poker[vec[0]].value == poker[vec[1]].value)
        && (poker[vec[0]].value == poker[vec[2]].value)) {
            *val = poker[vec[0]].value;
            return true;
    }

    return false;
}

// ը��
bool is_four_bomb(int vec[], int num, int* val)
{
    if (num != 4) { return false; }

    if ((poker[vec[0]].value == poker[vec[1]].value)
        && (poker[vec[0]].value == poker[vec[2]].value)
        && (poker[vec[0]].value == poker[vec[3]].value)) {
            *val = poker[vec[0]].value;
            return true;
    }

    return false;
}

// ����һ
bool is_three_plus_one(int vec[], int num, int* val)
{
    if (num != 4) { return false; }

    // "333A"
    if ((poker[vec[0]].value == poker[vec[1]].value)
        && (poker[vec[0]].value == poker[vec[2]].value)
        && (poker[vec[0]].value != poker[vec[3]].value)) {
            *val = poker[vec[0]].value;
            return true;
    }

    // "3AAA"
    if ((poker[vec[0]].value != poker[vec[1]].value)
        && (poker[vec[1]].value == poker[vec[2]].value)
        && (poker[vec[1]].value == poker[vec[3]].value)) {
            *val = poker[vec[1]].value;
            return true;
    }

    return false;
}

// ������
bool is_three_plus_two(int vec[], int num, int* val)
{
    if (num != 5) { return false; }

    // "33344"
    if ((poker[vec[0]].value == poker[vec[1]].value)
        && (poker[vec[0]].value == poker[vec[2]].value)
        && (poker[vec[3]].value == poker[vec[4]].value)) {
            *val = poker[vec[0]].value;
            return true;
    }

    // "33444"
    if ((poker[vec[0]].value == poker[vec[1]].value)
        && (poker[vec[2]].value == poker[vec[3]].value)
        && (poker[vec[2]].value == poker[vec[4]].value)) {
            *val = poker[vec[2]].value;
            return true;
    }

    return false;
}

// �Ĵ���
bool is_four_plus_two(int vec[], int num, int* val)
{
    if (num != 6) { return false; }

#if 0
    for (int i = 0; i < num - 3; i++) {
        if ((poker[vec[i]].value == poker[vec[i + 1]].value)
            && (poker[vec[i]].value == poker[vec[i + 2]].value)
            && (poker[vec[i]].value == poker[vec[i + 3]].value)) {
                *val = poker[vec[i]].value;
                return true;
        }
    }
#else
    // "333345"
    if ((poker[vec[0]].value == poker[vec[1]].value)
        && (poker[vec[0]].value == poker[vec[2]].value)
        && (poker[vec[0]].value == poker[vec[3]].value)) {
            *val = poker[vec[0]].value;
            return true;
    }

    // "344445"
    if ((poker[vec[1]].value == poker[vec[2]].value)
        && (poker[vec[1]].value == poker[vec[3]].value)
        && (poker[vec[1]].value == poker[vec[4]].value)) {
            *val = poker[vec[1]].value;
            return true;
    }

    // "345555"
    if ((poker[vec[2]].value == poker[vec[3]].value)
        && (poker[vec[2]].value == poker[vec[4]].value)
        && (poker[vec[2]].value == poker[vec[5]].value)) {
            *val = poker[vec[2]].value;
            return true;
    }
#endif

    return false;
}

// �Ĵ���
bool is_four_plus_four(int vec[], int num, int* val)
{
    if (num != 8) { return false; }

    // "33334455"
    if ((poker[vec[0]].value == poker[vec[1]].value)
        && (poker[vec[0]].value == poker[vec[2]].value)
        && (poker[vec[0]].value == poker[vec[3]].value)
        && (poker[vec[4]].value == poker[vec[5]].value)
        && (poker[vec[4]].value != poker[vec[6]].value)
        && (poker[vec[6]].value == poker[vec[7]].value)) {
            *val = poker[vec[0]].value;
            return true;
    }

    // "33444455"
    if ((poker[vec[0]].value == poker[vec[1]].value)
        && (poker[vec[2]].value == poker[vec[3]].value)
        && (poker[vec[2]].value == poker[vec[4]].value)
        && (poker[vec[2]].value == poker[vec[5]].value)
        && (poker[vec[6]].value == poker[vec[7]].value)) {
            *val = poker[vec[2]].value;
            return true;
    }

    // "33445555"
    if ((poker[vec[0]].value == poker[vec[1]].value)
        && (poker[vec[0]].value != poker[vec[2]].value)
        && (poker[vec[2]].value == poker[vec[3]].value)
        && (poker[vec[4]].value == poker[vec[5]].value)
        && (poker[vec[4]].value == poker[vec[6]].value)
        && (poker[vec[4]].value == poker[vec[7]].value)) {
            *val = poker[vec[4]].value;
            return true;
    }

    return false;
}

// ˳�ӣ����ӣ�
bool is_series(int vec[], int num, int* val)
{
    int i;

    if ((num < 5) || (num > 12)) { return false; }

    // ��������Ʋ���Ϊ��2,����
    if ((poker[vec[num - 1]].unit == P2)
        || (poker[vec[num - 1]].unit == PJOKER0)
        || (poker[vec[num - 1]].unit == PJOKER1)) {
            return false;
    }

    for (i = num - 1; i > 0; i--) {
        if (poker[vec[i]].value - poker[vec[i - 1]].value != 1) {
            return false;
        }
    }

    *val = poker[vec[num - 1]].value;
    return true;
}

// ����
bool is_series_pair(int vec[], int num, int* val)
{
    int i;

    if (num % 2 != 0) { return false; }
    if ((num < 6) || (num > 20)) { return false; }

    // ��������Ʋ���Ϊ��2,����
    if ((poker[vec[num - 1]].unit == P2)
        || (poker[vec[num - 1]].unit == PJOKER0)
        || (poker[vec[num - 1]].unit == PJOKER1)) {
            return false;
    }

    for (i = 0; i < num - 1; i += 2) {
        if (poker[vec[i]].value != poker[vec[i + 1]].value) {
            return false;
        }
    }

    for (i = num - 1; i > 2; i -= 2) {
        if (poker[vec[i]].value - poker[vec[i - 2]].value != 1) {
            return false;
        }
    }

    *val = poker[vec[num - 1]].value;
    return true;
}

// ����
bool is_series_triangle(int vec[], int num, int* val)
{
    int i;

    if (num % 3 != 0) { return false; }
    if ((num < 6) || (num > 18)) { return false; }

    // ��������Ʋ���Ϊ��2,����
    if ((poker[vec[num - 1]].unit == P2)
        || (poker[vec[num - 1]].unit == PJOKER0)
        || (poker[vec[num - 1]].unit == PJOKER1)) {
            return false;
    }

    for (i = 0; i < num - 2; i += 3) {
        if ((poker[vec[i]].value != poker[vec[i + 1]].value)
            || (poker[vec[i]].value != poker[vec[i + 2]].value)) {
                return false;
        }
    }

    for (i = num - 1; i > 3; i -= 3) {
        if (poker[vec[i]].value - poker[vec[i - 3]].value != 1) {
            return false;
        }
    }

    *val = poker[vec[num - 1]].value;
    return true;
}

// ����
bool is_series_four(int vec[], int num, int* val)
{
    int i;

    if (num % 4 != 0) { return false; }
    if ((num < 8) || (num > 20)) { return false; };

    // ��������Ʋ���Ϊ��2,����
    if ((poker[vec[num - 1]].unit == P2)
        || (poker[vec[num - 1]].unit == PJOKER0)
        || (poker[vec[num - 1]].unit == PJOKER1)) {
            return false;
    }

    for (i = 0; i < num - 3; i += 4) {
        if ((poker[vec[i]].value != poker[vec[i + 1]].value)
            || (poker[vec[i]].value != poker[vec[i + 2]].value)
            || (poker[vec[i]].value != poker[vec[i + 3]].value)) {
                return false;
        }
    }

    for (i = num - 1; i > 4; i -= 4) {
        if (poker[vec[i]].value - poker[vec[i - 4]].value != 1) {
            return false;
        }
    }

    *val = poker[vec[num - 1]].value;
    return true;
}

// ������һ
bool is_series_three_plus_one(int vec[], int num, int* val)
{
    int i, n, v;

    if (num % 4 != 0) { return false; }
    if ((num < 8) || (num > 20)) { return false; }

    n = num / 4; // �����������һ������n�������ţ���˵����n������ (1<n<6)

    // ���㷨���ӡ�3333444555666777������3344445555666777������3334445556667777��
    // Ϊ���Ϸ��ĳ��ơ�����ѭը�������ԭ��

    // �Ӹߵ��Ͳ�����n��������һ���硰3344455566677788������Ϊ��Ч��������һ��
    // ��ֵΪ7

    for (i = 0; i < num - 3; i++) {
        if ((poker[vec[i]].value == poker[vec[i + 1]].value)
            && (poker[vec[i]].value == poker[vec[i + 2]].value)
            && (poker[vec[i]].value == poker[vec[i + 3]].value)) {
                return false;
        }
    }

    for (i = n; i >= 0; i--) {
        if (is_series_triangle(vec + i, n * 3, &v)) {
            *val = v;
            return true;
        }
    }

    return false;
}

// ��������
bool is_series_three_plus_two(int vec[], int num, int* val)
{
    int i, j, n, v;

    if (num % 5 != 0) { return false; }
    if ((num < 10) || (num > 20)) { return false; }

    n = num / 5; // �������������������n�������ţ���˵����n������ (1<n<5)

    // �ڴ��㷨�У����ӡ�333344455566677���͡�334445556667777��Ϊ���Ϸ����ơ�
    // ����ѭը�������ԭ��

    // �Ӹߵ��Ͳ�����n�����������������������������ж��������Ƿ��Ƕ��ӡ�
    // �硰3344555666777888AA22������Ϊ��Ч��������������ֵΪ7

    for (i = 0; i < num - 3; i++) {
        if ((poker[vec[i]].value == poker[vec[i + 1]].value)
            && (poker[vec[i]].value == poker[vec[i + 2]].value)
            && (poker[vec[i]].value == poker[vec[i + 3]].value)) {
                return false;
        }
    }

    for (i = n * 2; i >= 0; i -= 2) {
        if (is_series_triangle(vec + i, n * 3, &v)) {
            for (j = 0; j < i - 1; j += 2) {
                if (poker[vec[j]].value != poker[vec[j + 1]].value) {
                    return false;
                }
            }
            
            for (j = i + n * 3; j < num - 1; j += 2) {
                if (poker[vec[j]].value != poker[vec[j + 1]].value) {
                    return false;
                }
            }

            *val = v;
            return true;
        }
    }

    return false;
}

// ���Ĵ���
bool is_series_four_plus_two(int vec[], int num, int* val)
{
    int i, j, n, v;

    if (num % 6 != 0) { return false; }
    if ((num < 12) || (num > 18)) { return false; }

    n = num / 6;

    // ����ѭը�������ԭ��
    // �Ӹ����Ͳ������ģ�����ҵ����ģ����ж���ͷ��β�Ƿ����ը����
    // �硰4444666677778888JQ������34777788889999AAAA�����ǺϷ������Ĵ�����
    // ����4445666677778888JQ������34777788889999AAA2���ǺϷ������Ĵ�����

    for (i = n * 2; i >= 0; i--) {
        if (is_series_four(vec + i, n * 4, &v)) {
            for (j = 0; j < i - 3; j++) {
                if ((poker[vec[j]].value == poker[vec[j + 1]].value)
                    && (poker[vec[j]].value == poker[vec[j + 2]].value)
                    && (poker[vec[j]].value == poker[vec[j + 3]].value)) {
                        return false;
                }
            }

            for (j = i + n * 4; j < num - 3; j++) {
                if ((poker[vec[j]].value == poker[vec[j + 1]].value)
                    && (poker[vec[j]].value == poker[vec[j + 2]].value)
                    && (poker[vec[j]].value == poker[vec[j + 3]].value)) {
                        return false;
                }
            }

            *val = v;
            return true;
        }
    }

    return false;
}

// ���Ĵ���
bool is_series_four_plus_four(int vec[], int num, int* val)
{
    int i, j, n, v;

    if (num != 16) { return false; }

    n = 2;

    // ��������Ĵ��ģ�ֻ����16���ƣ���ֻ�����������ġ�

    // ����ѭը�������ԭ��
    // �ȴӸ����Ͳ��ҵ����ģ����ж���ͷ��β�Ƿ����ը��������ж������ƶ��ɶ��ӡ�
    // �硰444466667777JJQQ������334477778888AAAA�����ǺϷ������Ĵ��ġ�

    for (i = n * 4; i >= 0; i -= 2) {
        if (is_series_four(vec + i, n * 4, &v)) {
            for (j = 0; j < i - 3; j++) {
                if ((poker[vec[j]].value == poker[vec[j + 1]].value)
                    && (poker[vec[j]].value == poker[vec[j + 2]].value)
                    && (poker[vec[j]].value == poker[vec[j + 3]].value)) {
                        return false;
                }
            }

            for (j = i + n * 4; j < num - 3; j++) {
                if ((poker[vec[j]].value == poker[vec[j + 1]].value)
                    && (poker[vec[j]].value == poker[vec[j + 2]].value)
                    && (poker[vec[j]].value == poker[vec[j + 3]].value)) {
                        return false;
                }
            }

            for (j = 0; j < i - 1; j += 2) {
                if (poker[vec[j]].value != poker[vec[j + 1]].value) {
                    return false;
                }
            }

            for (j = i + n * 4; j < num - 1; j += 2) {
                if (poker[vec[j]].value != poker[vec[j + 1]].value) {
                    return false;
                }
            }

            *val = v;
            return true;
        }
    }

    return false;
}

//
// Function:
//      get_poker_property
// Description:
//      ��ȡ���Ƶ��������
// Parameter:
//      pp  - [out] poker property if succeed
//      vec - poker vector to be analyzed. The element are indexes to GLOBAL poker.
//      num - poker num to be analyzed
// Return:
//      E_NONE if everything is OK, else the pokers are not valid.
// Remark:
//      the poker vector in array vec should be sorted in ascend.
//
POKER_RET get_poker_property(POKER_PROPERTY* pp, int vec[], int num)
{
    int value;

    assert(pp != NULL);

    if (is_single(vec, num, &value)) {
        pp->type = SINGLE;
        goto match_succeed;
    } else if (is_pair(vec, num, &value)) {
        pp->type = PAIR;
        goto match_succeed;
    } else if (is_king_bomb(vec, num, &value)) {
        pp->type = BOMB;
        goto match_succeed;
    } else if (is_triangle(vec, num, &value)) {
        pp->type = TRIANGLE;
        goto match_succeed;
    } else if (is_four_bomb(vec, num, &value)) {
        pp->type = BOMB;
        goto match_succeed;
    } else if (is_three_plus_one(vec, num, &value)) {
        pp->type = THREE_PLUS_ONE;
        goto match_succeed;
    } else if (is_three_plus_two(vec, num, &value)) {
        pp->type = THREE_PLUS_TWO;
        goto match_succeed;
    } else if (is_four_plus_two(vec, num, &value)) {
        pp->type = FOUR_PLUS_TWO;
        goto match_succeed;
    } else if (is_four_plus_four(vec, num, &value)) {
        pp->type = FOUR_PLUS_FOUR;
        goto match_succeed;
    } else if (is_series(vec, num, &value)) {
        pp->type = SERIES;
        goto match_succeed;
    } else if (is_series_pair(vec, num, &value)) {
        pp->type = SERIES_PAIR;
        goto match_succeed;
    } else if (is_series_triangle(vec, num, &value)) {
        pp->type = SERIES_TRIANGLE;
        goto match_succeed;
    } else if (is_series_three_plus_one(vec, num, &value)) {
        pp->type = SERIES_THREE_PLUS_ONE;
        goto match_succeed;
    } else if (is_series_three_plus_two(vec, num, &value)) {
        pp->type = SERIES_THREE_PLUS_TWO;
        goto match_succeed;
    } else if (is_series_four(vec, num, &value)) {
        pp->type = SERIES_FOUR;
        goto match_succeed;
    } else if (is_series_four_plus_two(vec, num, &value)) {
        pp->type = SERIES_FOUR_PLUS_TWO;
        goto match_succeed;
    } else if (is_series_four_plus_four(vec, num, &value)) {
        pp->type = SERIES_FOUR_PLUS_FOUR;
        goto match_succeed;
    } else {
        return E_INVALID;
    }

match_succeed:
    pp->value = value;
    pp->num = num;

    return E_NONE;
}

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
//      ���������жϸú����ķ���ֵ��������false����ʾ�޷��ж��Ƶ����ԣ�������
//      Ϊ���Ϸ����С�
//      ����Ĳ��� vec Ϊȫ���˿��Ƶ���������������ҵ�ǰ�Ƶ����������򣬵���
//      �ú���ǰ��Ҫ����ת����
//
POKER_API 
bool can_play_poker(POKER_PROPERTY* pp, int vec[], int num)
{
    assert(pp != NULL);

    return (E_NONE == get_poker_property(pp, vec, num)) ? true : false;
}

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
//      ���������жϸú����ķ���ֵ��������false����ʾ�޷��ж��Ƶ����ԣ�������
//      Ϊ���Ϸ����С�
//      ����Ĳ��� vec Ϊȫ���˿��Ƶ���������������ҵ�ǰ�Ƶ����������򣬵���
//      �ú���ǰ��Ҫ����ת����
//
POKER_API 
bool can_follow_poker(POKER_PROPERTY* pp, int vec[], int num, POKER_PROPERTY* req)
{
    int value;

    assert((pp != NULL) && (req != NULL));

    // ��ǰҪ��������˫��ը��
    if (is_king_bomb(vec, num, &value)) {
        pp->type = BOMB;
        goto can_follow;
    }

    // ��ǰҪ��������ը��
    if (is_four_bomb(vec, num, &value)) {
        if ((req->type != BOMB) || ((req->type == BOMB) && (value > req->value))) {
            pp->type = BOMB;
            goto can_follow;
        } else {
            return false;
        }
    }

    // ��ǰҪ�����Ʋ���ը�������ϼҵ�����ը��
    if (req->type == BOMB) {
        return false;
    }
    
    // �ϼҵ��Ʋ���ը������ǰҪ�������Ҳ����ը��
    if (num != req->num) {
        return false;
    }

    //
    // �����˵���ϼҳ������뵱ǰҪ�����ƶ�����ը��������������ȣ���ˣ�
    // ֻ��Ҫ�Ƚ����ǵ����ͺ�ֵ���ɡ�
    //
    switch (req->type) {
        case SINGLE:
            if (is_single(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case PAIR:
            if (is_pair(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case TRIANGLE:
            if (is_triangle(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case THREE_PLUS_ONE:
            if (is_three_plus_one(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case THREE_PLUS_TWO:
            if (is_three_plus_two(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case FOUR_PLUS_TWO:
            if (is_four_plus_two(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case FOUR_PLUS_FOUR:
            if (is_four_plus_four(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case SERIES:
            if (is_series(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case SERIES_PAIR:
            if (is_series_pair(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case SERIES_TRIANGLE:
            if (is_series_triangle(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case SERIES_THREE_PLUS_ONE:
            if (is_series_three_plus_one(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case SERIES_THREE_PLUS_TWO:
            if (is_series_three_plus_two(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case SERIES_FOUR:
            if (is_series_four(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case SERIES_FOUR_PLUS_TWO:
            if (is_series_four_plus_two(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        case SERIES_FOUR_PLUS_FOUR:
            if (is_series_four_plus_four(vec, num, &value)) {
                goto type_matched;
            } else {
                return false;
            }

        default:
            return false;
    }

type_matched:
    if (value <= req->value) {
        return false;
    }

    pp->type = req->type;

can_follow:
    pp->value = value;
    pp->num = num;

    return true;
}