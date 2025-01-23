# speedhack.dll

这里是speedhack.dll的Visual Studio项目, 可能日后会被单独移动至一个新仓库. 



### 编译教程:

**① 安装Visual Studio及C++环境依赖**

**② 打开main.sln, 导入当前目录下"speedhack_code"文件夹的两个文件到项目中**

**③ 打开项目属性页面**

​	(1) 添加附加包含目录, 并置于最前的位置:  

```
\Detours\include;
```

​	(2) 添加附加库目录, 并置于最前的位置:  

```
Detours\lib.X64;
```

​	(3) 添加附加依赖项, 并置于最前的位置:  

```
Detours\lib.X64\detours.lib;
```

​	(4) 禁用预编译头，对其选中"不使用预编译头".

**④ 依次点击位于Visual Studio工具栏的 生成 > 生成解决方案**

**⑤ 等待编译完成, 然后你会在某个新文件夹找到"vs_project.dll", 那个就是成品**



### 致谢:

感谢 [repo:absoIute/Speedhack](https://github.com/absoIute/Speedhack) 的变速技术驱动, 这个程序是抄这个仓库做的.  
感谢 [repo:microsoft/Detours](https://github.com/microsoft/Detours) 的Hook技术驱动, 我们靠它实现钩子拦截.
