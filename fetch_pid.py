import psutil

process_name = "WeChatAppEx.exe"
PID = None
RAM = 0

for process in psutil.process_iter(['pid', 'name', 'memory_info']):
    try:
        if process.info['name'] == process_name:
            ram_usage = process.info['memory_info'].rss  # 获取常驻内存大小（字节）
            if ram_usage > RAM:
                RAM = ram_usage
                PID = process.info['pid']
    except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess) as E:
        print("Error 1!")
        exit(f"未找到{process_name}, 请先运行微信小程序, 并且给予本程序足够的权限, 然后再试一次! ")