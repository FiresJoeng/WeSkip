// 防止重复包含
#pragma once


// 包含所需头文件
#include <Windows.h>
#include <detours.h>


// 为导出函数定义宏，方便管理
#ifdef SPEEDHACK_EXPORTS
#define SPEEDHACK_API extern "C" __declspec(dllexport)
#else
#define SPEEDHACK_API extern "C" __declspec(dllimport)
#endif


// Speedhack 命名空间
namespace Speedhack
{
	extern double speed;
	extern bool initialised;

	// 函数指针类型定义
	typedef DWORD(WINAPI* _tGetTickCount)(void);
	extern _tGetTickCount _GTC;
	extern DWORD _GTC_BaseTime, _GTC_OffsetTime;

	typedef ULONGLONG(WINAPI* _tGetTickCount64)(void);
	extern _tGetTickCount64 _GTC64;
	extern ULONGLONG _GTC64_BaseTime, _GTC64_OffsetTime;

	typedef BOOL(WINAPI* _tQueryPerformanceCounter)(LARGE_INTEGER*);
	extern _tQueryPerformanceCounter _QPC;
	extern LARGE_INTEGER _QPC_BaseTime, _QPC_OffsetTime;

	// Hook 函数
	DWORD WINAPI _hGetTickCount();
	ULONGLONG WINAPI _hGetTickCount64();
	BOOL WINAPI _hQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);

	// 核心功能函数
	void Setup();
	void Detach();
	void SetSpeed(double relSpeed);
}


// 注入后在目标进程中调用的初始化函数
SPEEDHACK_API DWORD WINAPI RemoteThread_Initialize(LPVOID lpParameter);
// 在目标进程中调用的卸载函数
SPEEDHACK_API DWORD WINAPI RemoteThread_Shutdown(LPVOID lpParameter);
// Inject 函数
SPEEDHACK_API bool Inject(DWORD dwProcessId, double speed);
// Eject 函数
SPEEDHACK_API bool Eject(DWORD dwProcessId);
