//
// File: Logon.cpp
//
// LOGON DIALOG
//
// IMPORTANT:
// The logon dialog used HIMAGELIST g_himlUser32x32, make sure the image list
// has already been created and initialized before call UserLogon().
//
#include "stdafx.h"
#include "DdzClient.h"
#include "AppResource.h"
#include "Logon.h"

extern HINSTANCE g_hAppInstance;

HWND g_hWndLogonTip = NULL;
BOOL g_bWndLogonTipVisible = FALSE;

// ���Ի�ComboBox
void DrawUserImgComboItem(LPDRAWITEMSTRUCT lpdis)
{
    int imgcount = ImageList_GetImageCount(g_himlUser32x32);
    if (lpdis->itemID >= (UINT)imgcount) {
        return;
    }

    IMAGEINFO imgInfo;
    if (ImageList_GetImageInfo(g_himlUser32x32, lpdis->itemID, &imgInfo) == FALSE) {
        return;
    }

    // ͷ��ͼ��λͼ�ߴ�
    int cxImage = imgInfo.rcImage.right - imgInfo.rcImage.left;
    int cyImage = imgInfo.rcImage.bottom - imgInfo.rcImage.top;

    // Ҫ������ľ��γߴ�
    int cxDrawItem = lpdis->rcItem.right - lpdis->rcItem.left;
    int cyDrawItem = lpdis->rcItem.bottom - lpdis->rcItem.top;

    // ���������ڴ�DC������Ƶ�λͼ
    HDC memDC = CreateCompatibleDC(lpdis->hDC);
    HBITMAP memBmp = CreateCompatibleBitmap(lpdis->hDC, cxDrawItem, cyDrawItem);
    HBITMAP oldmemBmp = (HBITMAP)SelectObject(memDC, memBmp);

    // ���ڴ�DC��λͼȫ��ˢΪ��ɫ
    RECT rcFill;
    SetRect(&rcFill, 0, 0, cxDrawItem, cyDrawItem);
    FillRect(memDC, &rcFill, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // ������ѡ�У����ڴ�DC �ӣ�1,1����ʼ����ͼ��ߴ磫2�ľ��λ�����ɫ�߿�
    if ((lpdis->itemState & ODS_FOCUS) || (lpdis->itemState & ODS_SELECTED)) {
        HBRUSH hbrush = CreateSolidBrush(RGB(0, 0, 255));
        SetRect(&rcFill, 1, 1, (1 + cxImage + 2), (1 + cyImage + 2));
        FrameRect(memDC, &rcFill, hbrush);
        DeleteObject(hbrush);
    }

    // ����Ӧͼ����Ƶ��ڴ�DC�ģ�2,2����ʼ��
    ImageList_Draw(g_himlUser32x32, lpdis->itemID, memDC, 2, 2, ILD_NORMAL);

    // ���ڴ�DCˢ����Ļ
    BitBlt(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, cxDrawItem, cyDrawItem,
        memDC, 0, 0, SRCCOPY);

    // ��λԭλͼ
    SelectObject(memDC, oldmemBmp);

    // ����
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

// ��ʼ���û�ͷ���б�
void InitUserImageCombox(HWND hComboBox)
{
    int imgcount = ImageList_GetImageCount(g_himlUser32x32);

    for (int i = 0; i < imgcount; i++) {
        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)i);
    }

    SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
}

// ��½�Ի�����Ӧ�ؼ�����
void OnLogonDlgCommand(HWND hDlg, int nResult)
{
    int len = 0;
    TCHAR name[MAX_USER_NAME_LEN] = { 0 };
    int nImage = 0;
    BOOL bMale = TRUE;

    if (nResult == IDCANCEL) { // ESCȡ����رմ���
        EndDialog(hDlg, nResult);
        return;
    }
    
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_TRACK;

    if (nResult == IDOK) { // OK��ť
        len = GetDlgItemText(hDlg, IDC_LOGON_NAME, name, MAX_USER_NAME_LEN);

        // �����ǳƱ༭��TIP�ļ�ͷ��
        RECT rect;
        GetWindowRect(GetDlgItem(hDlg, IDC_LOGON_NAME), &rect);

        POINT pt;
        pt.x = rect.left + 4;
        pt.y = rect.bottom - 4;

        SendMessage(g_hWndLogonTip, TTM_TRACKPOSITION, 0, (LPARAM)POINTTOPOINTS(pt));

        if (len == 0) {
            ti.uId = IDC_LOGON_NAME;
            ti.lpszText = _T("�����������ǳ�(������31���ַ�)");
            SendMessage(g_hWndLogonTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

            g_bWndLogonTipVisible = TRUE;
            SendMessage(g_hWndLogonTip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
        } else if (name[0] == _T(' ')) {
            ti.uId = IDC_LOGON_NAME;
            ti.lpszText = _T("�벻Ҫʹ�ÿո���Ϊ�ǳƵĿ�ʼ");
            SendMessage(g_hWndLogonTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

            g_bWndLogonTipVisible = TRUE;
            SendMessage(g_hWndLogonTip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
        } else {
            nImage = SendMessage(GetDlgItem(hDlg, IDC_LOGON_USER_IMG), CB_GETCURSEL, 0, 0);
            bMale = BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_LOGON_MALE), BM_GETCHECK, 0, 0);

            SaveUserLogonInfo(hDlg, name, nImage, bMale);
            EndDialog(hDlg, IDOK);
        }
    } else { // ��TIP�����Ѿ���ʾ����ʹ������
        if (g_bWndLogonTipVisible == TRUE) {
            SendMessage(g_hWndLogonTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
            g_bWndLogonTipVisible = FALSE;
        }
    }
}

// ��½�Ի��򴰿ڴ������
INT_PTR CALLBACK LogonDialogProc(HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    int nResult;
    LPMEASUREITEMSTRUCT lpmis; 
    LPDRAWITEMSTRUCT lpdis;
    HICON hicon;

    switch (nMsg) {
    case WM_INITDIALOG:
        // ���Ի�����������ͼ��
        hicon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DDZCLIENT));
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hicon);

        // ��������Ĳ���
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

        // ����һ��TIP��ʾ�ؼ�
        g_hWndLogonTip = CreateWindow(TOOLTIPS_CLASS,
            NULL, TTS_NOPREFIX | TTS_BALLOON,
            0, 0, 0, 0, hDlg, NULL, g_hAppInstance, NULL);

        // Ϊ��½�ǳƱ༭�����TIP
        TOOLINFO ti;
        ZeroMemory(&ti, sizeof(TOOLINFO));
        ti.cbSize = sizeof(TOOLINFO);
        ti.uFlags = TTF_TRACK;
        ti.uId = IDC_LOGON_NAME;
        SendMessage(g_hWndLogonTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

        // ��ʼ���û�ͷ���б�
        InitUserImageCombox(GetDlgItem(hDlg, IDC_LOGON_USER_IMG));

        // Ĭ�������û��Ա�Ϊ��
        SendDlgItemMessage(hDlg, IDC_LOGON_MALE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        return TRUE;

    case WM_COMMAND:
        nResult = LOWORD(wParam);
        OnLogonDlgCommand(hDlg, nResult);
        return TRUE;

    case WM_ACTIVATE:
    case WM_NCLBUTTONDOWN:
    case WM_LBUTTONDOWN:
    case WM_NCRBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (g_bWndLogonTipVisible == TRUE) { // ��TIP�����Ѿ���ʾ����ʹ������
            SendMessage(g_hWndLogonTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
            g_bWndLogonTipVisible = FALSE;
        }
        return FALSE; // ����FALSE����ʾ��Ҫϵͳ�����������Ϣ

    case WM_MEASUREITEM:
        lpmis = (LPMEASUREITEMSTRUCT)lParam;

        if (lpmis->CtlType == ODT_COMBOBOX) {
            if (lpmis->CtlID == IDC_LOGON_USER_IMG) {
                lpmis->itemHeight = CY_USER_ICON_BIG + 4;
            }
        }
        return TRUE;

    case WM_DRAWITEM:
        lpdis = (LPDRAWITEMSTRUCT)lParam;

        if (lpdis->CtlType == ODT_COMBOBOX) {
            if (lpdis->CtlID == IDC_LOGON_USER_IMG) {
                DrawUserImgComboItem(lpdis);
            }
        }
        return TRUE;
    }

    return FALSE;
}

// ��ʾ��½�Ի��򣬽����û���½
BOOL UserLogon(HINSTANCE hInstance, HWND hWndParent, LPARAM lParam)
{
    int nResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_LOGON),
        hWndParent, LogonDialogProc, lParam);
    if (nResult == IDOK) {
        return TRUE;
    }

    return FALSE;
}

// �����û���½��Ϣ
BOOL SaveUserLogonInfo(HWND hDlg, LPCTSTR name, int nImage, BOOL bMale)
{
    USER_LOGON_INFO* uli = (USER_LOGON_INFO*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    if (uli != NULL) {
        _tcscpy_s(uli->szName, MAX_USER_NAME_LEN, name);
        uli->nImage = nImage;
        uli->bMale = bMale;
        return TRUE;
    }

    return FALSE;
}
