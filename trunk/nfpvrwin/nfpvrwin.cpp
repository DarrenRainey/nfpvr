#define _WIN32_IE 0x0600
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <powrprof.h>
#include "nfpvrwin.h"
#include "resource.h"

#include "NfpvrLib.h"
#include "NfpvrwinInterface.h"
#include "XBrowseForFolder.h"

static const wchar_t* nfpvrwinVersion = L"0.0.2";
static const wchar_t* nfpvrwinWebpage = L"http://nfpvr.sourceforge.net/";

static const wchar_t* nfwinClassName = L"nfwinClass";
static const int NFPVR_DEFAULT_PORT = 50000;

static HANDLE hIconNfwin = 0;
static HANDLE hIconNfwinRecord = 0;
static HANDLE hIconCurrent = 0;

void nfwinRemoveSystemTrayIcon(HWND hWnd);
void nfwinModifySystemTrayIcon(HWND hWnd, HANDLE hIcon);
void nfwinModifySystemTrayTip(HWND hWnd, const wchar_t* tip);
void nfwinUpdateTip(HWND hWnd);

NfpvrwinInterface nfpvrwinInterface;

enum NfpvrwinShutdownState
{
	ShutdownStateDisabled  = 0,
	ShutdownStateHibernate = 1,
	ShutdownStateReboot    = 2,
	ShutdownStateShutdown  = 3,
	ShutdownStateStandby   = 4
} nfpvrwinShutdownState = ShutdownStateDisabled;

DWORD WINAPI nfpvrThreadProc(LPVOID param)
{
	HWND hWnd = (HWND)param;
	nfpvrwinInterface.setWindow(hWnd);
	nfwinUpdateTip(hWnd);
	NfpvrLibReadFromUdp(nfpvrwinInterface, NFPVR_DEFAULT_PORT);
	return 0;
}

void nfwinRemoveFromStartup()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey)==ERROR_SUCCESS)
	{
		RegDeleteValue(hKey, L"nfpvrwin");
	}
}

bool nfwinGetStartup()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey)==ERROR_SUCCESS)
	{
		DWORD type;
		if (RegQueryValueEx(hKey, L"nfpvrwin", 0, &type, 0, 0) == ERROR_SUCCESS)
			return true;
	}
	return false;
}

void nfwinAddToStartup()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey)==ERROR_SUCCESS)
	{
		wchar_t filename[1024];
		GetModuleFileName(NULL, filename, sizeof(filename)/sizeof(wchar_t));
		DWORD size = (DWORD)((wcslen(filename)+1)*sizeof(wchar_t));
		RegSetValueEx(hKey, L"nfpvrwin", 0, REG_EXPAND_SZ, 
			(LPBYTE)filename, size);
	}
}

void nfwinWriteConfiguration()
{
	HKEY hKey;
	RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\nfpvrwin", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);

	DWORD size = 0;
	const wchar_t* outputDirectory = nfpvrwinInterface.getOutputDirectory();

	if (outputDirectory)
	{
		size = (DWORD)(wcslen(outputDirectory)+1)*sizeof(wchar_t);
		RegSetValueEx(hKey, L"OutputDirectory", 0, REG_EXPAND_SZ, (LPBYTE)outputDirectory, size);
	}

	DWORD autoShutdown = (DWORD)nfpvrwinShutdownState;
	size = sizeof(autoShutdown);
	RegSetValueEx(hKey, L"AutoShutdown", 0, REG_DWORD, (LPBYTE)&autoShutdown, size);
}

void nfwinReadConfiguration()
{
	HKEY hKey;
	RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\nfpvrwin", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);

	DWORD size = 0;
	wchar_t outputDirectory[1024];
	outputDirectory[0] = 0;

	size = sizeof(outputDirectory);
	if (RegQueryValueEx(hKey, L"OutputDirectory", 0, NULL, (LPBYTE)outputDirectory, &size) == ERROR_SUCCESS)
	{
		nfpvrwinInterface.setOutputDirectory(outputDirectory);
	}

	DWORD autoShutdown = 0;
	size = sizeof(autoShutdown);
	if (RegQueryValueEx(hKey, L"AutoShutdown", 0, NULL, (LPBYTE)&autoShutdown, &size) == ERROR_SUCCESS)
	{
		nfpvrwinShutdownState = (NfpvrwinShutdownState)autoShutdown;
	}
}

void nfwinUpdateMenuOutputDirectory(HMENU hMenu)
{
	MENUITEMINFO info;
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_STRING;
	info.fType = MIIM_STRING;

	const wchar_t* outputDirectory = nfpvrwinInterface.getOutputDirectory();
	wchar_t string[1024];
	info.dwTypeData = string;

	if (*outputDirectory)
		swprintf(string, 1024, L"Currently: %s", outputDirectory);
	else
		swprintf(string, 1024, L"<directory not set>");

	SetMenuItemInfo(hMenu, ID_OUTPUTDIRECTORY_CURRENTLY, false, &info);
}

void nfwinPerformShutdown(NfpvrwinShutdownState state)
{
	switch(state)
	{
	case ShutdownStateHibernate:
		SetSuspendState(TRUE, FALSE, FALSE);
		break;

	case ShutdownStateReboot:
		ExitWindowsEx(EWX_REBOOT, 0);
		break;

	case ShutdownStateShutdown:
		ExitWindowsEx(EWX_POWEROFF, 0);
		break;
	
	case ShutdownStateStandby:
		SetSuspendState(FALSE, FALSE, FALSE);
		break;
	}
}

void nfwinUpdateShutdownOption(HMENU hMenu)
{
	MENUITEMINFO info;
	info.cbSize = sizeof(MENUITEMINFO );
	info.fMask = MIIM_STATE|MIIM_FTYPE;
	info.fType = MFT_RADIOCHECK;
	
	info.fState = (nfpvrwinShutdownState==ShutdownStateDisabled)?MFS_CHECKED:MFS_UNCHECKED;
	SetMenuItemInfo(hMenu, ID_SHUTDOWN_DISABLED, false, &info);
	
	info.fState = (nfpvrwinShutdownState==ShutdownStateHibernate)?MFS_CHECKED:MFS_UNCHECKED;
	SetMenuItemInfo(hMenu, ID_SHUTDOWN_HIBERNATE, false, &info);

	info.fState = (nfpvrwinShutdownState==ShutdownStateReboot)?MFS_CHECKED:MFS_UNCHECKED;
	SetMenuItemInfo(hMenu, ID_SHUTDOWN_REBOOT, false, &info);

	info.fState = (nfpvrwinShutdownState==ShutdownStateShutdown)?MFS_CHECKED:MFS_UNCHECKED;
	SetMenuItemInfo(hMenu, ID_SHUTDOWN_SHUTDOWN, false, &info);

	info.fState = (nfpvrwinShutdownState==ShutdownStateStandby)?MFS_CHECKED:MFS_UNCHECKED;
	SetMenuItemInfo(hMenu, ID_SHUTDOWN_STANDBY, false, &info);
}

BOOL nfwinGetItemIdListFromPath(const wchar_t* lpszPath, LPITEMIDLIST *lpItemIdList)
{
   LPSHELLFOLDER pShellFolder = NULL;
   HRESULT       hr;
   ULONG         chUsed;

   // Get desktop IShellFolder interface
   if (SHGetDesktopFolder (&pShellFolder) != NOERROR)
      return FALSE;     // failed

   // convert the path to an ITEMIDLIST
   hr = pShellFolder->ParseDisplayName (
                  NULL,           // owner window
                  NULL,           // reserved (must be NULL)
                  (LPOLESTR)(lpszPath),       // folder name
                  &chUsed,    // number of chars parsed
                  lpItemIdList,   // ITEMIDLIST
                  NULL            // attributes (can be NULL)
               );
      
   if (FAILED(hr))
   {
      pShellFolder->Release();
      *lpItemIdList = NULL;
      return FALSE;
   }
   
   pShellFolder->Release();
   return TRUE;
} // GetItemIdListFromPath

void nfwinUpdateStartupOption(HMENU hMenu)
{
	bool startup = nfwinGetStartup();

	MENUITEMINFO info;
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_STATE|MIIM_FTYPE;
	
	info.fState = (startup)?MFS_CHECKED:MFS_UNCHECKED;
	info.fType = MFT_RADIOCHECK;
	SetMenuItemInfo(hMenu, ID_STARTUP_START, false, &info);

	info.fState = (!startup)?MFS_CHECKED:MFS_UNCHECKED;
	info.fType = MFT_RADIOCHECK;
	SetMenuItemInfo(hMenu, ID_STARTUP_DONT, false, &info);
}

void nfwinShowPopup(HWND hWnd)
{
	HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU_POPUP));
	HMENU hMenuPopup = GetSubMenu(hMenu, 0);

	nfwinUpdateMenuOutputDirectory(hMenuPopup);
	nfwinUpdateShutdownOption(hMenuPopup);
	nfwinUpdateStartupOption(hMenuPopup);

	POINT point;
	GetCursorPos(&point);
	int item = TrackPopupMenuEx(hMenuPopup, TPM_RIGHTALIGN|TPM_RETURNCMD,
		point.x, point.y, hWnd, NULL);
	
	switch(item)
	{
	case ID_OUTPUTDIRECTORY_BROWSE:
		{
			// holy mombo jumbo, the windows api is so fucked up
			const wchar_t* outputDirectory = nfpvrwinInterface.getOutputDirectory();
			wchar_t newOutputDirectory[1024];
			if (XBrowseForFolder(hWnd, outputDirectory, newOutputDirectory, 1024))
				nfpvrwinInterface.setOutputDirectory(newOutputDirectory);
		}
		break;

	case ID_OUTPUTDIRECTORY_OPEN:
		{
			const wchar_t* outputDirectory = nfpvrwinInterface.getOutputDirectory();
			
			if (outputDirectory)
			{
				ShellExecute(NULL, L"open", outputDirectory, NULL, NULL, SW_SHOWNORMAL);
			}
			else
			{
				wchar_t localDirectory[1024];
				::GetCurrentDirectory(sizeof(localDirectory)/sizeof(wchar_t)-1, 
						localDirectory);
				ShellExecute(NULL, L"open", localDirectory, NULL, NULL, SW_SHOWNORMAL);
			}
		}
		break;

	case ID_ABOUT:
		{
			wchar_t text[1024];
			swprintf(text, 1024,
				L"nfpvrwin %s by mrblack1134.\n"
				L"\n"
				L"Thanks to y'all except F2ATV's retarded fucknut CSPO.\n"
				L"A reminder that 'testing' ain't watching stolen porn channels;\n"
				L"it's coding, snooping and developing shit like this.\n", nfpvrwinVersion);

			MessageBox(hWnd,
				text,
				L"About nfpvrwin",
				MB_OK);
		}
		break;

	case ID_STARTUP_START:
		nfwinAddToStartup();
		break;

	case ID_STARTUP_DONT:
		nfwinRemoveFromStartup();
		break;

	case ID_WEBPAGE:
		ShellExecute(NULL, L"open", nfpvrwinWebpage, NULL, NULL, SW_SHOWNORMAL);
		break;

	case ID_SHUTDOWN_DISABLED: 
		nfpvrwinShutdownState = ShutdownStateDisabled;
		break;

	case ID_SHUTDOWN_HIBERNATE: 
		nfpvrwinShutdownState = ShutdownStateHibernate;
		break;

	case ID_SHUTDOWN_REBOOT: 
		nfpvrwinShutdownState = ShutdownStateReboot;
		break;

	case ID_SHUTDOWN_SHUTDOWN: 
		nfpvrwinShutdownState = ShutdownStateShutdown;
		break;

	case ID_SHUTDOWN_STANDBY: 
		nfpvrwinShutdownState = ShutdownStateStandby;
		break;

	case ID_EXIT:
		{
			nfwinWriteConfiguration();
			nfwinRemoveSystemTrayIcon(hWnd);
			PostQuitMessage(0);
		}
		break;
	}
}

LRESULT CALLBACK nfwinWndProc(HWND hWnd, 
							  UINT message, 
							  WPARAM wParam, 
							  LPARAM lParam)
{
	switch (message)
	{
	case WM_TIMER:
		{
			const int timerId = (int)wParam;
			switch (timerId)
			{
			case 1:
				nfwinModifySystemTrayIcon(hWnd, (hIconCurrent == hIconNfwin)?(hIconNfwinRecord):(hIconNfwin));
				break;

			case 2:
				nfwinUpdateTip(hWnd);
				break;
			}
		}
		break;

	case WM_NFWIN_START_RECORD:
		SetTimer(hWnd, 1, 1000, NULL);
		SetTimer(hWnd, 2, 1000, NULL);
		nfwinUpdateTip(hWnd);
		break;

	case WM_NFWIN_STOP_RECORD:
		nfwinModifySystemTrayIcon(hWnd, hIconNfwin);
		KillTimer(hWnd, 1);
		KillTimer(hWnd, 2);
		nfwinUpdateTip(hWnd);
		nfwinPerformShutdown(nfpvrwinShutdownState);
		break;

	case WM_NFWIN_MESSAGE:
		{
			switch (lParam)
			{
			case WM_LBUTTONDOWN:
				//nfwinShowTooltip(hWnd, L"Starting to record on RDS (#123) at 8:30AM from 192.168.0.1:1234.");
				break;

			case WM_RBUTTONDOWN:
				nfwinShowPopup(hWnd);
				break;
			}
		}
		break;

	case WM_CREATE:
		{
			CreateThread(NULL, NULL, nfpvrThreadProc, (void*)hWnd, NULL, NULL);
			SetTimer(hWnd, 2, 1000, NULL);
		}		
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND nfwinCreateWindow()
{
	WNDCLASS wc = {0};

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = nfwinWndProc;
	wc.lpszClassName = nfwinClassName;
	wc.hCursor = NULL;
	wc.hIcon = NULL;
	
	if (RegisterClass(&wc))
	{
		HWND hWnd = CreateWindow(nfwinClassName, L"",
			WS_OVERLAPPEDWINDOW,
			0,0,0,0,
			HWND_MESSAGE, 
			NULL,
			GetModuleHandle(NULL),
			NULL);

		if (hWnd)
			return hWnd;
	}

	return 0;
}

void nfwinFormatNumber(wchar_t* output, const int number)
{
	char temp[16], *input=temp;
	itoa(number, temp, 10);

	size_t length = strlen(temp);
	size_t n = 3-(length%3);
	
	while (length--)
	{
		*output++ = (wchar_t)*input++;

		n++;
		n%=3;

		if (!n && length)
			*output++ = ',';
	}

	*output = '\0';
}

void nfwinUpdateTip(HWND hWnd)
{
	int handlerCount = 0;
	uint64 total = 0;

	handlerCount = nfpvrwinInterface.getHandlerCount();
	total = nfpvrwinInterface.getTotalReceived();

#define KB (1024)
#define MB (1024*KB)
#define GB (1024*MB)

	wchar_t recording[32];
	if (handlerCount)
		swprintf(recording, 32, L"%d recording(s) in progress.", handlerCount);
	else
		swprintf(recording, 32, L"No recording in progress.");

	wchar_t data[32];
	if (total>MB)
	{
		wchar_t number[16];
		nfwinFormatNumber(number, static_cast<int>(total/MB));
		swprintf(data, 32, L"%s MB transferred.", number);
	}
	else if (total>KB)
	{
		wchar_t number[16];
		nfwinFormatNumber(number, static_cast<int>(total/KB));
		swprintf(data, 32, L"%s KB transferred.", number);
	}
	else
	{
		swprintf(data, 32, L"%d bytes transferred.", static_cast<int>(total));
	}

	wchar_t tip[256];
	swprintf(tip, 256, L"nfpvrwin version %s\n%s\n%s",
		nfpvrwinVersion,
		recording,
		data);

	nfwinModifySystemTrayTip(hWnd, tip);
}

bool nfwinCreateSystemTray(HWND hWnd)
{
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = 0;
	nid.uFlags = NIF_ICON|NIF_MESSAGE;

	nid.hIcon = (HICON) hIconNfwin;
    nid.uCallbackMessage = WM_NFWIN_MESSAGE;

	return Shell_NotifyIcon(NIM_ADD, &nid)!=0;
}

void nfwinRemoveSystemTrayIcon(HWND hWnd)
{
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = 0;
	nid.uFlags = NIF_ICON;

	Shell_NotifyIcon(NIM_DELETE, &nid);
}

void nfwinModifySystemTrayIcon(HWND hWnd, HANDLE hIcon)
{
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = 0;
	nid.uFlags = NIF_ICON;

	nid.hIcon = (HICON) hIcon;
	hIconCurrent = hIcon;
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void nfwinModifySystemTrayTip(HWND hWnd, const wchar_t* tip)
{
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = 0;
	nid.uFlags = NIF_TIP;

	wcscpy_s(nid.szTip, wcslen(tip)+1, tip);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

bool nfwinGetShutdownPrivilege()
{
   HANDLE hToken; 
   TOKEN_PRIVILEGES tkp; 
 
   // Get a token for this process. 
   if (!OpenProcessToken(GetCurrentProcess(),
	   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	   return false;
 
   // Get the LUID for the shutdown privilege. 
   LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
        &tkp.Privileges[0].Luid); 
 
   tkp.PrivilegeCount = 1;  // one privilege to set    
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
   // Get the shutdown privilege for this process. 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES)NULL, 0); 
 
   if (GetLastError() != ERROR_SUCCESS) 
      return false; 
 
   return true;
}

void nfwinLoadIcons()
{
	hIconNfwin       = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_NFWIN), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);
	hIconNfwinRecord = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_NFWIN_RECORD), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);
}

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	bool canShutdown = nfwinGetShutdownPrivilege();
	nfwinReadConfiguration();
	nfwinLoadIcons();
	HWND hWnd = nfwinCreateWindow();
	bool ok = nfwinCreateSystemTray(hWnd);
	
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
		DispatchMessage(&msg);

	return 0;
}

