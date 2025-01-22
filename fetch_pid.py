import psutil

def get_max_ram_pid(process_name):
    max_ram = 0
    max_pid = None

    for process in psutil.process_iter(['pid', 'name', 'memory_info']):
        try:
            if process.info['name'] == process_name:
                ram_usage = process.info['memory_info'].rss  # 获取进程的常驻内存大小（以字节为单位）
                if ram_usage > max_ram:
                    max_ram = ram_usage
                    max_pid = process.info['pid']
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            continue

    return max_pid, max_ram

if __name__ == "__main__":
    process_name = "WeChatAppEx.exe"
    pid, ram = get_max_ram_pid(process_name)
    if pid:
        print(f"PID: {pid}, RAM Usage: {ram / (1024 * 1024):.2f} MB")
    else:
        print(f"No process found with name '{process_name}'.")
