import os
import ctypes
from pyinjector import inject
from fetch_pid import PID


# .dll function calling: 
# InjectSpeedhack(Speed)
# EjectSpeedhack()

DLL_PATH = "dll/speedhack.dll"
if not os.path.exists(DLL_PATH):
    print("Error 3!")
    exit(f"DLL文件 {DLL_PATH} 不存在!")

try:
    inject(PID, DLL_PATH)
except Exception as e:
    print(f"错误: {e}")

DLL = ctypes.windll.LoadLibrary(DLL_PATH)


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
