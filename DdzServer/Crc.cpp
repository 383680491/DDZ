//
// Crc.cpp
//
//  ����CRCУ���֡����Ǵ������ҵĴ���
//

#include "stdafx.h"
#include "Crc.h"

// CRC-16 = X16 + X15 + X2 + X0
static unsigned short cnCRC_16 = 0x8005;
// CRC-CCITT = X16 + X12 + X5 + X0
static unsigned short cnCRC_CCITT = 0x1021;
// CRC16 �� 
static unsigned long Table_CRC16[256] = { 0 };
// CRC ���Ƿ��Ѿ�����
static BOOL g_bCrc16TableBuilded = FALSE;

// ����CRC16��
void BuildTable16(unsigned short aPoly)
{
    unsigned short i, j;
    unsigned short nData;
    unsigned short nAccum;

    for (i = 0; i < 256; i++) {
        nData = (unsigned short)(i << 8);
        nAccum = 0;
        for (j = 0; j < 8; j++) {
            if ((nData ^ nAccum) & 0x8000) {
                nAccum = (nAccum << 1) ^ aPoly;
            } else {
                nAccum <<= 1;
            }
            nData <<= 1;
        }
        Table_CRC16[i] = (unsigned long)nAccum;
    }
}

// ���� 16 λ CRC ֵ��CRC-16 �� CRC-CCITT
unsigned short CRC16(unsigned char * aData, unsigned long aSize)
{
    unsigned long i;
    unsigned short nAccum = 0;

    if (g_bCrc16TableBuilded == FALSE) {
        BuildTable16(cnCRC_16); // or cnCRC_CCITT
        g_bCrc16TableBuilded = TRUE;
    }

    for (i = 0; i < aSize; i++) {
        nAccum = (nAccum << 8) ^ (unsigned short)Table_CRC16[(nAccum >> 8) ^ *aData++];
    }

    return nAccum;
}