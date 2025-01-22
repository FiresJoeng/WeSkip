import tkinter as tk
from fetch_pid import PID, process_name, RAM
from speed_gear import SPEEDx1, SPEEDx10


if PID:
    print(f"已确定{process_name}的PID: {PID}, RAM使用: {RAM / (1024 * 1024):.2f} MB.")
else:
    print("Error 2!")
    exit(f"未找到{process_name}, 请先运行微信小程序, 并且给予本程序足够的权限, 然后再试一次! ")


class main:
    def __init__(self, main_window):
        self.main_window = main_window
        self.main_window.title('WeSkip')
        self.main_window.resizable(width=False, height=False)
        screenwidth = self.main_window.winfo_screenwidth()
        screenheight = self.main_window.winfo_screenheight()
        size = '%dx%d+%d+%d' % (200, 50, (screenwidth - 193) / 2, (screenheight - 50) / 2)
        self.main_window.geometry(size)
        
        self.tk.StringVar().set('Button')
        self.start_button = tk.Button(self.main_window,textvariable=tk.StringVar())
        self.start_button.place(x=50,y=8,width=100,height=32)
                
    
if __name__ == '__main__':
    root = tk.Tk()
    app = main(root)
    root.mainloop()
