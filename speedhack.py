from ctypes import *
from fetch_pid import PID

# .dll function calling: InjectSpeedhack(Speed)
dll = windll.LoadLibrary("dll/speedhack.dll")


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
