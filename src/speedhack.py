# 导入模块
import os
import ctypes
from src.fetch_pid import PID


# 全局变量
DLL_PATH = "dll/speedhack.dll"
if not os.path.exists(DLL_PATH):
    print("Error 3!")
    exit(f"DLL文件 {DLL_PATH} 不存在!")


# 函数: 检查管理员权限
def is_admin():
    """检查当前脚本是否以管理员权限运行"""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        print("Error 4!")
        return False


# 函数: 加载 DLL
def load_dll():
    if not is_admin():
        exit(f"请以管理员权限运行此脚本。右键单击 CMD 或 PowerShell -> 以管理员身份运行")
    try:
        DLL = ctypes.WinDLL(DLL_PATH)
        return DLL
    except Exception as e:
        print(f"错误: {e}")
        exit(f"加载 DLL 失败: {e}. 请确保 DLL 路径正确，并且 Python 和 DLL 都是相同的位数。")


speedhack_dll = load_dll()


# 函数: 注入 DLL
def inject_dll(pid, speed):
    try:
        inject_func = speedhack_dll.Inject
        inject_func.argtypes = [ctypes.c_ulong, ctypes.c_double]
        inject_func.restype = ctypes.c_bool
        result = inject_func(pid, speed)
        return result
    except Exception as e:
        print(f"注入失败: {e}")
        return False


# 函数: 卸载 DLL
def eject_dll(pid):
    try:
        eject_func = speedhack_dll.Eject
        eject_func.argtypes = [ctypes.c_ulong]
        eject_func.restype = ctypes.c_bool
        result = eject_func(pid)
        return result
    except Exception as e:
        print(f"卸载失败: {e}")
        return False


# 测试入口
if __name__ == "__main__":
    speed = 1
    inject_dll(PID, speed)
    print(f"Finished. PID: {PID}, Speed: {speed}")
