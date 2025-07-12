// 定义和导入
#define SPEEDHACK_EXPORTS

#include "speedhack.h"
#include <psapi.h>
#include <string>


// DllMain 入口点
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // 当 DLL 被加载时，禁用线程库调用，可以防止某些情况下的死锁
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        // 当 DLL 被卸载时，确保我们的 hook 被正确移除
        if (Speedhack::initialised) {
            Speedhack::Detach();
        }
        break;
    }
    return TRUE;
}


// Speedhack 命名空间实现
namespace Speedhack
{
    // 全局变量定义
    double speed = 1.0;
    bool initialised = false;

	// 函数指针类型定义
    _tGetTickCount _GTC = nullptr;
    DWORD _GTC_BaseTime = 0, _GTC_OffsetTime = 0;

    _tGetTickCount64 _GTC64 = nullptr;
    ULONGLONG _GTC64_BaseTime = 0, _GTC64_OffsetTime = 0;

    _tQueryPerformanceCounter _QPC = nullptr;
    LARGE_INTEGER _QPC_BaseTime = LARGE_INTEGER(), _QPC_OffsetTime = LARGE_INTEGER();

	// 获取 GetTickCount 的 Hook 实现
    DWORD WINAPI _hGetTickCount()
    {
        return _GTC_OffsetTime + ((_GTC() - _GTC_BaseTime) * speed);
    }

	// 获取 GetTickCount64 的 Hook 实现
    ULONGLONG WINAPI _hGetTickCount64()
    {
        return _GTC64_OffsetTime + ((_GTC64() - _GTC64_BaseTime) * speed);
    }

	// 获取 QueryPerformanceCounter 的 Hook 实现
    BOOL WINAPI _hQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
    {
        LARGE_INTEGER x;
        _QPC(&x);
        lpPerformanceCount->QuadPart = _QPC_OffsetTime.QuadPart + ((x.QuadPart - _QPC_BaseTime.QuadPart) * speed);
        return TRUE;
    }

    // 初始化 Hook
    void Setup()
    {
        if (initialised) return;

        _GTC = &GetTickCount;
        _GTC64 = &GetTickCount64;
        _QPC = &QueryPerformanceCounter;

        _GTC_BaseTime = _GTC();
        _GTC_OffsetTime = _GTC_BaseTime;

        _GTC64_BaseTime = _GTC64();
        _GTC64_OffsetTime = _GTC64_BaseTime;

        _QPC(&_QPC_BaseTime);
        _QPC_OffsetTime.QuadPart = _QPC_BaseTime.QuadPart;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)_GTC, _hGetTickCount);
        DetourAttach(&(PVOID&)_GTC64, _hGetTickCount64);
        DetourAttach(&(PVOID&)_QPC, _hQueryPerformanceCounter);
        DetourTransactionCommit();

        initialised = true;
    }

    // 移除 Hook
    void Detach()
    {
        if (!initialised) return;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)_GTC, _hGetTickCount);
        DetourDetach(&(PVOID&)_GTC64, _hGetTickCount64);
        DetourDetach(&(PVOID&)_QPC, _hQueryPerformanceCounter);
        DetourTransactionCommit();

        initialised = false;
    }

    // 设置新的速度值
    void SetSpeed(double relSpeed)
    {
        if (initialised)
        {
            _GTC_OffsetTime = _hGetTickCount();
            _GTC_BaseTime = _GTC();

            _GTC64_OffsetTime = _hGetTickCount64();
            _GTC64_BaseTime = _GTC64();

            _hQueryPerformanceCounter(&_QPC_OffsetTime);
            _QPC(&_QPC_BaseTime);
        }
        speed = relSpeed;
    }
}


// 导出函数实现
SPEEDHACK_API DWORD WINAPI RemoteThread_Initialize(LPVOID lpParameter)
{
    double initial_speed = *(double*)lpParameter;
    Speedhack::Setup();
    Speedhack::SetSpeed(initial_speed);
    return 0;
}

SPEEDHACK_API DWORD WINAPI RemoteThread_Shutdown(LPVOID lpParameter)
{
    Speedhack::Detach();
    return 0;
}


// 内部辅助函数：获取本 DLL 的名称
std::string GetCurrentDllName() {
    char dllPath[MAX_PATH];
    HMODULE hModule = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&GetCurrentDllName, &hModule);
    GetModuleFileNameA(hModule, dllPath, sizeof(dllPath));
    std::string pathStr(dllPath);
    size_t lastSlash = pathStr.find_last_of("\\/");
    return pathStr.substr(lastSlash + 1);
}


// 内部辅助函数：在远程进程中获取模块句柄
HMODULE GetRemoteModuleHandle(DWORD dwProcessId, const char* moduleName) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
    if (!hProcess) {
        return NULL;
    }

    HMODULE hMods[1024];
    DWORD cbNeeded;
    HMODULE hModule = NULL;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            char szModName[MAX_PATH];
            if (GetModuleBaseNameA(hProcess, hMods[i], szModName, sizeof(szModName))) {
                if (_stricmp(szModName, moduleName) == 0) {
                    hModule = hMods[i];
                    break;
                }
            }
        }
    }

    CloseHandle(hProcess);
    return hModule;
}


// 核心注入函数实现 (按 PID)
SPEEDHACK_API bool Inject(DWORD dwProcessId, double speed)
{
    if (dwProcessId == 0) {
        return false;
    }

    char dllPath[MAX_PATH];
    std::string dllName = GetCurrentDllName();
    HMODULE hModule = GetModuleHandleA(dllName.c_str());
    if (hModule == NULL) return false;
    GetModuleFileNameA(hModule, dllPath, MAX_PATH);

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    if (hProcess == NULL) {
        return false;
    }

    LPVOID pRemoteDllPath = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteDllPath == NULL) {
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pRemoteDllPath, dllPath, sizeof(dllPath), NULL)) {
        VirtualFreeEx(hProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    LPVOID pLoadLibraryA = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    HANDLE hThreadLoad = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, pRemoteDllPath, 0, NULL);
    if (hThreadLoad == NULL) {
        VirtualFreeEx(hProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThreadLoad, INFINITE);
    CloseHandle(hThreadLoad);
    VirtualFreeEx(hProcess, pRemoteDllPath, 0, MEM_RELEASE);

    LPVOID pRemoteSpeed = VirtualAllocEx(hProcess, NULL, sizeof(double), MEM_COMMIT, PAGE_READWRITE);
    if (!WriteProcessMemory(hProcess, pRemoteSpeed, &speed, sizeof(double), NULL)) {
        CloseHandle(hProcess);
        return false;
    }

    LPVOID pRemoteInit = (LPVOID)GetProcAddress(hModule, "RemoteThread_Initialize");
    HANDLE hThreadInit = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteInit, pRemoteSpeed, 0, NULL);

    WaitForSingleObject(hThreadInit, INFINITE);
    CloseHandle(hThreadInit);
    VirtualFreeEx(hProcess, pRemoteSpeed, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return true;
}


// 核心取消注入函数实现
SPEEDHACK_API bool Eject(DWORD dwProcessId)
{
    if (dwProcessId == 0) {
        return false;
    }

    std::string dllName = GetCurrentDllName();
    HMODULE hRemoteModule = GetRemoteModuleHandle(dwProcessId, dllName.c_str());
    if (hRemoteModule == NULL) {
        return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    if (hProcess == NULL) {
        return false;
    }

    LPVOID pRemoteShutdown = (LPVOID)GetProcAddress(GetModuleHandleA(dllName.c_str()), "RemoteThread_Shutdown");
    HANDLE hThreadShutdown = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteShutdown, NULL, 0, NULL);
    if (hThreadShutdown) {
        WaitForSingleObject(hThreadShutdown, INFINITE);
        CloseHandle(hThreadShutdown);
    }

    LPVOID pFreeLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary");
    HANDLE hThreadEject = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFreeLibrary, hRemoteModule, 0, NULL);
    if (hThreadEject == NULL) {
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThreadEject, INFINITE);
    CloseHandle(hThreadEject);
    CloseHandle(hProcess);

    return true;
}
