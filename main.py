from ctypes import *
from fetch_pid import PID

dll = windll.LoadLibrary("HookDll.dll")

try:
    print(f'已查找到WeChatAppEx.exe: {PID}')
except Exception as e:
    print(f"未查找到WeChatAppEx.exe: {e}")

StartHook = getattr(dll, "StartHook")
setSpeed = getattr(dll, "setSpeed")
StopHook = getattr(dll, "StopHook")

def SPEEDx10():
    try:
        StartHook(c_ulong(PID))
        setSpeed(c_double(10.0))
    except Exception as e:
        print(f"错误: {e}")

def SPEEDx1():
    try:
        setSpeed(c_double(1))
        StopHook(c_ulong(PID))
    except Exception as e:
        print(f"错误: {e}")