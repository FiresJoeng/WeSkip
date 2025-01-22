import psutil

process_name = "WeChatAppEx.exe"
PID = None

for process in psutil.process_iter(['pid', 'name', 'memory_info']):
    try:
        if process.info['name'] == process_name:
            PID = process.info['pid']
            break
    except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
        exit()

if PID:
    print(f"已确定{process_name}的PID: {PID}")
else:
    exit(f"未找到'{process_name}'.")
