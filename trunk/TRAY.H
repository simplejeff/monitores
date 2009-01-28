// tray.h

//About message
//Original Code by Massimiliano Miani

#define APP_ABOUT "Monitor Energy Saver \n\n\Monitor Energy Saver helps to turn off monitors or activate screensaver automatically when user locks workstation or laptop . It will help the environment ;) \n\n Coded by Umakanthan Chandran (cumakt@gmail.com) / Credits To : Massimiliano Miani"

//Other message
#define MY_MSG "Minimize this window in tray and click with \n\nleft button on icon to see a 'button effect'"

//Application icon, Tray icon and Popup menu bitmaps
#define APP_ICON       90 //app icon
#define TRAY_ICON      91 //tray icon
#define IDB_MENU_CHECK 92 //checked item bitmap (popup menu)
#define IDB_MENU_CLOSE 93 //close bitmap (popup menu)
#define IDB_MENU_ABOUT 94 //about bitmap (popup menu)

#define WM_NOTIFYICONTRAY (WM_APP + 1)//tray message

//Popup Menu items
#define IDM_MINIMIZE   421 							
#define IDM_TEXT_MINIMIZE " Minimize to tray"			
#define IDM_TEXT_RESTORE "  Show "

#define IDM_ABOUT	   422							
#define IDM_TEXT_ABOUT "  About "

#define IDM_CLOSE	   423						
#define IDM_TEXT_CLOSE "  Close "

#define IDM_ONTOP	   424						
#define IDM_TEXT_SCR " Enable ScreenSaver On"

#define IDM_ONTOP_2	   425						
#define IDM_TEXT_MON " Enable Monitor Off"


//Functions
void  TRAY_Init(HWND hwnd);             //Create tray and its popup menu 
BOOL  TRAY_IsInTray(void);              //Is window minimized in tray?
void  TRAY_Menu_Show(void);             //Show popup menu
void  OnNotifyTray(LPARAM lParam);      //Tray message (mouse clicks on tray icon)
BOOL  OnSizeTray(WPARAM wParam);        //WM_SIZE message
void  OnDestroyTray(void);              //WM_DESTROY message 
BOOL  OnCommandTrayMenu(WPARAM wParam); //Popup menu events
BOOL  OnTray(WPARAM wParam);

