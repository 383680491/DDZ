//
// File: HomePageWnd.h
//
// shining      2009-05-10      initial creation
//
#pragma once

#define HOME_PAGE_WND_CLASS_NAME        _T("_HPW_CLASS_")

//
// ����������ҪĿ������ʾһ��HTMLҳ�棬����Ϊ��������Ϸ�İ���ҳ�档
//
// MSDN��û�о�����������Win32��Ŀ��Ƕ��IE��
//
// �������ҵ���CWebPage����֧��Win32����MFC������Ƕ��IE�����������DLLԴ����Ŀ
// ���뱾����������뱾��Ŀһ����롣����������ʱ��������DLL����������ʱж�ء�
//
// CWebPage����Ƚ����أ���û������Ƕ��IE�ľ������������ֻ�������丽����ʾ����
// �ڱ�������ʹ�ø�DLL��ʵ��HTMLҳ��ļ�������ʾ��
//
// CWebPage�ƺ�Ҳ�ȽϹ��졣����û��ʹ�ø�ģ��ʱ������ʱ�����ڴ�ʹ����������4MB��
// �����ظ�DLL֮�󣬳�������ʱ����ռ�õ��ڴ泬��20MB��������Ȼ������10���δ֪
// �̣߳�����IE�ؼ���������Ҫ��ô�����Դ�ɡ�δ֪�Ķ����Ƚ϶࣬�޷����вü���
//
// ��Ҫע�⣺CWebPage��IEǶ�봰��ʱ������ʹ�ô��ڵ�USER_DATA����ˣ���Ҫ�ڱ�����
// ���У����������䣺 SetWindowLongPtr(hWnd, GWLP_USERDATA, pMyData)
// 
// �������º꣬����ʹ��CWebPage��
//
//#define NOT_USE_CWEBPAGE_DLL

class HomePageWnd
{
    HWND    m_hWnd;

    static int m_snInstances;
    static HMODULE m_shCWebPageDll;

    static EmbedBrowserObjectPtr        *lpEmbedBrowserObject;
    static UnEmbedBrowserObjectPtr      *lpUnEmbedBrowserObject;
    static DisplayHTMLPagePtr           *lpDisplayHTMLPage;
    static DisplayHTMLStrPtr            *lpDisplayHTMLStr;
    static ResizeBrowserPtr             *lpResizeBrowser;

public:
    HomePageWnd(void);
    ~HomePageWnd(void);

protected:
    static BOOL LoadDll(void);
    static void UnLoadDll(void);

    static void OnWndPaint(HWND hWnd, HDC hdc);
    static void OnWndSize(HWND hWnd, LPARAM lParam);

public:
    static ATOM HomePageWndRegister(HINSTANCE hInstance);
    static LRESULT CALLBACK HomePageWndProc(HWND, UINT, WPARAM, LPARAM);

public:
    HWND Create(int x, int y, int cx, int cy, HWND hWndParent, HINSTANCE hInstance);
    void SetPageURL(LPCTSTR szURL);
    void SetWindowRect(int x, int y, int cx, int cy);
    void Show(BOOL bShow = TRUE);

    operator HWND() const {
        return m_hWnd;
    }
};