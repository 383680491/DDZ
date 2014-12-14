//
// File: Logon.h
//
#pragma once

static void InitUserImageCombox(HWND hComboBox);
static void OnLogonDlgCommand(HWND hDlg, int nResult);
static BOOL SaveUserLogonInfo(HWND hDlg, LPCTSTR name, int nImage, BOOL bMale);
static INT_PTR CALLBACK LogonDialogProc(HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam);

// ��ʾ��½�Ի��򣬲����û���½��Ϣ���浽 lParam ��ָ�ṹ
BOOL UserLogon(HINSTANCE hInstance, HWND hWndParent, LPARAM lParam);
