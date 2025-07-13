// speedhack.cpp
#include "speedhack.h"
#include "inject.h"
#include <Shlwapi.h>
#include <Psapi.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "inject.lib")
#pragma comment(lib, "Psapi.lib")


namespace Speedhack
{
    double speed = 1.0;
    bool initialised = false;

    _tGetTickCount    _GTC = nullptr;
    DWORD             _GTC_BaseTime = 0, _GTC_OffsetTime = 0;

    _tGetTickCount64  _GTC64 = nullptr;
    ULONGLONG         _GTC64_BaseTime = 0, _GTC64_OffsetTime = 0;

    _tQueryPerformanceCounter _QPC = nullptr;
    LARGE_INTEGER            _QPC_BaseTime = LARGE_INTEGER(), _QPC_OffsetTime = LARGE_INTEGER();

    DWORD WINAPI _hGetTickCount()
    {
        return _GTC_OffsetTime + DWORD(( _GTC() - _GTC_BaseTime ) * speed);
    }

    ULONGLONG WINAPI _hGetTickCount64()
    {
        return _GTC64_OffsetTime + ULONGLONG(( _GTC64() - _GTC64_BaseTime ) * speed);
    }

    BOOL WINAPI _hQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
    {
        LARGE_INTEGER now;
        _QPC(&now);
        lpPerformanceCount->QuadPart = _QPC_OffsetTime.QuadPart + LONGLONG(( now.QuadPart - _QPC_BaseTime.QuadPart ) * speed);
        return TRUE;
    }

    void Setup()
    {
        if (initialised) return;

        _GTC = &GetTickCount;
        _GTC_BaseTime = _GTC();
        _GTC_OffsetTime = _GTC_BaseTime;

        _GTC64 = &GetTickCount64;
        _GTC64_BaseTime = _GTC64();
        _GTC64_OffsetTime = _GTC64_BaseTime;

        _QPC = &QueryPerformanceCounter;
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

    void SetSpeed(double relSpeed)
    {
        if (initialised)
        {
            // 重新计算基准
            _GTC_OffsetTime    = _hGetTickCount();
            _GTC_BaseTime      = _GTC();

            _GTC64_OffsetTime  = _hGetTickCount64();
            _GTC64_BaseTime    = _GTC64();

            _hQueryPerformanceCounter(&_QPC_OffsetTime);
            _QPC(&_QPC_BaseTime);
        }
        speed = relSpeed;
    }
}

// ----------------------------------------------------------------------------
// 注入函数：在目标进程中被调用
// ----------------------------------------------------------------------------

extern "C" __declspec(dllexport)
DWORD WINAPI InjectSpeedhack(LPVOID lpParam)
{
    double* pSpeed = (double*)lpParam;
    if (pSpeed) {
        Speedhack::Setup();
        Speedhack::SetSpeed(*pSpeed);
    } else {
        Speedhack::Setup();
    }
    return 0;
}

extern "C" __declspec(dllexport)
DWORD WINAPI EjectSpeedhack(LPVOID lpParam)
{
    Speedhack::Detach();
    return 0;
}

// ----------------------------------------------------------------------------
// 以下为"宿主进程"调用的导出函数，实现注入/卸载接口
// ----------------------------------------------------------------------------

namespace {
    // speedhack.dll 文件名
    const wchar_t* SPEEDHACK_DLL = L"speedhack.dll";
}

extern "C" __declspec(dllexport)
int SHInitialize(DWORD pid, double spd)
{
    // 1. 构造 DLL 路径
    wchar_t dllPath[MAX_PATH] = { 0 };
    GetModuleFileNameW(GetModuleHandleW(nullptr), dllPath, MAX_PATH);
    PathRemoveFileSpecW(dllPath);
    PathAppendW(dllPath, SPEEDHACK_DLL);

    // 2. 注入 DLL
    if (!InjectDll1(pid, dllPath)) {
        return -1;  // 注入失败
    }

    // 3. 获取目标进程句柄
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return -2;

    // 4. 在目标进程中获取注入的DLL模块句柄
    HMODULE hRemoteMod = nullptr;
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            wchar_t szModName[MAX_PATH];
            if (GetModuleFileNameExW(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t))) {
                if (wcsstr(szModName, SPEEDHACK_DLL)) {
                    hRemoteMod = hMods[i];
                    break;
                }
            }
        }
    }
    
    if (!hRemoteMod) {
        CloseHandle(hProcess);
        return -3;
    }

    // 5. 获取InjectSpeedhack函数地址
    HMODULE hLocalMod = GetModuleHandleW(nullptr);
    FARPROC localFunc = GetProcAddress(hLocalMod, "InjectSpeedhack");
    if (!localFunc) {
        CloseHandle(hProcess);
        return -4;
    }
    
    // 计算远程函数地址
    FARPROC remoteFunc = (FARPROC)((BYTE*)hRemoteMod + ((BYTE*)localFunc - (BYTE*)hLocalMod));

    // 6. 在目标进程中分配内存存储speed参数
    LPVOID pRemoteSpeed = VirtualAllocEx(hProcess, nullptr, sizeof(double), MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteSpeed) {
        CloseHandle(hProcess);
        return -5;
    }
    
    if (!WriteProcessMemory(hProcess, pRemoteSpeed, &spd, sizeof(double), nullptr)) {
        VirtualFreeEx(hProcess, pRemoteSpeed, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return -6;
    }

    // 7. 创建远程线程执行InjectSpeedhack
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)remoteFunc, pRemoteSpeed, 0, nullptr);
    if (!hThread) {
        VirtualFreeEx(hProcess, pRemoteSpeed, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return -7;
    }
    
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteSpeed, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return 0;  // 成功
}

extern "C" __declspec(dllexport)
int SHUninitialize(DWORD pid)
{
    // 1. 获取目标进程句柄
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return -1;

    // 2. 在目标进程中获取注入的DLL模块句柄
    HMODULE hRemoteMod = nullptr;
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            wchar_t szModName[MAX_PATH];
            if (GetModuleFileNameExW(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t))) {
                if (wcsstr(szModName, SPEEDHACK_DLL)) {
                    hRemoteMod = hMods[i];
                    break;
                }
            }
        }
    }
    
    if (!hRemoteMod) {
        CloseHandle(hProcess);
        return -2;
    }

    // 3. 获取EjectSpeedhack函数地址
    HMODULE hLocalMod = GetModuleHandleW(nullptr);
    FARPROC localFunc = GetProcAddress(hLocalMod, "EjectSpeedhack");
    if (!localFunc) {
        CloseHandle(hProcess);
        return -3;
    }
    
    // 计算远程函数地址
    FARPROC remoteFunc = (FARPROC)((BYTE*)hRemoteMod + ((BYTE*)localFunc - (BYTE*)hLocalMod));

    // 4. 创建远程线程执行EjectSpeedhack
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)remoteFunc, nullptr, 0, nullptr);
    if (!hThread) {
        CloseHandle(hProcess);
        return -4;
    }
    
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    // 5. 卸载 DLL
    if (!EnjectDll(pid, SPEEDHACK_DLL)) {
        return -5;
    }

    return 0;
}
