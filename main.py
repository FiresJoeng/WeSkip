import tkinter as tk
from fetch_pid import PID, process_name, RAM
from speedhack import SPEEDx1, SPEEDx10

if PID:
    print(f"已确定{process_name}的PID: {PID}, RAM使用: {RAM / (1024 * 1024):.2f} MB.")
else:
    print("Error 2!")
    exit(f"未找到{process_name}, 请先运行微信小程序, 并且给予本程序足够的权限, 然后再试一次!")

class MainApp:
    def __init__(self, main_window):
        self.main_window = main_window
        self.main_window.title('WeSkip')
        self.main_window.resizable(width=False, height=False)
        screenwidth = self.main_window.winfo_screenwidth()
        screenheight = self.main_window.winfo_screenheight()
        size = '%dx%d+%d+%d' % (250, 50, (screenwidth - 250) / 2, (screenheight - 50) / 2)
        self.main_window.geometry(size)

        self.button_state = tk.StringVar()
        self.button_state.set('广告加速')

        self.start_button = tk.Button(
            self.main_window, 
            textvariable=self.button_state, 
            command=self.toggle_speed
        )
        self.start_button.place(relx=0.5, rely=0.5, anchor='center', width=150, height=30)

    def toggle_speed(self):
        if self.button_state.get() == '广告加速':
            try:
                SPEEDx10()
                print("广告已加速!")
                self.button_state.set('恢复正常')
            except Exception as e:
                print(f"启动变速失败: {e}!")
        else:
            try:
                SPEEDx1()
                print("已恢复正常!")
                self.button_state.set('广告加速')
            except Exception as e:
                print(f"恢复正常失败: {e}!")

if __name__ == '__main__':
    root = tk.Tk()
    app = MainApp(root)
    root.mainloop()
