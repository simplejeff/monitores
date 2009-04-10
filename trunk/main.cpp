/*
 VERSION 0.2 (beta) 
 
 CHANGELOG(0.2)
     * ADDED CLOSE EVENT TO SYSTEM TRAY
     * ADDED HOTKEY 
     * ADDED AUTOSTART OPTION
     * ADDED IN TIME CHECK
     * ADDED ONE INSTANCE RUN CHECK
     * ADDED ONECLICK SHOW WINDOW
     * ADDED CLICK TO SHOW/HIDE WINDOW
     * FIXED TRANSPARENCY BUGS
     * CODE CLEANUP
     
 CHANGELOG(0.1)
     * Initial Version    
               
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
#define ID_CHECK1 3
#define ID_TEXT 4
#define ID_INTIME 5
#define IS_EXIT 6
#define MAX_X 200
#define MAX_Y 120
#define START_X 15

HINSTANCE g_hinst;
HWND o1,o2,o3,t1,t2,b1;
HDC hdc;
HBRUSH hbr;

int ID_GLOBAL_FLAG = 0;
int ID_WEEKEND_FLAG = 0;
bool flagFirstTime = false;
char *LocalTimeHHMM;

bool GetLocalTimeHHMM(char *LocalTimeHHMM);

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
char szClassName[ ] = "MonitorES";
char szTitleText[ ] = "Monitor Energy Saver";

int (__stdcall * WorkStationStatus)();

/* Main Program call */ 

HWND GetRunningWindow()
{
 // Check if exists an application with the same class name as this application
 HWND hWnd = FindWindow(szClassName, NULL);
 if (IsWindow(hWnd))
 {
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
    if(pWTSUnRegisterSessionNotification) {
        pWTSUnRegisterSessionNotification(hwnd,NOTIFY_FOR_THIS_SESSION);
    }
    ::FreeLibrary(handle);     
     
}

void RegisterSession(HWND hwnd) {
    typedef DWORD (WINAPI *tWTSRegisterSessionNotification)( HWND,DWORD );

    tWTSRegisterSessionNotification pWTSRegisterSessionNotification=0;
    HINSTANCE handle = ::LoadLibrary("wtsapi32.dll");
    pWTSRegisterSessionNotification = (tWTSRegisterSessionNotification) :: GetProcAddress(handle,"WTSRegisterSessionNotification");
    if(pWTSRegisterSessionNotification) {
        pWTSRegisterSessionNotification(hwnd,NOTIFY_FOR_THIS_SESSION);
    }
    ::FreeLibrary(handle);         
}

bool isAutoStart(){
     HKEY hKey;
     char rgValue[1024];
     bool val = false;
     DWORD size1 = sizeof(rgValue);
     DWORD Type;
     RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&hKey);
     if(ERROR_SUCCESS == RegQueryValueEx( hKey, "MastekES", NULL, &Type, (LPBYTE)rgValue,&size1)) {
         val = true;             
     }
     return val;     
}

void auto_start(bool val){
try {
    if(val) {
        TCHAR szPath[MAX_PATH];
        GetModuleFileName(NULL,szPath,MAX_PATH);
        HKEY newValue;
        RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&newValue);
        RegSetValueEx(newValue,"MastekES",0,REG_SZ,(LPBYTE)szPath,sizeof(szPath));
        RegCloseKey(newValue);
        throw 1;
    } else {
        HKEY newValue;
        RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&newValue);
        RegDeleteValue(newValue,"MastekES");
    }    
  } catch(int i){}      
}


void alert(char *item){
  MessageBox(NULL, item,  "Message", MB_OK | MB_ICONINFORMATION);
} 

void SaveEnergyShort(){
  Sleep(500); // Eliminate user's interaction for 500 ms 
  if(ID_GLOBAL_FLAG == 0) {SendMessage(HWND_BROADCAST, WM_SYSCOMMAND,SC_MONITORPOWER, (LPARAM) 2); }
  else {SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_SCREENSAVE, (LPARAM) 0); }     
}   

//*-----------------------------------------------------------*/
bool GetLocalTimeHHMM(char *LocalTimeHHMM)
{
SYSTEMTIME lpSystemTime;
char *lpTimeFmt = "-> %02d:%02d <-";
char *bTime;

   	GetLocalTime(&lpSystemTime);
   	if ( (bTime = new char(9)) != NULL ) {
   		wsprintf(bTime, lpTimeFmt,
                        lpSystemTime.wHour,
                        lpSystemTime.wMinute
                        );
       	lstrcpy(LocalTimeHHMM, bTime);
   	    delete [] bTime;
   	    return TRUE;
   	}
    else
    	return FALSE;    
}

//main function 
int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)

{
                    
    HWND hwnd,hCheckWnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    hCheckWnd = GetRunningWindow();
    if (hCheckWnd) // hCheckWnd != NULL -> Previous instance exists
    {
       MessageBox(hCheckWnd, "The program is already running", "Info", MB_OK);
       SetForegroundWindow(hCheckWnd); // Activate it &
       if (IsIconic(hCheckWnd))
          ShowWindow(hCheckWnd, SW_RESTORE); // restore it
       return FALSE; // Exit program
    }
    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;
    
    int desktopwidth=GetSystemMetrics(SM_CXSCREEN);
    int desktopheight=GetSystemMetrics(SM_CYSCREEN);
    

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           szTitleText,         /* Title Text */
           WS_OVERLAPPEDWINDOW , /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           MAX_X,                 /* The programs width */
           MAX_Y,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );
           
    HMENU hmenu= GetSystemMenu(hwnd,FALSE);    

    //DeleteMenu(hmenu,SC_CLOSE,MF_BYCOMMAND );
    
    ShowWindow (hwnd, nFunsterStil);
    
    RegisterSession(hwnd);

    RegisterHotKey(NULL,1,MOD_CONTROL,0x71);
    
    bool gRet;
    /* Run the message loop. It will run until GetMessage() returns 0 */
    while ((gRet = GetMessage (&messages, NULL, 0, 0)) != 0 )
    {
        if(gRet == -1) {
            UnregisterSession(hwnd);
            OnDestroyTray();
            PostQuitMessage (0);
        } else { 
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
        if (messages.message == WM_HOTKEY) {
              SaveEnergyShort();        
        }
        } 
    }
                                                                                    
    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPMINMAXINFO lpmmi;
  
    switch (message)                  /* handle the messages */
    {
        case WM_WTSSESSION_CHANGE:
              switch( wParam )
              {
                   case WTS_SESSION_LOCK:                        
                         SYSTEMTIME st1;
                         GetLocalTime(&st1);
                         if(st1.wHour >= 11 && flagFirstTime ) {
                               flagFirstTime = false;
                         }
                         SaveEnergyShort(); 
                         break;
                   case WTS_SESSION_UNLOCK:
                         SYSTEMTIME st;
                         GetLocalTime(&st);
                         if(st.wHour >= 8 && st.wHour <= 10 && !flagFirstTime) {                                  
                              flagFirstTime = true;
                              LocalTimeHHMM = new char(9);
                              if ( GetLocalTimeHHMM(LocalTimeHHMM) == TRUE) {                                   
                                  SendMessage(t2,WM_SETTEXT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),(LPARAM) TEXT(LocalTimeHHMM));                                
                                  delete [] LocalTimeHHMM;
                              }
                         } 
                         break;
                   default: break;
              }
              break; 
       case WM_CREATE: {               
              
                TRAY_Init(hWnd);
                
                OnTray(wParam);

                o1 = CreateWindow(TEXT("button"), TEXT("Monitor(By TurnOff)"),
                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
                        START_X, 10, 150, 30, hWnd, (HMENU)ID_MON , g_hinst, NULL);             

                o2 = CreateWindow(TEXT("button"), TEXT("Screen Saver(By TurnOn)"),
                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
                        START_X, 35, 190, 30, hWnd, (HMENU)ID_SCR , g_hinst, NULL);            
       
                o3 = CreateWindow(TEXT("button"), TEXT("Auto Start"),
                        WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
                        125, 60, 70, 35, hWnd, (HMENU) ID_CHECK1, g_hinst, NULL);
    
                t1 = CreateWindow(TEXT("STATIC"), TEXT("Hot-Key -> CTRL + F2 "),
                        WS_CHILD | WS_VISIBLE ,
                        0, 70, 120, 25, hWnd, (HMENU) ID_TEXT, g_hinst, NULL);
   
                t2 = CreateWindow(TEXT("STATIC"),TEXT(""),
                        WS_CHILD | WS_VISIBLE ,
                        130, 0, 120, 25, hWnd, (HMENU) ID_INTIME, g_hinst, NULL);
                        
               /* b1 = CreateWindow(TEXT("button"), TEXT("Exit"),
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON ,
                        75, 80, 20, 15, hWnd, (HMENU)IS_EXIT , g_hinst, NULL);*/
  
                SendMessage(o1, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0); 
                SendMessage(o2, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0); 
                SendMessage(o3, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0); 
                SendMessage(t1, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
                SendMessage(t2, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
                //SendMessage(b1, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
                CheckDlgButton(hWnd, ID_MON, BST_CHECKED);             
                if(isAutoStart()){
                   SendMessage(o3,BM_SETCHECK,BST_CHECKED,0);        
                 }else {
                   SendMessage(o3,BM_SETCHECK,BST_UNCHECKED,0);   
                 }                
                }            
                break;
               
      	case WM_SIZE:
    	         OnSizeTray(wParam); // Minimize/Restore to/from tray
                 break;            
    	case WM_NOTIFYICONTRAY:
    	         OnNotifyTray(lParam); // Manages message from tray
                 return TRUE;
        case WM_COMMAND:
	    { 
          if (OnCommandTrayMenu(wParam)) break;                
          if (HIWORD(wParam) == BN_CLICKED) {
	       switch (LOWORD(wParam)) {
                  case ID_MON: ID_GLOBAL_FLAG = 0; break;
                  case ID_SCR: ID_GLOBAL_FLAG = 1; break;  
                  case ID_CHECK1: 
                      if (SendMessage(o3,BM_GETCHECK,0,0)== BST_UNCHECKED) {
			                   	SendMessage(o3,BM_SETCHECK,BST_CHECKED,0);
			                   	auto_start(true); 				
                      } else {
                        		SendMessage(o3,BM_SETCHECK,BST_UNCHECKED,0);
                        		auto_start(false);
                      } break;
                  }     
            } 
        }
        break;
        case WM_RBUTTONDOWN:{          
	        TRAY_Menu_Show();//load POPUP menu in main window (why?)
        }break;
        case WM_CLOSE:            
            ShowWindow(hWnd, SW_SHOWMINIMIZED); // Minimize/Restore to/from tray
            break;
        case WM_DESTROY:
            UnregisterSession(hWnd);
            OnDestroyTray();
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
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
        case WM_CTLCOLORSTATIC:
			hdc = (HDC) wParam;
			SetBkMode ( hdc, TRANSPARENT );
			//SetBkColor( hdc , (COLORREF) GetSysColor(COLOR_HIGHLIGHT));		
            SetTextColor( hdc, RGB(0, 0, 0));            
			hbr = (HBRUSH)GetStockObject( NULL_BRUSH );		
		    return (LRESULT)hbr;	
           	break;
        /*case WM_PAINT:
             hdc = BeginPaint(hWnd, &ps);
             if(flagFirstTime) { 
   	              TextOut(hdc, 0, 0, lpString, cbString);
             }     
             EndPaint(hWnd, &ps);
             break;*/
        default: /* for messages that we don't deal with */
            return DefWindowProc (hWnd, message, wParam, lParam);
    }

    return 0;
}


