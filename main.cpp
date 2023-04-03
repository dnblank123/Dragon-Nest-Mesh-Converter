/***********************************************************
*龙之谷MSH模型转换程序V1.2
*
*编译平台:Win32 系统内核 NT6.1 7600(Windows 7 X64)
*
*编译环境:Microsoft Visual C++ 2008
*
*功能:用于龙之谷模型文件与FBX格式相互转换
*
*作者:GPBeta (studiocghibli@gmail.com) 2012年03月31日于茂名
*
*许可:NONE
***********************************************************/
#include "stdafx.h"
#include "translate.h"

//#define APP_NAME _T("Dragon Nest Mesh Converter")
//#define APP_VERSION _T("1.0.0") 
//#define LANG_BROWSE _T("Browse")
//#define LANG_CONVERT _T("Convert")
//#define LANG_CHOOSE_MODEL _T("Choose Model")
//#define LANG_CHOOSE_ANI _T("Choose ANI")

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    static WCHAR szAppName[] = APP_NAME APP_VERSION;
    HWND hwnd ;
    MSG msg ;
    WNDCLASS wndclass ;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, (LPCWSTR)ICON_LOH);
    wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground  = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    wndclass.lpszMenuName = L"MENU";
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))return 0;    //注册窗体

    hwnd = CreateWindow (szAppName, //创建主窗体
                        szAppName,
                        WS_OVERLAPPED| WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        480,
                        320,
                        NULL,
                        NULL,
                        hInstance,
                        NULL) ;

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

//消息循环
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg) ;
        DispatchMessage(&msg) ;
        }

    return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HFONT myfont;   //字体
    static int parts[] = {300,-1};   //状态栏分栏宽度
    static PAINTSTRUCT ps;
    //创建窗口句柄
    static HWND hWndStatus, titlehwnd,
                btn_mshhwnd, btn_anihwnd, btn_exchwnd,
                ebox_mshhwnd, ebox_anihwnd,
                chkbox_meshhwnd, chkbox_bonehwnd, chkbox_bindhwnd, chkbox_otherhwnd;

    static bool excResault = true;
    static wchar_t inputFileName[MAX_PATH] = {0},
                   aniFileName[MAX_PATH] = {0},
                   outputFileName[MAX_PATH] = {0};

    static int inputFormat = -1,
               aniFormat = -1,
               outputFormat = -1;

    //初始化FBXSDK管理器
    static FbxManager* lSdkManager;

    switch (message){
        case WM_CREATE:
            //创建操作界面元素
            titlehwnd = CreateWindowEx(0,
                                    L"STATIC",
                                    APP_NAME APP_VERSION,
                                    WS_CHILD | SS_SIMPLE | WS_VISIBLE,
                                    130, 20,
                                    300, 25,
                                    hwnd,
                                    NULL,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            //创建按钮
            btn_mshhwnd = CreateWindowEx(0,
                                    L"BUTTON",
                                    LANG_BROWSE,
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    20, 60,
                                    60, 25,
                                    hwnd,
                                    (HMENU)IMPORT_FILE_BUTTON,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            btn_anihwnd = CreateWindowEx(0,
                                    L"BUTTON",
                                    LANG_BROWSE,
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    20, 110,
                                    60, 25,
                                    hwnd,
                                    (HMENU)IMPORT_ANI_BUTTON,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            btn_exchwnd = CreateWindowEx(0,
                                    L"BUTTON",
                                    LANG_CONVERT,
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    170, 200,
                                    120, 30,
                                    hwnd,
                                    (HMENU)CONVERT_BUTTON,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            //创建编辑框
            ebox_mshhwnd = CreateWindowEx(WS_EX_STATICEDGE,
                                    L"EDIT",
                                    LANG_CHOOSE_MODEL,
                                    WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER,
                                    100, 60,
                                    350, 23,
                                    hwnd,
                                    (HMENU)IMPORT_FILE_EDITBOX,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            ebox_anihwnd = CreateWindowEx(WS_EX_STATICEDGE,
                                    L"EDIT",
                                    LANG_CHOOSE_ANI,
                                    WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER,
                                    100, 110,
                                    350, 23,
                                    hwnd,
                                    (HMENU)IMPORT_ANI_EDITBOX,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            //创建复选框
            chkbox_meshhwnd = CreateWindowEx(0,
                                    L"BUTTON",
                                    L"MeshData",
                                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_3STATE,
                                    20, 150,
                                    80, 30,
                                    hwnd,
                                    (HMENU)MESH_DATA_EDITBOX,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            chkbox_bonehwnd = CreateWindowEx(0,
                                    L"BUTTON",
                                    L"BoneData",
                                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_3STATE,
                                    130, 150,
                                    80, 30,
                                    hwnd,
                                    (HMENU)BONE_DATA_EDITBOX,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            chkbox_bindhwnd = CreateWindowEx(0,
                                    L"BUTTON",
                                    L"BindData",
                                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_3STATE,
                                    240, 150,
                                    80, 30,
                                    hwnd,
                                    (HMENU)BIND_DATA_EDITBOX,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            chkbox_otherhwnd = CreateWindowEx(0,
                                    L"BUTTON",
                                    L"*.ANI",
                                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                    350, 150,
                                    80, 30,
                                    hwnd,
                                    (HMENU)OTHER_DATA_EDITBOX,
                                    ((LPCREATESTRUCT)lParam)->hInstance,
                                    NULL);
            //创建状态栏
            hWndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,
                                    NULL,   //第一个栏位的文本
                                    hwnd,   //父窗口句柄
                                    STATUS_WINDOWS);   //状态栏ID
            //状态栏分栏
            SendMessage(hWndStatus, SB_SETPARTS, 2, (long)parts);
            SendMessage(hWndStatus, SB_SETTEXT, 0, (long)LANG_READY);
            SendMessage(hWndStatus, SB_SETTEXT, 1, (long)L"  by GPBeta");

            //初始化标题字体
            myfont = CreateFont(16,0,0,0,FW_BOLD,0,0,0,0,0,0,0,0,0);
            SendMessage(titlehwnd, WM_SETFONT, (WPARAM)myfont, TRUE);

            //初始化按钮字体
            myfont = CreateFont(14,0,0,0,FW_BOLD,0,0,0,0,0,0,0,0,0);
            SendMessage(btn_mshhwnd, WM_SETFONT, (WPARAM)myfont, TRUE);
            SendMessage(btn_anihwnd, WM_SETFONT, (WPARAM)myfont, TRUE);
            SendMessage(btn_exchwnd, WM_SETFONT, (WPARAM)myfont, TRUE);
            //初始化复选框字体
            SendMessage(chkbox_meshhwnd, WM_SETFONT, (WPARAM)myfont, TRUE);
            SendMessage(chkbox_bonehwnd, WM_SETFONT, (WPARAM)myfont, TRUE);
            SendMessage(chkbox_bindhwnd, WM_SETFONT, (WPARAM)myfont, TRUE);
            SendMessage(chkbox_otherhwnd, WM_SETFONT, (WPARAM)myfont, TRUE);

            //初始化编辑框字体
            myfont = CreateFont(16,0,0,0,FW_NORMAL,0,0,0,0,0,0,0,0,0);
            SendMessage(ebox_mshhwnd, WM_SETFONT, (WPARAM)myfont, TRUE);
            SendMessage(ebox_anihwnd, WM_SETFONT, (WPARAM)myfont, TRUE);

            //设定复选框状态
            SendMessage(chkbox_meshhwnd, BM_SETCHECK, BST_INDETERMINATE, TRUE);
            SendMessage(chkbox_bonehwnd, BM_SETCHECK, BST_INDETERMINATE, TRUE);
            SendMessage(chkbox_bindhwnd, BM_SETCHECK, BST_INDETERMINATE, TRUE);
            SendMessage(chkbox_otherhwnd, BM_SETCHECK, BST_INDETERMINATE, TRUE);

            //初始化FBX SDK
            lSdkManager = InitialFbxSdk();
            return 0;
        case WM_PAINT:
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            //菜单消息
            switch(LOWORD(wParam)){
                case IMPORT_FILE_BUTTON:
                    GetFileName(hwnd, lSdkManager, inputFileName, inputFormat, 0);
                    if(*inputFileName)
                        SetWindowText(GetDlgItem(hwnd, IMPORT_FILE_EDITBOX), inputFileName);
                    break;
                case IMPORT_ANI_BUTTON:
                    GetFileName(hwnd, lSdkManager, aniFileName, aniFormat, 1);
                    if(*aniFileName)
                        SetWindowText(GetDlgItem(hwnd, IMPORT_ANI_EDITBOX), aniFileName);
                    break;
                case CONVERT_BUTTON:
                    //获取输出文件名
                    GetFileName(hwnd, lSdkManager, outputFileName, outputFormat, 2);
                    //无ani需求则清空文件名缓冲
                    if(IsDlgButtonChecked(hwnd, OTHER_DATA_EDITBOX) == BST_UNCHECKED){
                        ZeroMemory(aniFileName, MAX_PATH);
                        SetWindowText(GetDlgItem(hwnd, IMPORT_ANI_EDITBOX), LANG_CHOOSE_ANI);
                    }
                    if(*outputFileName && *inputFileName){
                        if(inputFormat == 5 && !*aniFileName &&
                           IsDlgButtonChecked(hwnd, OTHER_DATA_EDITBOX) == BST_CHECKED)
                            MessageBox(hwnd, LANG_MSG_REQUIRE_ANI, LANG_WARN, MB_ICONINFORMATION);
                        else{
                                SendMessage(hWndStatus, SB_SETTEXT, 0, (long)LANG_WORKING);
                                excResault = ConvertFile(lSdkManager,
                                                         inputFileName,
                                                         inputFormat,
                                                         aniFileName,
                                                         aniFormat,
                                                         outputFileName,
                                                         outputFormat);
                                if(excResault){
                                    MessageBox(hwnd, LANG_MSG_SUCCESS, LANG_INFO,0);
                                    SendMessage(hWndStatus, SB_SETTEXT,0, (long)LANG_DONE);
                                }
                                else{
                                    MessageBox(hwnd, LANG_MSG_FAILURE, LANG_ERROR, MB_ICONERROR);
                                    SendMessage(hWndStatus, SB_SETTEXT, 0, (long)LANG_FAIL);
                                }
                        }
                        //还原设置
                        lSdkManager = InitialFbxSdk();
                    }
                    else
                        MessageBox(hwnd, LANG_MSG_REQUIRE_FILE, LANG_WARN, MB_ICONINFORMATION);
                    break;
                case IDM_EXIT:
                    PostQuitMessage(0);
                    break;
                case IDM_ABOUT:
                    MessageBox(hwnd, APP_INFO, LANG_ABOUT, MB_ICONINFORMATION);
                    break;
                default: break;
            }
            return 0 ;
        }
return DefWindowProc (hwnd, message, wParam, lParam) ;
}
