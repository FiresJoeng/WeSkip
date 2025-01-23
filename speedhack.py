import os
from ctypes import windll, c_int, c_ulong, byref

from fetch_pid import PID

# .dll function calling: 
# InjectSpeedhack(Speed)
# EjectSpeedhack()

DLL_PATH = "dll/speedhack.dll"
if not os.path.exists(DLL_PATH):
    print("Error 3!")
    exit(f"DLL文件 {DLL_PATH} 不存在!")
dll = windll.LoadLibrary(DLL_PATH)


def inject_dll(pid, dll_path):
    kernel32 = windll.kernel32
    process_handle = kernel32.OpenProcess(0x1F0FFF, False, pid)  # 0x1F0FFF = PROCESS_ALL_ACCESS
    
    if not process_handle:
        raise Exception(f"无法打开进程 {pid}")
    
    dll_path_encoded = dll_path.encode('utf-8')
    allocated_memory = kernel32.VirtualAllocEx(process_handle, 0, len(dll_path_encoded), 0x1000, 0x40)
    
    if not allocated_memory:
        raise Exception("内存分配失败")
    
    written_bytes = c_int(0)
    kernel32.WriteProcessMemory(process_handle, allocated_memory, dll_path_encoded, len(dll_path_encoded), byref(written_bytes))
    
    if written_bytes.value != len(dll_path_encoded):
        raise Exception("写入内存失败")
    
    thread_id = c_ulong(0)
    kernel32.CreateRemoteThread(process_handle, None, 0, kernel32.GetProcAddress(kernel32.GetModuleHandleW("kernel32.dll"), "LoadLibraryW"), allocated_memory, 0, byref(thread_id))
    
    # Close the process handle
    kernel32.CloseHandle(process_handle)
    print(f"成功注入 DLL 到进程 {pid}")


def eject_dll(pid):
    kernel32 = windll.kernel32
    process_handle = kernel32.OpenProcess(0x1F0FFF, False, pid)
    
    if not process_handle:
        raise Exception(f"无法打开进程 {pid}")
    
    # Assuming the DLL has already been loaded, you need to find its address or handle.
    # For simplicity, let's assume that you can call EjectSpeedhack without needing a specific address.
    kernel32.CreateRemoteThread(process_handle, None, 0, dll.EjectSpeedhack, None, 0, None)
    
    kernel32.CloseHandle(process_handle)
    print(f"成功从进程 {pid} 中卸载 DLL")


def SPEEDx10():
    try:
        inject_dll(PID, DLL_PATH)
        dll.InjectSpeedhack(10)
    except Exception as e:
        print(f"错误: {e}")


def SPEEDx1():
    try:
        inject_dll(PID, DLL_PATH)
        dll.InjectSpeedhack(1)
        eject_dll(PID)
    except Exception as e:
        print(f"错误: {e}")
