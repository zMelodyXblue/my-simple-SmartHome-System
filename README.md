- 一个简易的智能家居管理模拟系统项目;

- 编写环境: Linux

- Client端显示简易图形界面(需要安装ncurses库);

- 按下回车可输入信息，再次按下回车可发送信息:

1. 所发送信息默认所有在线用户可见;
2. 信息开头用"@用户名 "(之后有一个空格)格式可令信息只对该用户名的用户可见;
3. '#'开头的信息为设备控制信息;
4. 如果发送信息后屏幕乱码，可再发送一个回车恢复正常.

- '#' 开头的控制命令如下所示：

```
#1 : 查看可用的设备列表
#20+空格+id : 修改设备状态为关
#21+空格+id : 修改设备状态为开
#22+空格+id : 修改设备状态为待机
#30+空格+name :  新增设备灯
#31+空格+name : 新增设备开关
#32+空格+name : 新增设备恒温设备
#4+空格+id : 删除设备
```

使用前请修改config配置文件

可直接在Linux系统中运行bin文件夹中的可执行程序

注：目前仍含有调试信息
