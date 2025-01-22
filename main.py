from ctypes import *
from fetch_pid import PID

# 加载动态链接库
dll = windll.LoadLibrary("HookDll.dll")

# 获取 DLL 中的函数
StartHook = getattr(dll, "StartHook")
setSpeed = getattr(dll, "setSpeed")
StopHook = getattr(dll, "StopHook")

def SPEEDx10():
    """
    将指定进程变速为 10 倍速
    """
    try:
        print(f"Setting process {PID} to 10x speed.")
        StartHook(c_ulong(PID))  # 启用变速
        setSpeed(c_double(10.0))  # 设置为 10 倍速
        print(f"Process {PID} is now running at 10x speed.")
    except Exception as e:
        print(f"Failed to set process {PID} to 10x speed: {e}")

def SPEEDx1():
    """
    将指定进程变速恢复为 1 倍速或停止变速
    """
    try:
        print(f"Stopping speed modification for process {PID}.")
        StopHook(c_ulong(PID))  # 停止变速
        print(f"Process {PID} is now running at normal speed.")
    except Exception as e:
        print(f"Failed to stop speed modification for process {PID}: {e}")