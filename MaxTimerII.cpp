// MaxTime II Paltalk Timer Bot 
//
#include <windows.h>
#include <thread>
#include <iostream>
#include <atomic>
#include <condition_variable>

// Paltalk Windows Handles
HWND ghPtMain = NULL;
HWND ghMain = NULL;
//Quick Messagebox macros
char szAppName[] = "MaxTimerII";
#define msgba(h,x) MessageBoxA(h,x,szAppName,MB_OK)
// Quick Messagebox ANCI
#define msga(x) msgba(ghMain,x)

// Global variables 
const int iMsgLn = 256;
int giSec = 0;
int giItval = 5;
char gszMsg[iMsgLn] = { 0 };
char gszUserId[iMsgLn] = { 0 };
char gszLastUserId[iMsgLn] = { 0 };

std::atomic<bool> timerRunning(false);
std::condition_variable cv;
std::mutex cv_m;

// Forward Function Declarations
void TimerCallback(void);
void TimerThread(void);
void StartTimer(void);
void StopTimer(void);
// Paltalk related Functions
BOOL InitPaltalkWindows(void);
BOOL CopyPasteToPaltalk(char* szText);
// Paltalk Pocess Id
DWORD gdwPtProcId = 0;
// Paltalk path
const char* szPaltalkPath = "C:\\Program Files (x86)\\Paltalk\\Paltalk.exe";
// Paltalk Message Buffer
char gszPtMsg[MAX_PATH] = { 0 };

// This is called every time the giItval time passed
void TimerCallback()
{
    int iSec = 0;
    int iMin = 0;

    giSec += giItval;
    iMin = giSec / 60;
    iSec = giSec % 60;
    sprintf_s(gszPtMsg, MAX_PATH, "*** Mic is on for: %02d:%02d min. ***", iMin, iSec);
    CopyPasteToPaltalk(gszPtMsg);
    std::cout << "Timer running for: " << giSec << std::endl;
}

// This Timer Thread is independent from the main program thread
void TimerThread()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(cv_m);
        cv.wait_for(lock, std::chrono::seconds(giItval), [] { return !timerRunning.load(); });

        if (!timerRunning.load())
            break;

        TimerCallback();
    }
}

// Thread object id
std::thread timerThread;

// This is called when the debug string contains "STARTED"
void StartTimer()
{

    if (strcmp(gszUserId, gszLastUserId) != 0)
    {
        giSec = 0; // reset mic time
        sprintf_s(gszLastUserId, iMsgLn, "%s", gszUserId);
        sprintf_s(gszPtMsg, MAX_PATH, "*** Start Mic Timer ID: %s ***", gszUserId);
        CopyPasteToPaltalk(gszPtMsg);
        std::cout << "Starting Timer ID: " << gszUserId << "\n";
    }
    else
    {
        std::cout << "Timer Start on Same User ID: " << gszUserId << "\n";
    }

    if (!timerRunning.load())
    {
        timerRunning.store(true);
        timerThread = std::thread(TimerThread);
    }
}

// This is called when the debug string contains "STOPPED"
void StopTimer()
{

    std::cout << "Stopping Timer\n";
    if (timerRunning.load())
    {
        timerRunning.store(false);
        cv.notify_all();
        if (timerThread.joinable())
        {
            timerThread.join();
        }
    }
}

// This is the main application loop. It starts up Paltalk and attach the debuger.
// It loops until the program closed, and it closes Paltalk also
int main()
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    int iUserInput = 0;
    wchar_t szConsoleTitle[] = L"MaxTimer II.";

    // Rename the Console 
    SetConsoleTitle(szConsoleTitle);
    HWND hwConsole = GetConsoleWindow();
    if (hwConsole)
    {
        RECT rctConsol = {};
        GetWindowRect(hwConsole, &rctConsol);
        MoveWindow(hwConsole, rctConsol.left, rctConsol.top, 400, 600, TRUE);
    }
    else
    {
        std::cout << "ERROR: Could not get the Console handle!\n";
    }

    // Try to Start Paltalk

    CreateProcessA(szPaltalkPath, NULL, NULL, NULL, FALSE,
        NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
    // Save the Paltalk Process id in the clobal gdwPtProcId
    gdwPtProcId = pi.dwProcessId;

    std::cout << "Paltalk ProcH: " << pi.hProcess << "\nthreadH: " << pi.hThread << "\ndwProcId: " << pi.dwProcessId << "\n";

    std::cout << "\n When Paltalk is Running,\n Open the Room for Timming, \n If the Room is Not Open, Enter 2\n If it is Open, Enter 1 :";

    std::cin >> iUserInput;
    if (iUserInput != 1)
    {
        std::cout << "Start Paltalk and Try again!";
        return 2;
    }

    // Get the Room window handle
    if (!InitPaltalkWindows())
    {
        std::cout << " ERROR: Cannot find Paltalk Room!!! \n SHUTTING DOWN! \n";
        return 3;
    }

    std::cout << " Paltalk window handle: " << ghPtMain << "\n Attaching to Paltalk \n Entering debug loop\n";

    // Try to attach to Paltalk in DEBUG
    DebugActiveProcess(gdwPtProcId);

    // This is the debug loop
    DEBUG_EVENT debug_event = { 0 };
    for (;;)
    {
        if (!WaitForDebugEvent(&debug_event, INFINITE))
            return  4; // Did not Attached, Abort 
        // Debug Events switch 
        switch (debug_event.dwDebugEventCode)
        {
        case EXCEPTION_DEBUG_EVENT:
        {
            ContinueDebugEvent(debug_event.dwProcessId,
                debug_event.dwThreadId,
                DBG_EXCEPTION_NOT_HANDLED);
        }
        break;
        case CREATE_PROCESS_DEBUG_EVENT:
        {
            std::cout << "Debugger Attached. \n";
        }
        break;
        case CREATE_THREAD_DEBUG_EVENT:
        {
            // we do nothing here
        }
        break;
        case OUTPUT_DEBUG_STRING_EVENT:
        {
            OUTPUT_DEBUG_STRING_INFO& DebugString = debug_event.u.DebugString;

            WORD wDbStrLn = DebugString.nDebugStringLength;
            WORD wFunicode = DebugString.fUnicode;

            if (wFunicode == 0)
            {
                char* msg = new char[DebugString.nDebugStringLength];
                ZeroMemory(msg, DebugString.nDebugStringLength);
                ReadProcessMemory(pi.hProcess, DebugString.lpDebugStringData, msg, DebugString.nDebugStringLength, NULL);

                if (strstr(msg, "[TalkingNow]"))
                {
                    if (strstr(msg, "STARTED"))
                    {
                        char* pszUser = strstr(msg, "=") + 6;
                        sprintf_s(gszUserId, iMsgLn, "%s", pszUser);
                        StartTimer();
                    }
                    else if (strstr(msg, "STOPPED"))
                    {
                        StopTimer();
                    }

                }

                delete[]msg;
            }


        }
        break;

        default:

            break;
        }

        ContinueDebugEvent(debug_event.dwProcessId,
            debug_event.dwThreadId,
            DBG_CONTINUE);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

// Initialise the Paltalk Windows handles
BOOL InitPaltalkWindows(void)
{
    char szTitle[256] = { 0 };
    char szTemp[512] = { 0 };
    // Resetting handle
    ghPtMain = 0;

    ghPtMain = FindWindowW(L"Qt5150QWindowIcon", 0);

    if (!ghPtMain)
    {
        msga("There is No Paltalk Window!");
        return FALSE;
    }


    int iLen = GetWindowTextA(ghPtMain, szTitle, 254);
    if (iLen < 1)
    {
        msga("Error Getting Paltalk Room Title!");
        return FALSE;
    }

    // Getting and outputing Platalk room title
    std::cout << " Paltalk Room Title: " << szTitle << "\n";

    // Make the Paltalk Room window the HWND_TOPMOST 
    SetWindowPos(ghPtMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    return TRUE;
}

// Copy text and paste it to Paltalk
BOOL CopyPasteToPaltalk(char* szText)
{
    BOOL bRet = FALSE;
    size_t iSize = 0;
    INPUT ip;

    iSize = sizeof(szText) * strlen(szText) + 8;

    if (ghPtMain)
    {
        OpenClipboard(NULL);
        EmptyClipboard();
        HGLOBAL hGlb = GlobalAlloc(GMEM_MOVEABLE, iSize);
        if (!hGlb) return FALSE;
        LPWSTR lpwsClip = (LPWSTR)GlobalLock(hGlb);
        if (lpwsClip) memcpy(lpwsClip, szText, iSize);
        GlobalUnlock(hGlb);
        if (SetClipboardData(CF_TEXT, hGlb) == NULL)
        {
            GlobalFree(hGlb);
            EmptyClipboard();
            CloseClipboard();
            //Beep(900,200);
        }
        else
        {
            CloseClipboard();

            if (ghPtMain)
            {
                INPUT inpPaste[6] = {};
                ZeroMemory(inpPaste, sizeof(inpPaste));

                SetForegroundWindow(ghPtMain);

                // Control key down
                inpPaste[0].type = INPUT_KEYBOARD;
                inpPaste[0].ki.wVk = VK_CONTROL;
                inpPaste[0].ki.dwFlags = 0;

                //  "V" key down
                inpPaste[1].type = INPUT_KEYBOARD;
                inpPaste[1].ki.wVk = 0x56;
                inpPaste[1].ki.dwFlags = 0;
                // "V" key up
                inpPaste[3].type = INPUT_KEYBOARD;
                inpPaste[3].ki.wVk = 0x56;
                inpPaste[3].ki.dwFlags = KEYEVENTF_KEYUP;
                // Control key up
                inpPaste[4].type = INPUT_KEYBOARD;
                inpPaste[4].ki.wVk = VK_CONTROL;
                inpPaste[4].ki.dwFlags = KEYEVENTF_KEYUP;
                // Enter key down
                inpPaste[5].type = INPUT_KEYBOARD;
                inpPaste[5].ki.wVk = VK_RETURN;
                inpPaste[5].ki.dwFlags = 0;
                // Enter key up
                inpPaste[5].type = INPUT_KEYBOARD;
                inpPaste[5].ki.wVk = VK_RETURN;
                inpPaste[5].ki.dwFlags = 0;

                // Send these keys to Paltalk
                UINT uSent = SendInput(ARRAYSIZE(inpPaste), inpPaste, sizeof(INPUT));

                if (uSent != ARRAYSIZE(inpPaste))
                {
                    OutputDebugStringA("Error sending to Paltalk SendInput() failed!");
                }

            }

        }

    }

    return TRUE;
}