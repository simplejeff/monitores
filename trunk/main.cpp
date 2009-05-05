/*
   VERSION 0.3  

   TODO : 
        * i18n Support, 
        * Yahoo Messenger , MSN , SKYPE Support
        * VLC Player Support / any other media players.
        * Fix Memory leaks

   Copyright 2009 Umakanthan Chandran (cumakt@gmail.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions
   and limitations under the License.
   */

#define _UNICODE
#include <windows.h>
#include "tray.h"

#define NOTIFY_FOR_THIS_SESSION 0
#define WM_WTSSESSION_CHANGE 0x02B1
#define WTS_SESSION_LOGON                  0x5
#define WTS_SESSION_LOGOFF                 0x6
#define WTS_SESSION_LOCK                   0x7
#define WTS_SESSION_UNLOCK                 0x8
#define ID_MON 1
#define ID_SCR 2
#define ID_INTIME 5
#define ID_TITLE 6
#define ID_INTIME_CHECK 7
#define ID_IPADDRESS 8
#define OPTION_BNCLICK 9
#define OPTION_EXITCLICK 10
#define OPTION_HELPCLICK 11
#define MAX_X 240
#define MAX_Y 120
#define START_X 20
HINSTANCE g_hinst;
HWND o1,o2,b1,g1,intime,hwndButton,hwndExitButton,hwndHelpButton;
HDC hdc;
HBRUSH hbr;

int ID_GLOBAL_FLAG = 0;
int ID_WEEKEND_FLAG = 0;
int AUTO_START_FLAG = 0;
int MEDIA_CHECK_FLAG = 0;
int MESSENGER_CHECK_FLAG = 0;
bool flagFirstTime = false;
char *LocalTimeHHMM;
char *ipAddress;
SYSTEMTIME st;
HFONT font,font2;
char sBuffer[1000];
DWORD ret;


bool GetLocalTimeHHMM(char *LocalTimeHHMM);

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
char szClassName[ ] = "MonitorES";
char szTitleText[ ] = "Monitor ES 0.3";

char * originalMessage;

int (__stdcall * WorkStationStatus)();


void alert(char *item) {
    MessageBox(NULL, item,  "Message", MB_OK | MB_ICONINFORMATION);
}
/* Main Program call */

HWND GetRunningWindow() {
    // Check if exists an application with the same class name as this application
    HWND hWnd = FindWindow(szClassName, NULL);
    if (IsWindow(hWnd)) {
        HWND hWndPopup = GetLastActivePopup(hWnd);
        if (IsWindow(hWndPopup))
            hWnd = hWndPopup; // Previous instance exists
    }
    else hWnd = NULL; // Previous instance doesnt exist
    return hWnd;
}
void UnregisterSession(HWND hwnd) {
    typedef DWORD (WINAPI *tWTSUnRegisterSessionNotification)( HWND,DWORD );

    tWTSUnRegisterSessionNotification pWTSUnRegisterSessionNotification=0;
    HINSTANCE handle = ::LoadLibrary("wtsapi32.dll");
    pWTSUnRegisterSessionNotification = (tWTSUnRegisterSessionNotification) ::GetProcAddress(handle,"WTSUnRegisterSessionNotification");
    if (pWTSUnRegisterSessionNotification) {
        pWTSUnRegisterSessionNotification(hwnd,NOTIFY_FOR_THIS_SESSION);
    }
    ::FreeLibrary(handle);

}

void RegisterSession(HWND hwnd) {
    typedef DWORD (WINAPI *tWTSRegisterSessionNotification)( HWND,DWORD );

    tWTSRegisterSessionNotification pWTSRegisterSessionNotification=0;
    HINSTANCE handle = ::LoadLibrary("wtsapi32.dll");
    pWTSRegisterSessionNotification = (tWTSRegisterSessionNotification) :: GetProcAddress(handle,"WTSRegisterSessionNotification");
    if (pWTSRegisterSessionNotification) {
        pWTSRegisterSessionNotification(hwnd,NOTIFY_FOR_THIS_SESSION);
    }
    ::FreeLibrary(handle);
}

void controlMediaRunning() {
    if (MEDIA_CHECK_FLAG) {
        HWND hwndWinamp = FindWindow("Winamp v1.x",NULL); //Finding window
        if (hwndWinamp != NULL) {
            SendMessage(hwndWinamp,WM_COMMAND, 40046, 1); // Pause/play
        }
        HWND hwndWMP = FindWindow("WMPlayerApp",NULL);
        if (hwndWMP != NULL) {
            SendMessage(hwndWMP,WM_COMMAND, 0x00004978, 1); // Pause/play
        }
        /*HWND hwndVLC = FindWindow("QWidget",NULL);
        if (hwndVLC != NULL) {
            ShowWindow(hwndVLC, SW_RESTORE);
            SetFocus(hwndVLC);
            SendMessage(hwndVLC, WM_KEYDOWN, VK_SPACE, 0);
        }*/
    }
}

//Copyright Viktor Brange AKA Vikke
//You are allowed to change the code :P
//And don't forget to link to CustomizeTalk.com
//Thanks to Wumpus who made ChangeStatus for VB.

long SendMouseEvent(HWND hwnd, long Msg, int X, int Y) {
    LPARAM lParam =
        MAKELPARAM(X * GetDeviceCaps(GetDC(NULL), LOGPIXELSX) / 1440,
                   Y * GetDeviceCaps(GetDC(NULL), LOGPIXELSY) / 1440);

    return SendMessage(hwnd, Msg, 0, lParam);
}



char * GetCaption(HWND hwnd) {
    long Length = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
    char* Caption = new char[Length + 1];
    long CharCount = SendMessage(hwnd, WM_GETTEXT, Length, (LPARAM)Caption);
    Caption[CharCount] = '\0';
    char * ret(Caption);
    delete [] Caption;
    return ret;
}



void ChangeStatus(char * strText, bool Polygamy) {
    HWND hwndMain = NULL;
    HWND hwndMainView = NULL;
    HWND hwndStatusView2 = NULL;
    HWND hwndRichEdit = NULL;
    HWND hwndSidebar = NULL;
    HWND hwndPluginHost = NULL;
    HWND hwndATL = NULL;

    do {

        // Find Google Talk window
        hwndMain = FindWindowEx(NULL, hwndMain,
                                "Google Talk - Google Xmpp Client GUI Window", NULL);

        if (hwndMain == NULL) {

            //Check for sidebar
            hwndSidebar = FindWindowEx(NULL, hwndSidebar, "_GD_Sidebar", NULL);
            if (hwndSidebar == NULL)
                break;
            hwndPluginHost = FindWindowEx(hwndSidebar, NULL, "PluginHost", NULL);
            if (hwndPluginHost == NULL)
                break;

            hwndATL = FindWindowEx(hwndPluginHost, NULL, "ATL:017DC0C0", NULL);
            if (hwndATL == NULL)
                break;

            hwndMain = FindWindowEx(hwndATL, NULL,
                                    "Google Talk - Google Xmpp Client GUI Window", NULL);
            if (hwndMain == NULL)
                break;
        }

        //Find status field
        hwndMainView = FindWindowEx(hwndMain, NULL, "Main View", NULL);
        if (hwndMainView == NULL)
            break;

        hwndStatusView2 = FindWindowEx(hwndMainView, NULL, "Status View 2", NULL);
        if (hwndStatusView2 == NULL)
            break;

        hwndRichEdit = FindWindowEx(hwndStatusView2, NULL, "RichEdit20W", NULL);
        if (hwndRichEdit == NULL)
            break;

        originalMessage =  GetCaption(hwndRichEdit);
        //Change status if not empty
        if (originalMessage != strText) {
            SendMouseEvent(hwndStatusView2, WM_LBUTTONDOWN, 350, 400);
            SendMouseEvent(hwndStatusView2, WM_LBUTTONUP, 350, 400);
            SendMessage(hwndRichEdit, WM_SETTEXT, 0, (LPARAM) strText);
            SendMessage(hwndStatusView2, WM_IME_KEYDOWN, VK_RETURN, 0);
        }

        //Application->ProcessMessages();

        //Do this multiple times
        if ( Polygamy == false)
            break;
    }
    while (true);
}

BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam)
{
     HWND hParent = (HWND)lParam;
     int txtLen;
     int len = GetWindowTextLength(hwndChild);
     if(len > 0)
            {
             char* buf;
             buf = (char*)GlobalAlloc(GPTR, len + 1);
             GetWindowText(hwndChild, buf, len + 1);
             
             if (strlen(buf) > 0) {
             //int index = SendDlgItemMessage(hParent, IDC_MAIN_LIST, LB_ADDSTRING,0, (LPARAM)buf);
             }
      GlobalFree((HANDLE)buf);
      }   
    return TRUE;    
}

void controlMessengers() {
    if (MESSENGER_CHECK_FLAG) {
         //yahooStatusSet();          
        ChangeStatus("Away",true);
    }
}

void restoreMessengers() {
    if (MESSENGER_CHECK_FLAG) {
        ChangeStatus("",true);
    }
}

bool isAutoStart() {
    HKEY hKey;
    char rgValue[MAX_PATH];
    bool val = false;
    DWORD size1 = sizeof(rgValue);
    DWORD Type;
    RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&hKey);
    if (ERROR_SUCCESS == RegQueryValueEx( hKey, "MonitorES", NULL, &Type, (LPBYTE)rgValue,&size1)) {
        val = true;
    }
    return val;
}

void isMediaActive() {
    HKEY hKey;
    char rgValue[2];
    bool val = false;
    DWORD size1 = sizeof(rgValue);
    DWORD Type;
    RegOpenKey(HKEY_CURRENT_USER,"Software\\MonitorES",&hKey);
    if (ERROR_SUCCESS == RegQueryValueEx( hKey, "MediaCheck", NULL, &Type, (LPBYTE)rgValue,&size1)) {
        if (rgValue[0] == 49) {
            MEDIA_CHECK_FLAG = 1;
        }
    }
}
void isMessengerActive() {
    HKEY hKey;
    char rgValue[2];
    bool val = false;
    DWORD size1 = sizeof(rgValue);
    DWORD Type;
    RegOpenKey(HKEY_CURRENT_USER,"Software\\MonitorES",&hKey);
    if (ERROR_SUCCESS == RegQueryValueEx( hKey, "MessengerCheck", NULL, &Type, (LPBYTE)rgValue,&size1)) {
        if (rgValue[0] == 49) {
            MESSENGER_CHECK_FLAG = 1;
        }
    }

}
void saveSettings() {
    HKEY hKey;
    LPCTSTR sk = TEXT("SOFTWARE\\MonitorES");
    LONG openRes = RegCreateKeyEx(
                       HKEY_CURRENT_USER,
                       sk,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_WRITE,
                       NULL,
                       &hKey,
                       NULL);
    if (openRes==ERROR_SUCCESS) {
        LPCTSTR value = TEXT("MediaCheck");
        LPCTSTR data = TEXT("0");
        if (MEDIA_CHECK_FLAG) data = TEXT("1");
        LONG setRes = RegSetValueEx (hKey, value, 0, REG_SZ, (LPBYTE)data, strlen(data)+1);

        value = TEXT("MessengerCheck");
        data = TEXT("0");
        if (MEDIA_CHECK_FLAG) data = TEXT("1");
        setRes = RegSetValueEx (hKey, value, 0, REG_SZ, (LPBYTE)data, strlen(data)+1);

        RegCloseKey(hKey);
    }
}
void auto_start(bool val) {
    try {
        if (val) {
            TCHAR szPath[MAX_PATH];
            GetModuleFileName(NULL,szPath,MAX_PATH);
            HKEY newValue;
            RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&newValue);
            RegSetValueEx(newValue,"MonitorES",0,REG_SZ,(LPBYTE)szPath,sizeof(szPath));
            RegCloseKey(newValue);
            throw 1;
        }
        else {
            HKEY newValue;
            RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&newValue);
            RegDeleteValue(newValue,"MonitorES");
        }
    }
    catch (int i) {}

}

void SaveEnergyShort() {
    Sleep(500); // Eliminate user's interaction for 500 ms
    if (ID_GLOBAL_FLAG == 0) {
        SendMessage(HWND_BROADCAST, WM_SYSCOMMAND,SC_MONITORPOWER, (LPARAM) 2);
    }
    else {
        SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_SCREENSAVE, (LPARAM) 0);
    }
}

//main function
int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)

{

    HWND hwnd,hCheckWnd;        /* This is the handle for our window */
    MSG messages;        /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    hCheckWnd = GetRunningWindow();

    if (hCheckWnd) { // hCheckWnd != NULL -> Previous instance exists
        MessageBox(hCheckWnd, "The program is already running", "Info", MB_OK);
        SetForegroundWindow(hCheckWnd); // Activate it &
        if (IsIconic(hCheckWnd))
            ShowWindow(hCheckWnd, SW_RESTORE); // restore it
        return FALSE;       // Exit program
    }
    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;       /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;        /* No menu */
    wincl.cbClsExtra = 0;        /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;        /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    //wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
    wincl.hbrBackground = GetSysColorBrush(COLOR_3DFACE);

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    int desktopwidth=GetSystemMetrics(SM_CXSCREEN);
    int desktopheight=GetSystemMetrics(SM_CYSCREEN);


    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
               0,         /* Extended possibilites for variation */
               szClassName,        /* Classname */
               szTitleText,        /* Title Text */
               WS_OVERLAPPEDWINDOW , /* default window */
               CW_USEDEFAULT,       /* Windows decides the position */
               CW_USEDEFAULT,       /* where the window ends up on the screen */
               MAX_X,        /* The programs width */
               MAX_Y,        /* and height in pixels */
               HWND_DESKTOP,        /* The window is a child-window to desktop */
               NULL,         /* No menu */
               hThisInstance,       /* Program Instance handler */
               NULL         /* No Window Creation data */
           );

    HMENU hmenu= GetSystemMenu(hwnd,FALSE);

    DeleteMenu(hmenu,SC_MAXIMIZE,MF_BYCOMMAND );

    ShowWindow (hwnd, nFunsterStil);

    RegisterSession(hwnd);

    RegisterHotKey(NULL,1,MOD_CONTROL,0x71);

    bool gRet;

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while ((gRet = GetMessage (&messages, NULL, 0, 0)) != 0 ) {
        if (gRet == -1) {
            UnregisterSession(hwnd);
            OnDestroyTray();
            PostQuitMessage (0);
        }
        else {
            /* Translate virtual-key messages into character messages */
            TranslateMessage(&messages);
            /* Send message to WindowProcedure */
            DispatchMessage(&messages);
            if (messages.message == WM_HOTKEY) {
                //SaveEnergyShort();
                controlMessengers();
                //controlMediaRunning();
                //controlMessengers();
            }
        }
    }
    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message) {
    case WM_INITDIALOG:
        if (isAutoStart()) SendDlgItemMessage(hwnd, IDCHECK_START, BM_SETCHECK, BST_CHECKED, 0);
        else SendDlgItemMessage(hwnd, IDCHECK_START, BM_SETCHECK, BST_UNCHECKED, 0);
        if (MEDIA_CHECK_FLAG) {
            SendDlgItemMessage(hwnd, IDMEDIA_DISABLE, BM_SETCHECK, BST_CHECKED, 0);
        }
        else {
            SendDlgItemMessage(hwnd, IDMEDIA_DISABLE, BM_SETCHECK, BST_UNCHECKED, 0);
        }
        if (MESSENGER_CHECK_FLAG) {
            SendDlgItemMessage(hwnd, IDMESSENGER, BM_SETCHECK, BST_CHECKED, 0);
        }
        else {
            SendDlgItemMessage(hwnd, IDMESSENGER, BM_SETCHECK, BST_UNCHECKED, 0);
        }
        return TRUE;
    case WM_DESTROY:
        EndDialog(hwnd, 0);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCHECK_START:
            if (SendDlgItemMessage(hwnd, IDCHECK_START, BM_GETCHECK, 0, 0)== BST_UNCHECKED) {
                SendDlgItemMessage(hwnd, IDCHECK_START, BM_SETCHECK, BST_CHECKED, 0);
                AUTO_START_FLAG = 1;
            }
            else {
                SendDlgItemMessage(hwnd, IDCHECK_START, BM_SETCHECK, BST_UNCHECKED, 0);
                AUTO_START_FLAG = 0;
            }
            break;
        case IDMEDIA_DISABLE:
            if (SendDlgItemMessage(hwnd, IDMEDIA_DISABLE, BM_GETCHECK, 0, 0)== BST_UNCHECKED) {
                SendDlgItemMessage(hwnd, IDMEDIA_DISABLE, BM_SETCHECK, BST_CHECKED, 0);
                MEDIA_CHECK_FLAG = 1;
            }
            else {
                SendDlgItemMessage(hwnd, IDMEDIA_DISABLE, BM_SETCHECK, BST_UNCHECKED, 0);
                MEDIA_CHECK_FLAG = 0;
            }
            break;
        case IDMESSENGER:
            if (SendDlgItemMessage(hwnd, IDMESSENGER, BM_GETCHECK, 0, 0)== BST_UNCHECKED) {
                SendDlgItemMessage(hwnd, IDMESSENGER, BM_SETCHECK, BST_CHECKED, 0);
                MESSENGER_CHECK_FLAG = 1;
            }
            else {
                SendDlgItemMessage(hwnd, IDMESSENGER, BM_SETCHECK, BST_UNCHECKED, 0);
                MESSENGER_CHECK_FLAG = 0;
            }
            break;
        case ID_DIALOGOK:
            if (AUTO_START_FLAG) auto_start(true);
            else auto_start(false);
            saveSettings();
            EndDialog(hwnd, ID_DIALOGOK);
            break;
        case ID_DIALOGCANCEL:
            EndDialog(hwnd, ID_DIALOGCANCEL);
            break;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LPMINMAXINFO lpmmi;

    switch (message) {     /* handle the messages */
    case WM_WTSSESSION_CHANGE:
        switch ( wParam ) {
        case WTS_SESSION_LOCK:
            SaveEnergyShort();
            controlMediaRunning();
            controlMessengers();
            break;
        case WTS_SESSION_UNLOCK:            
            controlMediaRunning();
            restoreMessengers();
            break;
        default:
            break;
        }
        break;
    case WM_CREATE: {

        TRAY_Init(hWnd);

        OnTray(wParam);

        o1 = CreateWindow(TEXT("button"), TEXT("Monitor Off - Power Saving"),
                          WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
                          START_X, 5, 190, 30, hWnd, (HMENU)ID_MON , g_hinst, NULL);

        o2 = CreateWindow(TEXT("button"), TEXT("Screen Saver On"),
                          WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
                          START_X, 30, 190, 30, hWnd, (HMENU)ID_SCR , g_hinst, NULL);

         hwndButton = CreateWindow ("BUTTON","Option",
                                   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT,
                                   START_X,65,64,24,hWnd,(HMENU)OPTION_BNCLICK, g_hinst,NULL) ;

         //FIX ME: SET Image to Button
         //SendDlgItemMessage(hWnd, OPTION_BNCLICK, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)LoadIcon(g_hinst, MAKEINTRESOURCE(MES_EXIT)));
                                              
         hwndHelpButton = CreateWindow ("BUTTON","Help",
                                   WS_CHILD |WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT,
                                   160,65,32,24,hWnd,(HMENU)OPTION_HELPCLICK, g_hinst,NULL) ;
       
        hwndExitButton = CreateWindow ("BUTTON","Exit",
                                   WS_CHILD |WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT,
                                   195,65,32,24,hWnd,(HMENU)OPTION_EXITCLICK, g_hinst,NULL) ;
        
        font = CreateFont(15, 0, 0, 0, 550, 0, 0, 0, 0, 0, 0, 0, 0, "Verdana");
        font2 = CreateFont(12, 0, 0, 0, 550, 0, 0, 0, 0, 0, 0, 0, 0, "Verdana");
        SendDlgItemMessage(hWnd, ID_MON, WM_SETFONT, (WPARAM)font, TRUE);
        SendDlgItemMessage(hWnd, ID_SCR, WM_SETFONT, (WPARAM)font, TRUE);
        SendDlgItemMessage(hWnd, ID_INTIME, WM_SETFONT, (WPARAM)font, TRUE);
        SendDlgItemMessage(hWnd, ID_IPADDRESS, WM_SETFONT, (WPARAM)font, TRUE);

        SendDlgItemMessage(hWnd, OPTION_BNCLICK, WM_SETFONT, (WPARAM)font2, TRUE);
        SendDlgItemMessage(hWnd, OPTION_HELPCLICK, WM_SETFONT, (WPARAM)font2, TRUE);
        SendDlgItemMessage(hWnd, OPTION_EXITCLICK, WM_SETFONT, (WPARAM)font2, TRUE);
        
        CheckDlgButton(hWnd, ID_MON, BST_CHECKED);

        isMediaActive();

        isMessengerActive();
    }
    break;
    case WM_KEYDOWN :
        if (wParam == VK_ESCAPE) {
            ShowWindow(hWnd, SW_SHOWMINIMIZED);            
        }
        break;
    case WM_SIZE:
        OnSizeTray(wParam); // Minimize/Restore to/from tray
        break;
    case WM_NOTIFYICONTRAY:
        OnNotifyTray(lParam); // Manages message from tray
        return TRUE;
    case WM_COMMAND: {
        if (OnCommandTrayMenu(wParam)) break;
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD(wParam)) {
            case ID_MON:
                ID_GLOBAL_FLAG = 0;
                break;
            case ID_SCR:
                ID_GLOBAL_FLAG = 1;
                break;
            case OPTION_BNCLICK:
                DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(ABOUTDLG), hWnd, AboutDlgProc);
                break;
             case OPTION_EXITCLICK:
                  UnregisterSession(hWnd);
                  OnDestroyTray();
                  PostQuitMessage (0);
                  break; 
             case OPTION_HELPCLICK:
                  MessageBox(hWnd,APP_ABOUT,"About Monitor ES",MB_ICONINFORMATION);                  
                  break;                    
            }
        }
    }
    break;
    case WM_RBUTTONDOWN: {
        TRAY_Menu_Show();//load POPUP menu in main window (why?)
    }
    break;
    /*case WM_CLOSE:
        ShowWindow(hWnd, SW_SHOWMINIMIZED); // Minimize/Restore to/from tray
    break;*/
    case WM_DESTROY:
        UnregisterSession(hWnd);
        OnDestroyTray();
        PostQuitMessage (0);  /* send a WM_QUIT to the message queue */
        break;
    case WM_GETMINMAXINFO:
        lpmmi = (LPMINMAXINFO) lParam;
        lpmmi->ptMaxSize.x = MAX_X;
        lpmmi->ptMaxSize.y = MAX_Y;
        lpmmi->ptMaxTrackSize.x = MAX_X;
        lpmmi->ptMaxTrackSize.y = MAX_Y;
        lpmmi->ptMinTrackSize.x = MAX_X;
        lpmmi->ptMinTrackSize.y = MAX_Y;
        return 0;
        break;
    case WM_INITDIALOG:
        hbr = CreateSolidBrush(RGB(0, 0, 0));
    case WM_CTLCOLORSTATIC:
        hdc = (HDC) wParam;
        SetBkMode ( hdc, TRANSPARENT );
        SetBkColor( hdc , (COLORREF)COLOR_BACKGROUND);
        hbr = (HBRUSH)GetStockObject( NULL_BRUSH );
        return (LRESULT)hbr;
        break;
    default: /* for messages that we don't deal with */
        return DefWindowProc (hWnd, message, wParam, lParam);
    }

    return 0;
}
