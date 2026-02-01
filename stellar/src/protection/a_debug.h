#include"Windows.h"
#include "keyauth/skStr.h"
#include"filesystem"
namespace a_debug {

	inline bool dbgloop = true;



    inline void ErasePEHeaderFromMemory()
    {


        DWORD OldProtect = 0;

        
        char* pBaseAddr = (char*)GetModuleHandle(NULL);

        VirtualProtect(pBaseAddr, 4096,
            PAGE_READWRITE, &OldProtect);

        
        ZeroMemory(pBaseAddr, 4096);
    }

	inline BOOL CheckRemoteDebuggerPresentAPI()
	{


		BOOL IsDbgPresent;
		CheckRemoteDebuggerPresent(GetCurrentProcess(), &IsDbgPresent);
		return IsDbgPresent;
	}

	inline BOOL NtQueryInformationProcess_ProcessDbgPort()
	{

		typedef NTSTATUS(WINAPI* pNtQueryInformationProcess)(IN  HANDLE, IN  UINT, OUT PVOID, IN ULONG, OUT PULONG);

		
		const int ProcessDbgPort = 7;

		pNtQueryInformationProcess NtQueryInfoProcess = NULL;


		NTSTATUS Status;
		DWORD IsRemotePresent = 0;

		HMODULE hNtDll = LoadLibrary(TEXT("ntdll.dll"));
		if (hNtDll == NULL)
		{
			
		}

		NtQueryInfoProcess = (pNtQueryInformationProcess)GetProcAddress(hNtDll, skCrypt("NtQueryInformationProcess"));

		if (NtQueryInfoProcess == NULL)
		{
			
		}

		
		Status = NtQueryInfoProcess(GetCurrentProcess(), ProcessDbgPort, &IsRemotePresent, sizeof(unsigned long), NULL);
		if (Status == 0x00000000 && IsRemotePresent != 0)
			return TRUE;
		else
			return FALSE;
	}
	inline BOOL IsDebuggerPresentAPI()
	{
		

		return IsDebuggerPresent();
	}

	inline BOOL NtQueryInformationProcess_ProcessDebugObject()
	{

		typedef NTSTATUS(WINAPI* pNtQueryInformationProcess)(IN  HANDLE, IN  UINT, OUT PVOID, IN ULONG, OUT PULONG);

		
		const int ProcessDebugObjectHandle = 0x1e;

		
		pNtQueryInformationProcess NtQueryInfoProcess = NULL;

		
		NTSTATUS Status;
		HANDLE hDebugObject = NULL;

		HMODULE hNtDll = LoadLibrary(TEXT("ntdll.dll"));
		if (hNtDll == NULL)
		{
			
		}

		NtQueryInfoProcess = (pNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess");

		if (NtQueryInfoProcess == NULL)
		{
			
		}

		
		Status = NtQueryInfoProcess(GetCurrentProcess(), ProcessDebugObjectHandle, &hDebugObject, sizeof(DWORD), NULL);

		if (Status != 0x00000000)
			return false;

		if (hDebugObject)
			return true;
		else
			return false;
	}

	inline BOOL NtQueryInformationProcess_SystemKernelDebuggerInformation()
	{
		

		typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION {
			BOOLEAN DebuggerEnabled;
			BOOLEAN DebuggerNotPresent;
		} SYSTEM_KERNEL_DEBUGGER_INFORMATION, * PSYSTEM_KERNEL_DEBUGGER_INFORMATION;
		SYSTEM_KERNEL_DEBUGGER_INFORMATION Info;

		enum SYSTEM_INFORMATION_CLASS { SystemKernelDebuggerInformation = 35 };

		
		typedef NTSTATUS(WINAPI* pZwQuerySystemInformation)(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, IN OUT PVOID SystemInformation, IN ULONG SystemInformationLength, OUT PULONG ReturnLength);

		
		pZwQuerySystemInformation ZwQuerySystemInformation = NULL;

		
#define STATUS_SUCCESS   ((NTSTATUS)0x00000000L) 
		HANDLE hProcess = GetCurrentProcess();

		HMODULE hNtDll = LoadLibrary(TEXT("ntdll.dll"));
		if (hNtDll == NULL)
		{
			
		}

		ZwQuerySystemInformation = (pZwQuerySystemInformation)GetProcAddress(hNtDll, skCrypt("ZwQuerySystemInformation"));

		if (ZwQuerySystemInformation == NULL)
		{
			
		}

		// Time to finally make the call
		if (STATUS_SUCCESS == ZwQuerySystemInformation(SystemKernelDebuggerInformation, &Info, sizeof(Info), NULL)) {
			if (Info.DebuggerEnabled)
			{
				if (Info.DebuggerNotPresent)
					return FALSE;
				else
					return TRUE;
			}

		}
	}

	inline BOOL NtSetInformationThread_ThreadHideFromDebugger()
	{
		

		
		typedef NTSTATUS(WINAPI* pNtSetInformationThread)(IN HANDLE, IN UINT, IN PVOID, IN ULONG);

		
		const int ThreadHideFromDebugger = 0x11;

		
		pNtSetInformationThread NtSetInformationThread = NULL;

		
		NTSTATUS Status;
		BOOL IsBeingDebug = FALSE;

		HMODULE hNtDll = LoadLibrary(TEXT("ntdll.dll"));
		if (hNtDll == NULL)
		{
			
		}

		NtSetInformationThread = (pNtSetInformationThread)GetProcAddress(hNtDll, skCrypt("NtSetInformationThread"));

		if (NtSetInformationThread == NULL)
		{
		
		}

		
		Status = NtSetInformationThread(GetCurrentThread(), ThreadHideFromDebugger, NULL, 0);

		if (Status)
			IsBeingDebug = TRUE;

		return IsBeingDebug;
	}


	inline BOOL CloseHandleAPI()
	{
		

		__try {
			CloseHandle((HANDLE)0x99999999);
		}

		__except (STATUS_INVALID_HANDLE) {
			return TRUE;
		}


	}
	inline BOOLEAN HyperHideCheck()
	{
			HANDLE DriverHandle = CreateFileA(skCrypt("\\\\.\\HyperHideDrv"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (DriverHandle == INVALID_HANDLE_VALUE)
			return FALSE;
		return TRUE;
	}
	inline BOOLEAN TitanHideCheck()
	{
		HANDLE DriverHandle = CreateFileA(skCrypt("\\\\.\\TitanHide"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (DriverHandle == INVALID_HANDLE_VALUE)
			return FALSE;
		return TRUE;
	}
	inline BOOLEAN ProcessHackerCheck()
	{
		HANDLE DriverHandle = CreateFileA(skCrypt("\\\\.\\KProcessHacker3"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (DriverHandle == INVALID_HANDLE_VALUE)
			return FALSE;
		return TRUE;
	}
	inline BOOLEAN KsDumperCheck()
	{
		HANDLE DriverHandle = CreateFileA(skCrypt("\\\\.\\KsDumper"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (DriverHandle == INVALID_HANDLE_VALUE)
			return FALSE;
		return TRUE;
	}
	inline BOOLEAN HyperHideBinaryCheck() {

		if(std::filesystem::exists(skCrypt("C:\\Windows\\System32\\drivers\\HyperHideDrv.sys").decrypt())) {
			return true;
		}
		else {
			return false;

		}
	}
	inline BOOLEAN AirHVBinaryCheck(){
		if (std::filesystem::exists(skCrypt("C:\\Windows\\System32\\drivers\\airhv.sys").decrypt())) {
			return true;
		}
		else {
			return false;

		}
	}

	inline BOOLEAN TitanHideBinaryCheck() {
		if (std::filesystem::exists(skCrypt("C:\\Windows\\System32\\drivers\\TitanHide.sys").decrypt())) {
			return true;
		}
		else {
			return false;

		}
	}


	inline void dbg_all() {

		if (HyperHideCheck()) {
			exit(1);
		}
		if (HyperHideBinaryCheck()) {

			exit(1);
		}
		if (AirHVBinaryCheck()) {

			exit(1);
		}
		if (TitanHideCheck()) {

			exit(1);
		}
		if (TitanHideBinaryCheck()) {



			exit(1);
		}
		if (ProcessHackerCheck()) {

			exit(1);
		}
		if (KsDumperCheck()) {

			exit(1);
		}
		if (a_debug::CheckRemoteDebuggerPresentAPI()) {

			exit(1);
		}

		if (a_debug::NtQueryInformationProcess_ProcessDbgPort()) {

			exit(1);
		}

		if (a_debug::IsDebuggerPresentAPI()) {

			exit(1);
		}

		if (a_debug::NtQueryInformationProcess_ProcessDebugObject()) {

			exit(1);
		}

		if (a_debug::NtQueryInformationProcess_SystemKernelDebuggerInformation()) {

			exit(1);
		}


		if (a_debug::NtSetInformationThread_ThreadHideFromDebugger()) {

			exit(1);
		}

		if (a_debug::CloseHandleAPI()) {


			exit(1);
		}


	}


	inline void dbg_loop() {

		while (dbgloop) {

			if (a_debug::CheckRemoteDebuggerPresentAPI()) {
				
				exit(1);
			}

			if (a_debug::NtQueryInformationProcess_ProcessDbgPort()) {
				exit(1);
			}

			if (a_debug::IsDebuggerPresentAPI()) {
				exit(1);
			}

			if (a_debug::NtQueryInformationProcess_ProcessDebugObject()) {

				exit(1);
			}

			if (a_debug::NtQueryInformationProcess_SystemKernelDebuggerInformation()) {
				exit(1);
			}


			if (a_debug::NtSetInformationThread_ThreadHideFromDebugger()) {

				exit(1);
			}

			if (a_debug::CloseHandleAPI()) {

				exit(1);
			}
			Sleep(1000);
		}

	}
}