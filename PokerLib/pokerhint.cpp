//
// pokerhint.cpp
//
//  �����ϼ��������ƣ���ʾ��ҿ�����ʲô��ȥ�ӡ�
//
//  ���˼�룺
//      ��������ϵ��ƽ��з��࣬���ηֳ����š����ӡ����š����š�˳�ӡ����ԡ�
//      ���������ĵ����Ȼ������ϼ��������Ƶ����ͣ��������Ӧ����ʾ���͡�
//
//      ���ݽṹʹ�����飬��ȽϺķѿռ䣬��������Щ��
//      
//      ������а�������������ֵ�����˿��Ƶ�ֵ���������������ȫ�ֵ��˿�����������
//      ��ҵ�ǰ�����˿��Ƶ�����������ȡֵ��ΧΪ[0��19]��
//      
//      1��
//      ���ҳ�����ʾʱ����Ҫ�ȹ����˿��Ʒ��������ҳ��ƺ��Ƶ����������仯��
//      ���ԣ��´β�����ʾʱ����Ҫ���¹����˿˷����
//
//      2��
//      �����������ͣ��ȴ�ƥ�������в��ң����ѡ�����Թ����������ƥ������ͣ�
//      �����ʾ�Ƿ���ը�������磬�ϼҳ���Ϊ���š�3������������е���Ϊ��4456����
//      ��������ʾ��5���������ʾ��6���������ʾ��4����
//      
//      3,
//      ���ܴ��ڶ����ʾ��������Щ��ʾ���������ȼ�Ӧ����ѭ���ġ����磬��ҵ�һ��
//      ��ʾ��ť���򷵻ص�һ����ʾ���ٵ�һ����ʾ��ť���򷵻���һ����ʾ�����ֱ��
//      ���һ����ʾ���������ٵ�һ����ʾ��ť���򷵻ص�һ����ʾ��
//
//      ��ǰ�ĳ�������У����ܺܿ�ݵ�ʵ�ָù��ܡ�
//      Ϊʵ��������ܣ����ڲ�����ʾʱ������һ���ۼӵļ����������ò����ɵ����߿��ơ�
//
//      ���磬�������Ϊ1������ҵ���һ����ʾʱ�������أ���Ҫ�ȵ����ҵ��ڶ�����ʾʱ
//      �ŷ��أ����߷��� false ��ʾû�и������ʾ��
//
//      �Ե����ߵ�Ҫ�����������ʾʱ����Ϊ false��������Ӧ���һ�´���ļ�������
//      �Ƿ�Ϊ 0����0������ʾ�״ν��в��ң���������� 0������Ҫ�ü�������Ϊ 0������
//      ���ò��Һ������Խ�����ʾ�Ĳ��ҡ��������ʵ��ѭ����ʾ���ܡ����ַ�����Ȼ����
//      ���ŵģ�������һЩ����Ĳ��ҹ�����
//
//      ��һ������㷨�����ݽṹ�����ܻ�Ľ�����Ч�ʣ�����������Ϸ�е������������࣬
//      Ŀǰ����ʵ�֣��������ġ�
//
#include "stdafx.h"
#include "error.h"
#include "mystack.h"
#include "quicksort.h"
#include "poker.h"
#include "pokertype.h"
#include "pokerhint.h"
#include "PokerLib.h"


extern POKER poker[];


// �����Ź���
static void classify_single(POKER_CLASS_TABLE* pct, int vec[], int num)
{
    int i;
    int idx = 0;

    if (num == 1) {
        pct->one[0].idx = 0;
        pct->one[0].value = poker[vec[0]].value;
        pct->num1 = 1;
        return;
    }
 
    for (i = 0; i < num; i++) {
        if (i == 0) { // ��ʼ��
            if (poker[vec[i]].value != poker[vec[i + 1]].value) {
                pct->one[idx].idx = i;
                pct->one[idx].value = poker[vec[i]].value;
                pct->num1++;
                idx++;
            }
        } else if (i == num - 1) { // ĩβ��
            if (poker[vec[i]].value != poker[vec[i - 1]].value) {
                pct->one[idx].idx = i;
                pct->one[idx].value = poker[vec[i]].value;
                pct->num1++;
                idx++;
            }
        } else { // �м�
            if ((poker[vec[i]].value != poker[vec[i - 1]].value)
                && (poker[vec[i]].value != poker[vec[i + 1]].value)) {
                pct->one[idx].idx = i;
                pct->one[idx].value = poker[vec[i]].value;
                pct->num1++;
                idx++;
            }
        }
    }
}

// �����ӹ���
static void classify_pair(POKER_CLASS_TABLE* pct, int vec[], int num)
{
    int i;
    int idx = 0;

    if (num < 2) { return; }

    for (i = 0; i < num - 1; i++) {
        if (poker[vec[i]].value != poker[vec[i + 1]].value) {
            continue;
        }

        // ��ǰ����һ�����

        if (i + 1 == num - 1) { // ĩβ���Ķ���
            pct->two[idx].idx0 = i;
            pct->two[idx].idx1 = i + 1;
            pct->two[idx].value = poker[vec[i]].value;
            pct->num2++;
            idx++;
            break;
        } else if (poker[vec[i]].value != poker[vec[i + 2]].value) { // �м䴦
            pct->two[idx].idx0 = i;
            pct->two[idx].idx1 = i + 1;
            pct->two[idx].value = poker[vec[i]].value;
            pct->num2++;
            idx++;
            i++;
        } else { // TRIANGLE, not pair
            i += 2;
        }
    }
}

// �����Ź���
static void classify_triangle(POKER_CLASS_TABLE* pct, int vec[], int num)
{
    int i;
    int idx = 0;

    if (num < 3) { return; }

    for (i = 0; i < num - 2; i++) {
        if (poker[vec[i]].value != poker[vec[i + 1]].value) {
            continue;
        }

        // ������һ��������������һ���Ƚ�
        if (poker[vec[i]].value != poker[vec[i + 2]].value) {
            i++;
            continue;
        }

        // ������һ��Ҳ���
        if (i + 2 == num - 1) { // ĩβ��������
            pct->three[idx].idx0 = i;
            pct->three[idx].idx1 = i + 1;
            pct->three[idx].idx2 = i + 2;
            pct->three[idx].value = poker[vec[i]].value;
            pct->num3++;
            idx++;
            break;
        } else if (poker[vec[i]].value != poker[vec[i + 3]].value) { // �м䴦
            pct->three[idx].idx0 = i;
            pct->three[idx].idx1 = i + 1;
            pct->three[idx].idx2 = i + 2;
            pct->three[idx].value = poker[vec[i]].value;
            pct->num3++;
            idx++;
            i += 2;
        } else { // ���ŵ�ը��
            i += 3;
        }
    }
}

// �����Ź���
static void classify_four(POKER_CLASS_TABLE* pct, int vec[], int num)
{
    int i;
    int idx = 0;

    if (num < 4) { return; }

    for (i = 0; i < num - 3; i++) {
        if (poker[vec[i]].value == poker[vec[i + 1]].value) {
            if (poker[vec[i]].value == poker[vec[i + 2]].value) {
                if (poker[vec[i]].value == poker[vec[i + 3]].value) {
                    pct->four[idx].idx0 = i;
                    pct->four[idx].idx1 = i + 1;
                    pct->four[idx].idx2 = i + 2;
                    pct->four[idx].idx3 = i + 3;
                    pct->four[idx].value = poker[vec[i]].value;
                    pct->num4++;
                    idx++;
                    i += 3;
                } else {
                    i += 2;
                }
            } else {
                i += 1;
            }
        } else {
            i += 0;
        }
    }
}

// ��¼˫��ը��
static void classify_kingbom(POKER_CLASS_TABLE* pct, int vec[], int num)
{
    assert(vec != NULL);

    if (num < 2) { return; }
    
    if (poker[vec[num - 2]].unit == PJOKER0) {
        if (poker[vec[num - 1]].unit == PJOKER1) {
            pct->kingbomb.idx0 = num - 2;
            pct->kingbomb.idx1 = num - 1;
            pct->kingbomb.value = poker[vec[num - 1]].value;
            pct->has_king_bomb = 1;
        }
    }
}

// ���ҹ���˳�ӵ���һ��Ԫ��
static bool find_series_next_elem(POKER_CLASS_TABLE* pct, int value, int* index)
{
    int i;

    assert(index != NULL);

    if (value >= 14) { return false; } // A��ֵΪ14���Ѿ�����

    // �ڵ�������в���
    for (i = 0; i < pct->num1; i++) {
        if (pct->one[i].value == value + 1) {
            *index = pct->one[i].idx;
            return true;
        }
    }

    // �ڶ�������в���
    for (i = 0; i < pct->num2; i++) {
        if (pct->two[i].value == value + 1) {
            *index = pct->two[i].idx0;
            return true;
        }
    }

    // ����������в���
    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value == value + 1) {
            *index = pct->three[i].idx0;
            return true;
        }
    }

    // ����������в���
    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value == value + 1) {
            *index = pct->four[i].idx0;
            return true;
        }
    }

    return false;
}

// �������о����ܳ���˳��
static void generate_series(POKER_CLASS_TABLE* pct)
{
    int i, n = 0;
    int index;
    int value = 0;
    int count = 0;
    bool ret = false;

    MyStack<int> s(16);

    if (pct->count < 5) { return; }

    for (value = 2; value <= 14; value++) { // ��3��ʼ���ң�ֱ��AΪֹ
        ret = find_series_next_elem(pct, value, &index);
        if (ret == true) {
            s.Push(index);
            count++;
        } else if ((count > 0) && (count < 5)) {
            s.ClearStack();
            count = 0;
        } else if (count >= 5) {
            assert(count <= 12);
            for (i = count - 1; i >= 0; i--) { // ȷ����ջ��˳��Ϊ����
                pct->sone[n].idx[i] = s.Pop();
            }
            pct->sone[n].value = value;
            pct->sone[n].num = count;
            pct->num11++;
            n++;
            count = 0;
        }        
    }    
}

// ���ҹ�������Ե���һ������
static bool find_series_pair_next_elem(POKER_CLASS_TABLE* pct, int value, int* index0, int* index1)
{
    int i;

    assert((index0 != NULL) && (index1 != NULL));
    if (value >= 14) { return false; } // A��ֵΪ14���Ѿ�����

    // �ڶ�������в���
    for (i = 0; i < pct->num2; i++) {
        if (pct->two[i].value == value + 1) {
            *index0 = pct->two[i].idx0;
            *index1 = pct->two[i].idx1;
            return true;
        }
    }

    // ����������в���
    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value == value + 1) {
            *index0 = pct->three[i].idx0;
            *index1 = pct->three[i].idx1;
            return true;
        }
    }

    // ����������в���
    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value == value + 1) {
            *index0 = pct->four[i].idx0;
            *index1 = pct->four[i].idx1;
            return true;
        }
    }

    return false;
}

// �������о����ܳ�������
static void generate_series_pair(POKER_CLASS_TABLE* pct)
{
    int i;
    int index0, index1;
    int value;
    int n = 0;
    int count = 0;
    bool ret;
    MyStack<int> s(32);

    if (pct->count < 6) { return; }

    for (value = 2; value <= 14; value++) {
        ret = find_series_pair_next_elem(pct, value, &index0, &index1);
        if (ret == true) {
            s.Push(index0);
            s.Push(index1);
            count += 2;
        } else if ((count > 0) && (count < 6)) {
            s.ClearStack();
            count = 0;
        } else if (count >= 6) {
            assert(count <= 20);

            for (i = count - 1; i > 0; i -= 2) {
                pct->stwo[n].idx[i] = s.Pop();
                pct->stwo[n].idx[i - 1] = s.Pop();
            }

            pct->stwo[n].value = value;
            pct->stwo[n].num = count;
            pct->num22++;
            n++;
            count = 0;
        }
    }
}

// �����ܹ������������һ������
static bool find_series_triangle_next_elem(POKER_CLASS_TABLE* pct, int value, int* index0, int* index1, int* index2)
{
    int i;

    assert((index0 != NULL) && (index1 != NULL) && (index2 != NULL));
    if (value >= 14) { return false; } // A��ֵΪ14���Ѿ�����

    // �����ŵ�����в���
    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value == value + 1) {
            *index0 = pct->three[i].idx0;
            *index1 = pct->three[i].idx1;
            *index2 = pct->three[i].idx2;
            return true;
        }
    }

    // �����ŵ�����в���
    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value == value + 1) {
            *index0 = pct->four[i].idx0;
            *index1 = pct->four[i].idx1;
            *index2 = pct->four[i].idx2;
            return true;
        }
    }

    return false;
}

// ���쾡���ܳ�������
static void generate_series_triangle(POKER_CLASS_TABLE* pct)
{
    int i;
    int index0, index1, index2;
    int value;
    int n = 0;
    int count = 0;
    bool ret;
    MyStack<int> s(32);

    if (pct->count < 6) { return; }

    for (value = 2; value <= 14; value++) {
        ret = find_series_triangle_next_elem(pct, value, &index0, &index1, &index2);
        if (ret == true) {
            s.Push(index0);
            s.Push(index1);
            s.Push(index2);
            count += 3;
        } else if ((count > 0) && (count < 6)) {
            s.ClearStack();
            count = 0;
        } else if (count >= 6) {
            assert(count <= 18);

            for (i = count - 1; i > 1; i -= 3) {
                pct->sthree[n].idx[i] = s.Pop();
                pct->sthree[n].idx[i - 1] = s.Pop();
                pct->sthree[n].idx[i - 2] = s.Pop();
            }

            pct->sthree[n].value = value;
            pct->sthree[n].num = count;
            pct->num33++;
            n++;
            count = 0;
        }
    }
}

// �����ܹ�������ĵ���һ������
static bool find_series_four_next_elem(POKER_CLASS_TABLE* pct, int value, int* index0, int* index1, int* index2, int* index3)
{
    int i;

    assert((index0 != NULL) && (index1 != NULL)
        && (index2 != NULL) && (index3 != NULL));

    if (value >= 14) { return false; } // A��ֵΪ14���Ѿ�����

    // ����������в���
    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value == value + 1) {
            *index0 = pct->four[i].idx0;
            *index1 = pct->four[i].idx1;
            *index2 = pct->four[i].idx2;
            *index3 = pct->four[i].idx3;
            return true;
        }
    }

    return false;
}

// ���쾡���ܳ�������
static void generate_series_four(POKER_CLASS_TABLE* pct)
{
    int i;
    int index0, index1, index2, index3;
    int value;
    int n = 0;
    int count = 0;
    bool ret;
    MyStack<int> s(32);

    if (pct->count < 8) { return; }

    for (value = 2; value <= 14; value++) {
        ret = find_series_four_next_elem(pct, value, &index0, &index1, &index2, &index3);
        if (ret == true) {
            s.Push(index0);
            s.Push(index1);
            s.Push(index2);
            s.Push(index3);
            count += 4;
        } else if ((count > 0) && (count < 8)) {
            s.ClearStack();
            count = 0;
        } else if (count >= 8) {
            assert(count <= 20);

            for (i = count - 1; i > 2; i -= 4) {
                pct->sfour[n].idx[i] = s.Pop();
                pct->sfour[n].idx[i - 1] = s.Pop();
                pct->sfour[n].idx[i - 2] = s.Pop();
                pct->sfour[n].idx[i - 3] = s.Pop();
            }

            pct->sfour[n].value = value;
            pct->sfour[n].num = count;
            pct->num44++;
            n++;
            count = 0;
        }
    }
}

// ��λ�˿��Ƶķ����ṹ
POKER_API 
bool reset_poker_class_table(POKER_CLASS_TABLE* pct)
{
    assert(pct != NULL);

    pct->builded = 0;
    pct->count = 0;
    pct->num1 = 0;
    pct->num2 = 0;
    pct->num3 = 0;
    pct->num4 = 0;
    pct->num11 = 0;
    pct->num22 = 0;
    pct->num33 = 0;
    pct->num44 = 0;
    pct->has_king_bomb = 0;

    return true;
}

// �����˿��Ƶķ����
POKER_API 
bool build_poker_class_table(POKER_CLASS_TABLE* pct, int vec[], int num)
{
    assert(pct != NULL);

    if ((num < 0) || (num > 20)) { return false; }

    if (pct->builded == 1) {
        return true;
    }

    pct->count = num;

    classify_single(pct, vec, num);
    classify_pair(pct, vec, num);
    classify_triangle(pct, vec, num);
    classify_four(pct, vec, num);
    classify_kingbom(pct, vec, num);

    generate_series(pct);
    generate_series_pair(pct);
    generate_series_triangle(pct);
    generate_series_four(pct);

    pct->builded = 1;

    return true;
}

// ����һ���ȸ���ֵ��ĵ���
static bool retrieve_valid_single(int times, int val, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i;
    int index;
    int value;
    int count = 0;

    assert((out != NULL) && (vec != NULL));

    // �ڵ�������в���
    for (i = 0; i < pct->num1; i++) {
        if (pct->one[i].value > val) {
            if (count >= times) {
                index = pct->one[i].idx;
                value = pct->one[i].value;
                goto found;
            } else {
                count++;
            }
        }
    }

    // �ڶ�������в���
    for (i = 0; i < pct->num2; i++) {
        if (pct->two[i].value > val) {
            if (count >= times) {
                index = pct->two[i].idx0;
                value = pct->two[i].value;
                goto found;
            } else {
                count++;
            }
        }
    }

    // ����������в���
    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value > val) {
            if (count >= times) {
                index = pct->three[i].idx0;
                value = pct->three[i].value;
                goto found;
            } else {
                count++;
            }
        }
    }

    // ����������в���
    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value > val) {
            if (count >= times) {
                index = pct->four[i].idx0;
                value = pct->four[i].value;
                goto found;
            } else {
                count++;
            }
        }
    }

    // ����ը��
    RETRIEVE_BOMB();
    return false;

found:
    out->type = SINGLE;
    out->value = value;
    out->num = 1;
    vec[0] = index;
    return true;
}

// ����һ���ȸ���ֵ��Ķ���
static bool retrieve_valid_pair(int times, int val, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i;
    int value;
    int index0, index1;
    int count = 0;

    assert((out != NULL) && (vec != NULL));

    // �ڶ�������в���
    for (i = 0; i < pct->num2; i++) {
        if (pct->two[i].value > val) {
            if (count >= times) {
                index0 = pct->two[i].idx0;
                index1 = pct->two[i].idx1;
                value = pct->two[i].value;
                goto found;
            } else {
                count++;
            }
        }
    }

    // ����������в���
    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value > val) {
            if (count >= times) {
                index0 = pct->three[i].idx0;
                index1 = pct->three[i].idx1;
                value = pct->three[i].value;
                goto found;
            } else {
                count++;
            }
        }
    }

    // ����������в���
    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value > val) {
            if (count >= times) {
                index0 = pct->four[i].idx0;
                index1 = pct->four[i].idx1;
                value = pct->four[i].value;
                goto found;
            } else {
                count++;
            }
        }
    }

    // ����ը��
    RETRIEVE_BOMB();
    return false;

found:
    out->type = PAIR;
    out->value = value;
    out->num = 2;
    vec[0] = index0;
    vec[1] = index1;
    return true;
}

// ����һ���ȸ���ֵ�������
static bool retrieve_valid_triangle(int times, int val, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i;
    int value;
    int index0, index1, index2;
    int count = 0;

    assert((out != NULL) && (vec != NULL));

    // ����������в���
    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value > val) {
            if (count >= times) {
                index0 = pct->three[i].idx0;
                index1 = pct->three[i].idx1;
                index2 = pct->three[i].idx2;
                value = pct->three[i].value;
                goto found;
            } else {
                count++;
            }
        }
    }

    // ����������в���
    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value > val) {
            if (count >= times) {
                index0 = pct->four[i].idx0;
                index1 = pct->four[i].idx1;
                index2 = pct->four[i].idx2;
                value = pct->four[i].value;
                goto found;
            } else {
                count++;
            }
        }
    }

    // ����ը��
    RETRIEVE_BOMB();
    return false;

found:
    out->type = TRIANGLE;
    out->value = value;
    out->num = 3;
    vec[0] = index0;
    vec[1] = index1;
    vec[2] = index2;
    return true;
}

// ����һ�������ը��
static bool retrieve_valid_bomb(int times, int val, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i;
    int count = 0;

    assert((out != NULL) && (vec != NULL));
    
    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value > val) {
            if (count >= times) {
                out->type   = BOMB;
                out->num    = 4;
                out->value  = pct->four[i].value;
                vec[0]      = pct->four[i].idx0;
                vec[1]      = pct->four[i].idx1;
                vec[2]      = pct->four[i].idx2;
                vec[3]      = pct->four[i].idx3;

                return true;
            } else {
                count++;
            }
        }
    }

    if (pct->has_king_bomb != 0) {
        if (count >= times) {
            out->type   = BOMB;
            out->num    = 2;
            out->value  = pct->kingbomb.value;
            vec[0]      = pct->kingbomb.idx0;
            vec[1]      = pct->kingbomb.idx1;

            return true;
        } else {
            count++;
        }
    }

    return false;
}

// ����һ���ȸ���ֵ���������һ
static bool retrieve_valid_three_plus_one(int times, int val, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i;
    int count = 0;

    assert((out != NULL) && (vec != NULL));

    //
    // ����һ�����ʵ����ţ��ٲ���һ��ֵ���������ŵ�ֵ�ĵ���
    //

    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value > val) {
            if (count >= times) {
                out->type = THREE_PLUS_ONE;
                out->num = 4;
                out->value = pct->three[i].value;
                vec[0] = pct->three[i].idx0;
                vec[1] = pct->three[i].idx1;
                vec[2] = pct->three[i].idx2;
                goto find_three_ok;
            } else {
                count++;
            }
        }
    }

    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value > val) {
            if (count >= times) {
                out->type = THREE_PLUS_ONE;
                out->num = 4;
                out->value = pct->four[i].value;
                vec[0] = pct->four[i].idx0;
                vec[1] = pct->four[i].idx1;
                vec[2] = pct->four[i].idx2;
                goto find_three_ok;
            } else {
                count++;
            }
        }
    }

    goto retrieve_bomb;

find_three_ok:
    for (i = 0; i < pct->num1; i++) {
        vec[3] = pct->one[i].idx;
        goto find_three_plus_one_ok;
    }

    for (i = 0; i < pct->num2; i++) {
        vec[3] = pct->two[i].idx0;
        goto find_three_plus_one_ok;
    }

    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value != out->value) {
            vec[3] = pct->three[i].idx0;
            goto find_three_plus_one_ok;
        }
    }

    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value != out->value) {
            vec[3] = pct->four[i].idx0;
            goto find_three_plus_one_ok;
        }
    }

    if (count == 1) { count = 0; }

retrieve_bomb:
    RETRIEVE_BOMB();
    return false;

find_three_plus_one_ok:
    quick_sort(vec, 0, 3);
    return true;
}

// ����һ���ȸ���ֵ�����������
static bool retrieve_valid_three_plus_two(int times, int val, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i;
    int count = 0;

    assert((out != NULL) && (vec != NULL));

    //
    // ����һ�����ʵ����ţ��ٲ���һ��ֵ���������ŵ�ֵ�Ķ���
    //

    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value > val) {
            if (count >= times) {
                out->type = THREE_PLUS_TWO;
                out->num = 5;
                out->value = pct->three[i].value;
                vec[0] = pct->three[i].idx0;
                vec[1] = pct->three[i].idx1;
                vec[2] = pct->three[i].idx2;

                goto find_pair;
            } else {
                count++;
            }
        }
    }

    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value > val) {
            if (count >= times) {
                out->type = THREE_PLUS_TWO;
                out->num = 5;
                out->value = pct->four[i].value;
                vec[0] = pct->four[i].idx0;
                vec[1] = pct->four[i].idx1;
                vec[2] = pct->four[i].idx2;

                goto find_pair;
            } else {
                count++;
            }
        }
    }

    goto retrieve_bomb;

find_pair:
     for (i = 0; i < pct->num2; i++) {
        vec[3] = pct->two[i].idx0;
        vec[4] = pct->two[i].idx1;
        goto find_three_plus_two_ok;
    }

    for (i = 0; i < pct->num3; i++) {
        if (pct->three[i].value != out->value) {
            vec[3] = pct->three[i].idx0;
            vec[4] = pct->three[i].idx1;
            goto find_three_plus_two_ok;
        }
    }

    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value != out->value) {
            vec[3] = pct->four[i].idx0;
            vec[4] = pct->four[i].idx1;
            goto find_three_plus_two_ok;
        }
    }

    if (count == 1) { count = 0; }

retrieve_bomb:
    RETRIEVE_BOMB();
    return false;


find_three_plus_two_ok:
    quick_sort(vec, 0, 4);
    return true;
}

// ����һ���ȸ���ֵ����Ĵ���
static bool retrieve_valid_four_plus_two(int times, int val, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i;
    int count = 0;

    assert((out != NULL) && (vec != NULL));

    // ����һ�����ʵ����ţ��ٲ����������ţ����ɣ��������ը��

    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value > val) {
            if (count >= times) {
                out->type = FOUR_PLUS_TWO;
                out->num = 6;
                out->value = pct->four[i].value;
                vec[0] = pct->four[i].idx0;
                vec[1] = pct->four[i].idx1;
                vec[2] = pct->four[i].idx2;
                vec[3] = pct->four[i].idx3;

                goto find_two_singles;
            } else {
                count++;
            }
        }
    }

    goto retrieve_bomb;

find_two_singles:
    //
    // �����������ţ�ok��ʾ���ҵ��ļ���
    //
    int ok = 0;

    for (i = 0; i < pct->num1; i++) {
        if (ok >= 2) { goto find_four_plus_two_ok; }

        vec[4 + ok] = pct->one[i].idx;
        ok++;
    }

    for (i = 0; i < pct->num2; i++) {
        if (ok >= 2) { goto find_four_plus_two_ok; }

        if (ok == 0) {
            vec[4 + ok] = pct->two[i].idx0;
            vec[4 + ok + 1] = pct->two[i].idx1;
            ok += 2;
        } else {
            vec[4 + ok] = pct->two[i].idx0;
            ok++;
        }
    }

    for (i = 0; i < pct->num3; i++) {
        if (ok >= 2) { goto find_four_plus_two_ok; }

        if (ok == 0) {
            vec[4 + ok] = pct->three[i].idx0;
            vec[4 + ok + 1] = pct->three[i].idx1;
            ok += 2;
        } else {
            vec[4 + ok] = pct->three[i].idx0;
            ok++;
        }
    }

    for (i = 0; i < pct->num4; i++) {
        if (ok >= 2) { goto find_four_plus_two_ok; }

        if (pct->four[i].value != out->value) {
            if (ok == 0) {
                vec[4 + ok] = pct->four[i].idx0;
                vec[4 + ok + 1] = pct->four[i].idx1;
                ok += 2;
            } else {
                vec[4 + ok] = pct->four[i].idx0;
                ok++;
            }
        }
    }

    if (ok >= 2) { goto find_four_plus_two_ok; }

    if (count == 1) { count = 0; }

retrieve_bomb:
    RETRIEVE_BOMB();
    return false;

find_four_plus_two_ok:
    quick_sort(vec, 0, 5);
    return true;
}

// ����һ���ȸ���ֵ����Ĵ���
static bool retrieve_valid_four_plus_four(int times, int val, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i;
    int count = 0;

    assert((out != NULL) && (vec != NULL));

    // ����һ�����ʵ����ţ��ٲ����������ӣ����ɣ��������ը��

    for (i = 0; i < pct->num4; i++) {
        if (pct->four[i].value > val) {
            if (count >= times) {
                out->type = FOUR_PLUS_FOUR;
                out->num = 8;
                out->value = pct->four[i].value;
                vec[0] = pct->four[i].idx0;
                vec[1] = pct->four[i].idx1;
                vec[2] = pct->four[i].idx2;
                vec[3] = pct->four[i].idx3;

                goto find_two_pairs;
            } else {
                count++;
            }
        }
    }

    goto retrieve_bomb;

find_two_pairs:
    //
    // Ѱ���������ӡ�����ok��¼�ҵ���������������ҵ�4�����ʾ���ҳɹ�
    //
    int ok = 0;

    for (i = 0; i < pct->num2; i++) {
        if (ok >= 4) { goto find_four_plus_four_ok; }

        vec[4 + ok] = pct->two[i].idx0;
        vec[4 + ok + 1] = pct->two[i].idx1;
        ok += 2;
    }

    for (i = 0; i < pct->num3; i++) {
        if (ok >= 4) { goto find_four_plus_four_ok; }

        vec[4 + ok] = pct->three[i].idx0;
        vec[4 + ok + 1] = pct->three[i].idx1;
        ok += 2;
    }

    for (i = 0; i < pct->num4; i++) {
        if (ok >= 4) { goto find_four_plus_four_ok; }

        if (pct->four[i].value != out->value) {
            vec[4 + ok] = pct->four[i].idx0;
            vec[4 + ok + 1] = pct->four[i].idx1;
            ok += 2;
        }
    }

    if (ok >= 4) { goto find_four_plus_four_ok; }

    if (count == 1) {
        //
        // ����������е��˴���˵���乹���Ĵ���ʧ�ܣ�������������
        // ��find_four_plus_four_ok����֧�����ǵ�ǰ�ġ�retrieve_bomb����֧��
        // �����Ƶ���ʾ���ܻ��ж���ģ����� count �봫����� times ����Ϊ
        // ���Ʒ��صڼ�����ʾ������ģ�������ͼ�����Ĵ���ʱ���ܻ��޸� count ��ֵ��
        //
        // ʵ���ϣ�count ֵ��ӳ���ǲ�����ʾ�ɹ��ļ����������������е��˴���
        // ��ʾ�������ųɹ������Ҹ�������������ʧ�ܣ�Ϊʹ�� count ֵ��ȷ��ӳ
        // ������ʾ�ɹ��ļ������ڽ������ը��֮ǰ��Ӧ���临λΪ 0
        //
        // �߼���Ӧ���������ģ�������ܻ������ظ���ʾ���������Һ���Ҳ���ܴ���
        // ���������
        //
        // ʾ�����ϼҳ���Ϊ�Ĵ��ģ����ϵ���Ϊ��8888AAAA��������ظ���ʾ��
        //
        count = 0;
    }

retrieve_bomb:
    RETRIEVE_BOMB();
    return false;

find_four_plus_four_ok:
    quick_sort(vec, 0, 7);
    return true;
}

// ����һ���ȸ���ֵ������������ͬ��˳��
static bool retrieve_valid_series(int times, int val, int num, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i, j, k;
    int len;
    int value;
    int count = 0;
    int series_count = 0;

    assert((out != NULL) && (vec != NULL));

    //
    // �Ƶķ�������Ѿ���������õľ����ܳ���˳�ӣ�������Щ˳�ӣ�����ҵ�һ����
    // ��ֵ�������ֵ��ģ����䳤�ȱ������˳�ӻ���������ٹ�������ʵ�˳�ӡ�
    //
    for (i = 0; i < pct->num11; i++) {
        value = pct->sone[i].value;
        len = pct->sone[i].num;

        if (value > val) {
            if (len == num) {
                if (count >= times) {
                    out->type = SERIES;
                    out->num = num;
                    out->value = value;
                    for (j = 0; j < num; j++) {
                        vec[j] = pct->sone[i].idx[j];
                    }
                    return true;
                } else {
                    count++;
                }
            } else if (len > num) {
                //
                // ������е�˳�ӱ������˳�ӻ�����������ֵ��ͳ��Ȳ��ȷ��
                // ƥ���˳�ӵ����������ϵ���´������ʾ��
                //
                if (value - val <= len - num) {
                    series_count = value - val;
                } else {
                    series_count = len - num + 1;
                }

                for (k = series_count - 1; k >= 0; k--) {
                    if (count >= times) {
                        out->type = SERIES;
                        out->num = num;
                        out->value = value - k;
                        for (j = 0; j < num; j++) {
                            vec[j] = pct->sone[i].idx[len - num - k + j];
                        }
                        return true;
                    } else {
                        count++;
                    }
                }
            }
        }
    }

    RETRIEVE_BOMB();
    return false;
}


// ���Һ��ʵ�����
static bool retrieve_valid_series_pair(int times, int val, int num, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i, j, k;
    int len;
    int value;
    int count = 0;
    int series_count = 0;

    assert((out != NULL) && (vec != NULL));

    //
    // ���������˳������
    //

    for (i = 0; i < pct->num22; i++) {
        value = pct->stwo[i].value;
        len = pct->stwo[i].num;

        if (value > val) {
            if (len == num) {
                if (count >= times) {
                    out->type = SERIES_PAIR;
                    out->num = num;
                    out->value = value;
                    for (j = 0; j < num; j++) {
                        vec[j] = pct->stwo[i].idx[j];
                    }
                    return true;
                } else {
                    count++;
                }
            } else if (len > num) {
                if (value - val <= (len - num) / 2) {
                    series_count = value - val;
                } else {
                    series_count = (len - num) / 2 + 1;
                }

                for (k = series_count - 1; k >= 0; k--) {
                    if (count >= times) {
                        out->type = SERIES_PAIR;
                        out->num = num;
                        out->value = value - k;
                        for (j = 0; j < num; j++) {
                            vec[j] = pct->stwo[i].idx[len - num - k * 2 + j];
                        }
                        return true;
                    } else {
                        count++;
                    }
                }
            }
        }
    }

    RETRIEVE_BOMB();
    return false;
}

// ���Һ��ʵ�����
static bool retrieve_valid_series_triangle(int times, int val, int num, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i, j, k;
    int len;
    int value;
    int count = 0;
    int series_count = 0;

    assert((out != NULL) && (vec != NULL));

    //
    // ���������˳������
    //

    for (i = 0; i < pct->num33; i++) {
        value = pct->sthree[i].value;
        len = pct->sthree[i].num;

        if (value > val) {
            if (len == num) {
                if (count >= times) {
                    out->type = SERIES_TRIANGLE;
                    out->num = num;
                    out->value = value;
                    for (j = 0; j < num; j++) {
                        vec[j] = pct->sthree[i].idx[j];
                    }
                    return true;
                } else {
                    count++;
                }
            } else if (len > num) {
                if (value - val <= (len - num) / 3) {
                    series_count = value - val;
                } else {
                    series_count = (len - num) / 3 + 1;
                }

                for (k = series_count - 1; k >= 0; k--) {
                    if (count >= times) {
                        out->type = SERIES_TRIANGLE;
                        out->num = num;
                        out->value = value - k;
                        for (j = 0; j < num; j++) {
                            vec[j] = pct->sthree[i].idx[len - num - k * 3 + j];
                        }
                        return true;
                    } else {
                        count++;
                    }
                }
            }
        }
    }

    RETRIEVE_BOMB();
    return false;
}

// ���Һ��ʵ�������һ
static bool retrieve_valid_series_three_plus_one(int times, int val, int num, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i, j, k;
    int m, n;
    int len;
    int value;
    int count = 0;
    int series_count = 0;
    int ok = 0; // ���ҵ��ųɹ��ļ���

    assert(num % 4 == 0);
    assert((out != NULL) && (vec != NULL));

    m = num / 4; // m �������������� m ������
    n = m * 3; // ��������������3 ������������n = num - m��

    //
    // ����һ�����ʵ��������ٲ��ҵ��ţ����ѡ��ը��
    //

    for (i = 0; i < pct->num33; i++) {
        value = pct->sthree[i].value;
        len = pct->sthree[i].num;

        if (value > val) {
            if (len == n) {
                if (count >= times) {
                    out->type = SERIES_THREE_PLUS_ONE;
                    out->num = num;
                    out->value = value;
                    for (j = 0; j < n; j++) {
                        vec[j] = pct->sthree[i].idx[j];
                    }
                    goto find_single;
                } else {
                    count++;
                }
            } else if (len > n) {
                if (value - val <= (len - n) / 3) {
                    series_count = value - val;
                } else {
                    series_count = (len - n) / 3 + 1;
                }

                for (k = series_count - 1; k >= 0; k--) {
                    if (count >= times) {
                        out->type = SERIES_THREE_PLUS_ONE;
                        out->num = num;
                        out->value = value - k;
                        for (j = 0; j < n; j++) {
                            vec[j] = pct->sthree[i].idx[len - n - k * 3 + j];
                        }
                        goto find_single;
                    } else {
                        count++;
                    }
                }
            }
        }
    }

    goto find_bomb; // ��������ʧ�ܣ�Ѱ��ը��


find_single:
    // ����������ɣ��ٲ��ҳ� m �����ţ�������ɹ���
    for (i = 0; i < pct->num1; i++) { // �������
        if (ok >= m) { goto find_single_finished; }

        vec[n + ok] = pct->one[i].idx;
        ok++;
    }

    for (i = 0; i < pct->num2; i++) { // �������
        if (ok >= m) { goto find_single_finished; }

        vec[n + ok] = pct->two[i].idx0;
        ok++;

        if (ok < m) {
            vec[n + ok] = pct->two[i].idx1;
            ok++;
        }
    }

    for (i = 0; i < pct->num3; i++) { // �������
        if (ok >= m) { goto find_single_finished; }

        if ((pct->three[i].value > out->value - n / 3)
            && (pct->three[i].value <= out->value)) {
                //
                // �������Ѿ���ѡΪ���������ܽ�����Ϊ����
                //
                continue;
        }

        vec[n + ok] = pct->three[i].idx0;
        ok++;

        if (ok < m - 1) {
            vec[n + ok] = pct->three[i].idx1;
            vec[n + ok + 1] = pct->three[i].idx2;
            ok += 2;
        } else if (ok < m) {
            vec[n + ok] = pct->three[i].idx1;
            ok++;
        }
    }

    for (i = 0; i < pct->num4; i++) {
        if (ok >= m) { goto find_single_finished; }

        if ((pct->four[i].value > out->value - n / 3)
            && (pct->four[i].value <= out->value)) {
                //
                // �������Ѿ���ѡΪ���������ܽ�����Ϊ����
                //
                continue;
        }

        vec[n + ok] = pct->four[i].idx0;
        ok++;

        // Ϊ��ѭը�������ԭ�����ȡ����3������Ϊ����
        if (ok < m - 1) {
            vec[n + ok] = pct->four[i].idx1;
            vec[n + ok + 1] = pct->four[i].idx2;
            ok += 2;
        } else if (ok < m) {
            vec[n + ok] = pct->four[i].idx1;
            ok++;
        }
    }

    if (ok >= m) { goto find_single_finished; }

    if (count == 1) { count = 0; }

find_bomb:
    RETRIEVE_BOMB();
    return false;

find_single_finished:
    quick_sort(vec, 0, num - 1);
    return true;
}

// ���Һ��ʵ���������
static bool retrieve_valid_series_three_plus_two(int times, int val, int num, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i, j, k;
    int m, n;
    int len;
    int value;
    int count = 0;
    int series_count = 0;
    int ok = 0; // ���ҵ��ųɹ��ļ���

    assert(num % 5 == 0);
    assert((out != NULL) && (vec != NULL));

    m = num / 5; // m �������������� m ������
    n = m * 3; // ��������������3 ������������n = num - m * 2��

    //
    // ����һ�����ʵ��������ٲ��Ҷ��ӣ����ѡ��ը��
    //

    for (i = 0; i < pct->num33; i++) {
        value = pct->sthree[i].value;
        len = pct->sthree[i].num;

        if (value > val) {
            if (len == n) {
                if (count >= times) {
                    out->type = SERIES_THREE_PLUS_TWO;
                    out->num = num;
                    out->value = value;
                    for (j = 0; j < n; j++) {
                        vec[j] = pct->sthree[i].idx[j];
                    }
                    goto find_pair;
                } else {
                    count++;
                }
            } else if (len > n) {
                if (value - val <= (len - n) / 3) {
                    series_count = value - val;
                } else {
                    series_count = (len - n) / 3 + 1;
                }

                for (k = series_count - 1; k >= 0; k--) {
                    if (count >= times) {
                        out->type = SERIES_THREE_PLUS_TWO;
                        out->num = num;
                        out->value = value - k;
                        for (j = 0; j < n; j++) {
                            vec[j] = pct->sthree[i].idx[len - n - k * 3 + j];
                        }
                        goto find_pair;
                    } else {
                        count++;
                    }
                }
            }
        }
    }

    goto find_bomb; // ��������ʧ�ܣ�Ѱ��ը��

find_pair:
    // ����������ɣ��ٲ��ҳ� m �����ӣ�������ɹ���

    for (i = 0; i < pct->num2; i++) { // �������
        if (ok >= m * 2) { goto find_pair_finished; }

        vec[n + ok] = pct->two[i].idx0;
        vec[n + ok + 1] = pct->two[i].idx1;
        ok += 2;
    }

    for (i = 0; i < pct->num3; i++) { // �������
        if (ok >= m * 2) { goto find_pair_finished; }

        if ((pct->three[i].value > out->value - n / 3)
            && (pct->three[i].value <= out->value)) {
                //
                // �������Ѿ���ѡΪ���������ܽ�����Ϊ����
                //
                continue;
        }

        vec[n + ok] = pct->three[i].idx0;
        vec[n + ok + 1] = pct->three[i].idx1;
        ok += 2;
    }

    for (i = 0; i < pct->num4; i++) { // �������
        if (ok >= m * 2) { goto find_pair_finished; }

        if ((pct->four[i].value > out->value - n / 3)
            && (pct->four[i].value <= out->value)) {
                //
                // �������Ѿ���ѡΪ���������ܽ�����Ϊ����
                //
                continue;
        }

        vec[n + ok] = pct->four[i].idx0;
        vec[n + ok + 1] = pct->four[i].idx1;
        ok += 2;
    }

    if (ok >= m * 2) { goto find_pair_finished; }

    if (count == 1) { count = 0; }

find_bomb:
    RETRIEVE_BOMB();
    return false;

find_pair_finished:
    quick_sort(vec, 0, num - 1);
    return true;
}

// ���Һ��ʵ�����
static bool retrieve_valid_series_four(int times, int val, int num, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i, j, k;
    int len;
    int value;
    int count = 0;
    int series_count = 0;

    assert((out != NULL) && (vec != NULL));

    //
    // ���������˳������
    //

    for (i = 0; i < pct->num44; i++) {
        value = pct->sfour[i].value;
        len = pct->sfour[i].num;

        if (value > val) {
            if (len == num) {
                if (count >= times) {
                    for (j = 0; j < num; j++) {
                        vec[j] = pct->sfour[i].idx[j];
                    }
                    out->type = SERIES_FOUR;
                    out->num = num;
                    out->value = value;
                    return true;
                } else {
                    count++;
                }
            } else if (len > num) {
                if (value - val <= (len - num) / 4) {
                    series_count = value - val;
                } else {
                    series_count = (len - num) / 4 + 1;
                }

                for (k = series_count - 1; k >= 0; k--) {
                    if (count >= times) {
                        for (j = 0; j < num; j++) {
                            vec[j] = pct->sfour[i].idx[len - num - k * 4 + j];
                        }
                        out->type = SERIES_FOUR;
                        out->num = num;
                        out->value = value - k;
                        return true;
                    } else {
                        count++;
                    }
                }
            }
        }
    }

    RETRIEVE_BOMB();
    return false;
}

// ���Һ��ʵ����Ĵ���
static bool retrieve_valid_series_four_plus_two(int times, int val, int num, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{
    int i, j, k;
    int len;
    int value;
    int count = 0;
    int series_count = 0;
    int m, n;
    int ok = 0; // ���ڼ������ҵ������ĵ��Ÿ���

    assert(num % 6 == 0);
    assert((out != NULL) && (vec != NULL));

    m = num / 6 * 2; // ���ŵ�����Ϊ m
    n = num / 6 * 4; // ���ĵ�������Ϊ n

    //
    // �Ȳ��ҵ����ʵ����ģ��ٲ��Ҹ����ĵ��ţ����Ѱ��ը��
    //

    for (i = 0; i < pct->num44; i++) {
        value = pct->sfour[i].value;
        len = pct->sfour[i].num;

        if (value > val) {
            if (len == n) {
                if (count >= times) {
                    for (j = 0; j < n; j++) {
                        vec[j] = pct->sfour[i].idx[j];
                    }
                    out->type = SERIES_FOUR_PLUS_TWO;
                    out->num = num;
                    out->value = value;
                    goto find_two;
                } else {
                    count++;
                }
            } else if (len > n) {
                if (value - val <= (len - n) / 4) {
                    series_count = value - val;
                } else {
                    series_count = (len - n) / 4 + 1;
                }

                for (k = series_count - 1; k >= 0; k--) {
                    if (count >= times) {
                        for (j = 0; j < n; j++) {
                            vec[j] = pct->sfour[i].idx[len - n - k * 4 + j];
                        }
                        out->type = SERIES_FOUR_PLUS_TWO;
                        out->num = num;
                        out->value = value - k;
                        goto find_two;
                    } else {
                        count++;
                    }
                }
            }
        }
    }
    
    goto find_bomb;

find_two:
    for (i = 0; i < pct->num1; i++) { // �ӵ�������в���
        if (ok >= m) { goto find_single_finished; }

        vec[n + ok] = pct->one[i].idx;
        ok++;
    }

    for (i = 0; i < pct->num2; i++) { // �Ӷ�������в���
        if (ok >= m) { goto find_single_finished; }

        if (ok < m - 1) {
            vec[n + ok] = pct->two[i].idx0;
            vec[n + ok + 1] = pct->two[i].idx1;
            ok += 2;
        } else {
            vec[n + ok] = pct->two[i].idx0;
            ok++;
        }
    }

    for (i = 0; i < pct->num3; i++) { // ����������в���
        if (ok >= m) { goto find_single_finished; }

        if (ok < m - 2) {
            vec[n + ok] = pct->three[i].idx0;
            vec[n + ok + 1] = pct->three[i].idx1;
            vec[n + ok + 2] = pct->three[i].idx2;
            ok += 3;
        } else if (ok < m - 1) {
            vec[n + ok] = pct->three[i].idx0;
            vec[n + ok + 1] = pct->three[i].idx1;
            ok += 2;
        } else {
            vec[n + ok] = pct->three[i].idx0;
            ok++;
        }
    }

    for (i = 0; i < pct->num4; i++) { // ����������в��ң����ȡ������ͬ
        if (ok >= m) { goto find_single_finished; }

        if ((pct->four[i].value > out->value - n / 4)
            && (pct->four[i].value <= out->value)) {
                continue;
        }

        if (ok < m - 2) {
            vec[n + ok] = pct->four[i].idx0;
            vec[n + ok + 1] = pct->four[i].idx1;
            vec[n + ok + 2] = pct->four[i].idx2;
            ok += 3;
        } else if (ok < m - 1) {
            vec[n + ok] = pct->four[i].idx0;
            vec[n + ok + 1] = pct->four[i].idx1;
            ok += 2;
        } else {
            vec[n + ok] = pct->four[i].idx0;
            ok++;
        }
    }

    if (ok >= m) { goto find_single_finished; }

    if (count == 1) { count = 0; }

find_bomb:
    RETRIEVE_BOMB();
    return false;

find_single_finished:
    quick_sort(vec, 0, num - 1);
    return true;
}

// ���Һ��ʵ����Ĵ���
static bool retrieve_valid_series_four_plus_four(int times, int val, int num, POKER_CLASS_TABLE* pct, POKER_PROPERTY* out, int vec[])
{

    int i, j, k;
    int len;
    int value;
    int count = 0;
    int series_count = 0;
    int m, n;
    int ok = 0; // ���ڼ������ҵ������ĵ��Ÿ���

    assert(num % 8 == 0);
    assert((out != NULL) && (vec != NULL));

    m = num / 8 * 2; // �����Ķ�������Ϊ m����������������Ϊ (2 * m)
    n = num / 8 * 4; // ���ĵ�������Ϊ n

    //
    // �Ȳ��ҵ����ʵ����ģ��ٲ��Ҹ����ĵ��ţ����Ѱ��ը��
    //

    for (i = 0; i < pct->num44; i++) {
        value = pct->sfour[i].value;
        len = pct->sfour[i].num;

        if (value > val) {
            if (len == n) {
                if (count >= times) {
                    for (j = 0; j < n; j++) {
                        vec[j] = pct->sfour[i].idx[j];
                    }
                    out->type = SERIES_FOUR_PLUS_FOUR;
                    out->num = num;
                    out->value = value;
                    goto find_pair;
                } else {
                    count++;
                }
            } else if (len > n) {
                if (value - val <= (len - n) / 4) {
                    series_count = value - val;
                } else {
                    series_count = (len - n) / 4 + 1;
                }

                for (k = series_count - 1; k >= 0; k--) {
                    if (count >= times) {
                        for (j = 0; j < n; j++) {
                            vec[j] = pct->sfour[i].idx[len - n - k * 4 + j];
                        }
                        out->type = SERIES_FOUR_PLUS_FOUR;
                        out->num = num;
                        out->value = value - k;
                        goto find_pair;
                    } else {
                        count++;
                    }
                }
            }
        }
    }

    goto find_bomb;

find_pair:
    for (i = 0; i < pct->num2; i++) { // �Ӷ�������в���
        if (ok >= m * 2) { goto find_pair_finished; }

        vec[n + ok] = pct->two[i].idx0;
        vec[n + ok + 1] = pct->two[i].idx1;
        ok += 2;
    }

    for (i = 0; i < pct->num3; i++) { // ����������в���
        if (ok >= m * 2) { goto find_pair_finished; }

        vec[n + ok] = pct->three[i].idx0;
        vec[n + ok + 1] = pct->three[i].idx1;
        ok += 2;
    }

    for (i = 0; i < pct->num4; i++) { // ����������в��ң����ȡ������ͬ
        if (ok >= m * 2) { goto find_pair_finished; }

        if ((pct->four[i].value > out->value - n / 4)
            && (pct->four[i].value <= out->value)) {
                continue;
        }

        vec[n + ok] = pct->four[i].idx0;
        vec[n + ok + 1] = pct->four[i].idx1;
        ok += 2;
    }

    if (ok >= m * 2) { goto find_pair_finished; }

    if (count == 1) { count = 0; }

find_bomb:
    RETRIEVE_BOMB();
    return false;

find_pair_finished:
    quick_sort(vec, 0, num - 1);
    return true;
}

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
//      out     - ���ڷ��ز��ҵ�����ʾ�Ƶ�����
//      out_vec - ���ڷ��ز��ҵ�����ʾ�Ƶ��������飬�䳤�ȿ��ɷ��ص����Ի�á�
//
// Return:
//      TRUE if get a proper hint, else FALSE.
//
// Remark:
//      Finding PROPER pokers from [pct] according to the [req] poker property,
//      if operation succeeds, [out] poker property will contain info about the
//      pokers which were found, and the out_vec[] for corresponding indexes.
//      The [times] parameter indicates how many times the caller queried,
//      0 means first run.
// 
// ATTENTION:
//      ����Ĳ���vec���˿��Ƶ�ȫ���������飻
//      ���ص���������out_vec����Ԫ��Ϊ��ҵ�ǰ�Ƶ�������
//
POKER_API 
bool get_poker_hint(int times, POKER_CLASS_TABLE* pct, POKER_PROPERTY* req, POKER_PROPERTY* out, int out_vec[])
{
    int req_num;
    int req_type;
    int req_value;
    bool ret = false;

    assert((pct != NULL) && (req != NULL) && (out != NULL) && (out_vec != NULL));

    req_num = req->num;
    req_type = req->type;
    req_value = req->value;

    switch (req_type) {
        case SINGLE:
            ret = retrieve_valid_single(times, req_value, pct, out, out_vec);
            break;

        case PAIR:
            ret = retrieve_valid_pair(times, req_value, pct, out, out_vec);
            break;

        case TRIANGLE:
            ret = retrieve_valid_triangle(times, req_value, pct, out, out_vec);
            break;

        case THREE_PLUS_ONE:
            ret = retrieve_valid_three_plus_one(times, req_value, pct, out, out_vec);
            break;

        case THREE_PLUS_TWO:
            ret = retrieve_valid_three_plus_two(times, req_value, pct, out, out_vec);
            break;

        case BOMB:
            ret = retrieve_valid_bomb(times, req_value, pct, out, out_vec);
            break;

        case FOUR_PLUS_TWO:
            ret = retrieve_valid_four_plus_two(times, req_value, pct, out, out_vec);
            break;

        case FOUR_PLUS_FOUR:
            ret = retrieve_valid_four_plus_four(times, req_value, pct, out, out_vec);
            break;

        case SERIES:
            ret = retrieve_valid_series(times, req_value, req_num, pct, out, out_vec);
            break;

        case SERIES_PAIR:
            ret = retrieve_valid_series_pair(times, req_value, req_num, pct, out, out_vec);
            break;

        case SERIES_TRIANGLE:
            ret = retrieve_valid_series_triangle(times, req_value, req_num, pct, out, out_vec);
            break;

        case SERIES_THREE_PLUS_ONE:
            ret = retrieve_valid_series_three_plus_one(times, req_value, req_num, pct, out, out_vec);
            break;

        case SERIES_THREE_PLUS_TWO:
            ret = retrieve_valid_series_three_plus_two(times, req_value, req_num, pct, out, out_vec);
            break;

        case SERIES_FOUR:
            ret = retrieve_valid_series_four(times, req_value, req_num, pct, out, out_vec);
            break;

        case SERIES_FOUR_PLUS_TWO:
            ret = retrieve_valid_series_four_plus_two(times, req_value, req_num, pct, out, out_vec);
            break;

        case SERIES_FOUR_PLUS_FOUR:
            ret = retrieve_valid_series_four_plus_four(times, req_value, req_num, pct, out, out_vec);
            break;
    }

    return ret;
}
