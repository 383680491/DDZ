//
// Crc.h
//
#pragma once

// ����16-bit��CRC��ʹ�ñ�׼CRC-16����ʽ
unsigned short CRC16(unsigned char* aData, unsigned long aSize);

static void BuildTable16(unsigned short aPoly);
