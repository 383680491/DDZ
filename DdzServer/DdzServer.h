#pragma once

#include "resource.h"

// ���ڼ��ش������ַ���
#define MAX_LOADSTRING 32


// �˴���ģ���а����ĺ�����ǰ������:
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);

BOOL CreateChildWindows(void);
void DoPokerSelCombo(HWND hWnd, int notify);
BOOL CALLBACK EnumChildWndProc(HWND hWnd, LPARAM lParam);

void OnLogOptions(HWND hWnd, int id, int nEvent);

