import ctypes
from ctypes import wintypes
import sys

# Define necessary Windows API functions and constants
OpenProcess = ctypes.windll.kernel32.OpenProcess
VirtualAllocEx = ctypes.windll.kernel32.VirtualAllocEx
WriteProcessMemory = ctypes.windll.kernel32.WriteProcessMemory
CreateRemoteThread = ctypes.windll.kernel32.CreateRemoteThread
GetProcAddress = ctypes.windll.kernel32.GetProcAddress
GetModuleHandle = ctypes.windll.kernel32.GetModuleHandleA
LoadLibraryA = ctypes.windll.kernel32.LoadLibraryA
CloseHandle = ctypes.windll.kernel32.CloseHandle

# Constants for Windows API
PROCESS_ALL_ACCESS = 0x1F0FFF
MEM_COMMIT = 0x1000
MEM_RESERVE = 0x2000
PAGE_READWRITE = 0x04

def inject_dll(pid, dll_path):
    # Open the target process
    h_process = OpenProcess(PROCESS_ALL_ACCESS, False, pid)
    if not h_process:
        print(f"Failed to open process {pid}. Error: {ctypes.GetLastError()}")
        return False

    # Allocate memory in the target process for the DLL path
    dll_path_bytes = dll_path.encode('utf-8')
    dll_path_len = len(dll_path_bytes) + 1
    alloc_address = VirtualAllocEx(h_process, None, dll_path_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
    if not alloc_address:
        print(f"Failed to allocate memory in process {pid}. Error: {ctypes.GetLastError()}")
        CloseHandle(h_process)
        return False

    # Write the DLL path into the allocated memory
    written = ctypes.c_size_t(0)
    if not WriteProcessMemory(h_process, alloc_address, dll_path_bytes, dll_path_len, ctypes.byref(written)):
        print(f"Failed to write DLL path to process {pid}. Error: {ctypes.GetLastError()}")
        CloseHandle(h_process)
        return False

    # Get the address of LoadLibraryA
    kernel32_handle = GetModuleHandle(b"kernel32.dll")
    load_library_addr = GetProcAddress(kernel32_handle, b"LoadLibraryA")
    if not load_library_addr:
        print(f"Failed to get address of LoadLibraryA. Error: {ctypes.GetLastError()}")
        CloseHandle(h_process)
        return False

    # Create a remote thread in the target process to call LoadLibraryA
    thread_handle = CreateRemoteThread(h_process, None, 0, load_library_addr, alloc_address, 0, None)
    if not thread_handle:
        print(f"Failed to create remote thread in process {pid}. Error: {ctypes.GetLastError()}")
        CloseHandle(h_process)
        return False

    # Wait for the remote thread to finish
    ctypes.windll.kernel32.WaitForSingleObject(thread_handle, 0xFFFFFFFF)

    # Clean up
    CloseHandle(thread_handle)
    CloseHandle(h_process)
    print(f"Successfully injected DLL into process {pid}.")
    return True

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python inject.py <pid> <dll_path>")
        sys.exit(1)

    target_pid = int(sys.argv[1])
    target_dll = sys.argv[2]

    if inject_dll(target_pid, target_dll):
        print("Injection succeeded.")
    else:
        print("Injection failed.")