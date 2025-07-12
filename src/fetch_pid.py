# 导入模块
import psutil


# 预载变量
process_name = "WeChatAppEx.exe"
PID = None
RAM = 0


# 主要逻辑
for process in psutil.process_iter(['pid', 'name', 'memory_info']):
    try:
        if process.info['name'] == process_name:
            ram_usage = process.info['memory_info'].rss
            if ram_usage > RAM:
                RAM = ram_usage
                PID = process.info['pid']
    except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess) as E:
        print("Error 1!")
        exit(f"未找到{process_name}, 请先运行微信小程序, 并且给予本程序足够的权限, 然后再试一次! ")


# 测试入口
if __name__ == "__main__":
    if PID is not None:
        print(f"微信小程序的 PID: {PID}, 内存占用: {RAM / (1024 * 1024):.2f} MB")
    else:
        print(f"未找到名为 {process_name} 的进程。请确保微信小程序正在运行。")
