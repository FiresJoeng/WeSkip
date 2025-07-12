// speedhack.cpp
#include "speedhack.h"
#include "inject.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "inject.lib")


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
// 以下为“宿主进程”调用的导出函数，实现与 example.cpp 类似的注入/卸载接口
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
    GetModuleFileNameW(GetModuleHandleW(SPEEDHACK_DLL), dllPath, MAX_PATH);
    PathRemoveFileSpecW(dllPath);
    PathAppendW(dllPath, SPEEDHACK_DLL);

    // 2. 注入 DLL
    if (!InjectDll1(pid, dllPath)) {
        return -1;  // 注入失败
    }

    // 3. 远程调用 InjectSpeedhack(spd)
    //    这里假设 InjectDll1 会在注入后自动执行 DllMain，
    //    并且暴露了 InjectSpeedhack 导出函数，可用 CreateRemoteThread 调用。
    //    为简单起见，我们直接创建远程线程执行 InjectSpeedhack：
    HMODULE hMod = GetModuleHandleW(SPEEDHACK_DLL);
    FARPROC fn = GetProcAddress(hMod, "InjectSpeedhack");
    if (!fn) return -2;

    HANDLE hThread = CreateRemoteThread(
        OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid),
        nullptr, 0,
        (LPTHREAD_START_ROUTINE)fn,
        (LPVOID)&spd,
        0, nullptr
    );
    if (!hThread) return -3;
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    return 0;  // 成功
}

extern "C" __declspec(dllexport)
int SHUninitialize(DWORD pid)
{
    // 1. 远程调用 EjectSpeedhack()
    HMODULE hMod = GetModuleHandleW(SPEEDHACK_DLL);
    FARPROC fn = GetProcAddress(hMod, "EjectSpeedhack");
    if (!fn) return -1;

    HANDLE hThread = CreateRemoteThread(
        OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid),
        nullptr, 0,
        (LPTHREAD_START_ROUTINE)fn,
        nullptr,
        0, nullptr
    );
    if (!hThread) return -2;
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    // 2. 卸载 DLL
    if (!EnjectDll(pid, SPEEDHACK_DLL)) {
        return -3;
    }

    return 0;
}
