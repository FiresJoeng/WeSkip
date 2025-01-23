import os
from ctypes import *
from fetch_pid import PID


# .dll function calling: 
# InjectSpeedhack(Speed)
# EjectSpeedhack()

dll_path = "dll/speedhack.dll"
if not os.path.exists(dll_path):
    print("Error 3!")
    exit(f"DLL文件 {dll_path} 不存在!")
dll = windll.LoadLibrary(dll_path)


def SPEEDx10():
    try:
        pass
    except Exception as e:
        print(f"错误: {e}")


def SPEEDx1():
    try:
        pass
    except Exception as e:
        print(f"错误: {e}")
