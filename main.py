from ctypes import *
from fetch_pid import PID, process_name, RAM

if PID:
    print(f"已确定{process_name}的PID: {PID}, RAM使用: {RAM / (1024 * 1024):.2f} MB.")
else:
    print("Error 2!")
    exit(f"未找到{process_name}, 请先运行微信小程序, 并且给予本程序足够的权限, 然后再试一次! ")

dll = windll.LoadLibrary("HookDll.dll")

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