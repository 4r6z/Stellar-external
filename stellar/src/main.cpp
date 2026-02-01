#include "stellar/stellar.hpp"
#include "protection/protect_x/antiDbg.h"
#include "protection/a_debug.h"
#include "protection/keyauth/auth.hpp"
#include "protection/spoofer.h"
#include "protection/keyauth/utils.hpp"
#include "protection/integ.h"
#include <VMProtectSDK.h>
#include "protection/protect_x/main.h"
#include "Obsidium SDK/obsidium64.h"
#include <shellapi.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <ShlObj.h>

HWND fnt = NULL;
char title[100];
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);


using namespace std;

std::string RandomString(const int len)
{

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return tmp_s;
}

void NameChanger()
{
    std::string NAME = (std::string)(RandomString(30));
    SetConsoleTitleA(NAME.c_str());

}

DWORD ChangeName(LPVOID in)
{

    while (true)
    {
        NameChanger();
    }
}

__forceinline std::int32_t mempatch() {

    integrity::check check = integrity::check();


    while (true) {
        const std::vector<integrity::check::section>& sections = check.compare_checksums(check.retrieve_sections());

        if (sections.size() == std::size_t())
            std::printf("");

        for (const integrity::check::section& section : sections)
            system("exit");

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }


}

string GetFileName(const string& path) {

    char sep = '/';

#ifdef _WIN32
    sep = '\\';
#endif

    size_t i = path.rfind(sep, path.length());
    if (i != string::npos) {
        return(path.substr(i + 1, path.length() - i));
    }

    return("");
}

static const char alphanum[] = "0123456789667" "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int stringLengthh = sizeof(alphanum) - 1;

char genRandomn()
{
    return alphanum[rand() % stringLengthh];
}


void sex()
{
    srand(time(0));
    std::string Str;

    for (unsigned int i = 0; i < 30; ++i)
    {
        Str += genRandomn();

    }

    std::string rename = Str + (".exe");

    char filename[MAX_PATH];
    DWORD size = GetModuleFileNameA(NULL, filename, MAX_PATH);
    if (size)
        std::filesystem::rename(filename, rename);

}

void slow_print(const std::string& str, int delay_time)
{
    for (size_t i = 0; i != str.size(); ++i)
    {
        std::cout << str[i];
        Sleep(delay_time);
    }
}

bool CheckForTimingAnomalies() {
    VMProtectBegin("CheckForTimingAnomalies");
    auto start = std::chrono::high_resolution_clock::now();

    volatile int result = 0;
    for (int i = 0; i < 1000; ++i) {
        result += i * i;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    if (elapsed.count() > 1.0) {
        return true;
    }
    VMProtectEnd();
    return false;
}

std::wstring appdata_path() {
    wchar_t path[MAX_PATH];

    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
        return std::wstring(path);

    return L"";
}

void checkadmin() {

    bool admin = false;

    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    admin = fRet;

    if (!admin) {
        exit(0);
    }

}

//int main() {
//
//    OBSIDIUM_VM_START;
//    OBSIDIUM_ENC_START;
//    OBSIDIUM_LIC_START;
//
//    VMProtectBeginUltra("main");
//    mainprotect();
//    sex();
//
//    hide_thread(LI_FN(GetCurrentThread).forwarded_safe_cached()());
//    thread_hide_debugger();
//    hide_loader_thread();
//
//    anti_suspend();
//    check_processes();
//    obfuscator();
//    window_check();
//
//    ollydbg_exploit();
//    hardware_breakpoints();
//    hardware_register();
//    vmware_check();
//    virtual_box_drivers();
//    virtual_box_registry();
//
//    if (check12()) BlackList();
//    if (check2()) BlackList();
//    if (last_error()) BlackList();
//    if (close_handle()) BlackList();
//    if (thread_context()) BlackList();
//    if (anti_injection_vm()) BlackList();
//    if (remote_is_present()) BlackList();
//
//    if (a_debug::HyperHideCheck()) {
//
//        exit(1);
//    }
//    if (a_debug::HyperHideBinaryCheck()) {
//
//        exit(1);
//    }
//    if (a_debug::AirHVBinaryCheck()) {
//
//        exit(1);
//    }
//    if (a_debug::TitanHideCheck()) {
//
//        exit(1);
//    }
//    if (a_debug::TitanHideBinaryCheck()) {
//
//
//
//        exit(1);
//    }
//    if (a_debug::ProcessHackerCheck()) {
//
//
//        exit(1);
//    }
//    if (a_debug::KsDumperCheck()) {
//
//        exit(1);
//    }
//    if (a_debug::CheckRemoteDebuggerPresentAPI()) {
//
//        exit(1);
//    }
//
//    if (a_debug::NtQueryInformationProcess_ProcessDbgPort()) {
//
//        exit(1);
//    }
//
//    if (a_debug::IsDebuggerPresentAPI()) {
//
//        exit(1);
//    }
//
//    if (a_debug::NtQueryInformationProcess_ProcessDebugObject()) {
//
//        exit(1);
//    }
//
//    if (a_debug::NtQueryInformationProcess_SystemKernelDebuggerInformation()) {
//
//        exit(1);
//    }
//
//
//    if (a_debug::NtSetInformationThread_ThreadHideFromDebugger()) {
//
//        exit(1);
//    }
//
//    if (a_debug::CloseHandleAPI()) {
//
//
//        exit(1);
//    }
//
//    if (CheckForTimingAnomalies()) {
//        MessageBoxW(NULL, (L"Real"), (L"ERROR"), MB_OK | MB_ICONWARNING);
//        ShellExecuteW(0, 0, L"https://www.youtube.com/watch?v=dQw4w9WgXcQ", 0, 0, SW_SHOW);
//        exit(EXIT_FAILURE);
//    }
//    else {
//        SetConsoleTextAttribute(GetStdHandle, 0x04);
//
//        SPOOF_FUNC
//
//            CreateThread(NULL, NULL, ChangeName, NULL, NULL, NULL);
//
//
//
//        std::wstring appdata = appdata_path();
//        if (!std::filesystem::exists(appdata + L"\\stellar"))
//            std::filesystem::create_directory(appdata + L"\\stellar");
//
//        if (!std::filesystem::exists(appdata + L"\\stellar\\configs"))
//            std::filesystem::create_directory(appdata + L"\\stellar\\configs");
//        
//        stellar::init();
//
//        stellar::close();
//        OBSIDIUM_VM_END;
//        OBSIDIUM_ENC_END;
//        OBSIDIUM_LIC_END;
//        VMProtectEnd();
//
//        return 0;
//    }
//}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    system("mode con cols=90 lines=25");
    SetConsoleTextAttribute(hConsole, 7);
    system("Color 4");
    std::cout << R"(
                          .▄▄ · ▄▄▄▄▄▄▄▄ .▄▄▌  ▄▄▌   ▄▄▄· ▄▄▄  
                          ▐█ ▀. •██  ▀▄.▀·██•  ██•  ▐█ ▀█ ▀▄ █·
                          ▄▀▀▀█▄ ▐█.▪▐▀▀▪▄██▪  ██▪  ▄█▀▀█ ▐▀▀▄ 
                          ▐█▄▪▐█ ▐█▌·▐█▄▄▌▐█▌▐▌▐█▌▐▌▐█ ▪▐▌▐█•█▌
                           ▀▀▀▀  ▀▀▀  ▀▀▀ .▀▀▀ .▀▀▀  ▀  ▀ .▀  ▀  
                                           
)" << '\n';
    std::cout << ("                                   Stellar External Leak\n") << std::endl;
    Sleep(3000);
    NameChanger();
    sex();
    
    if (!stellar::init()) {
        std::cout << "Stellar initialization failed! Press any key to exit..." << std::endl;
        std::cin.get();
        return -1;
    }
    
    return 0;
}