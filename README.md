# WebServer

![https://img.shields.io/github/workflow/status/VcSo/WebServer/webserverci](https://img.shields.io/github/workflow/status/VcSo/WebServer/webserverci)

C++ WebServer

```
边理解边写 项目目前已正常运行  http部分还没吃透 
压力测试 入门云服务器 1000并发 60秒 稳定
```

使用线程池+非阻塞socket+epoll(LT/ET)+Reactor/Proactor高并发模型

- 支持http 可解析get/post请求 
- 支持请求图片/视频 
- 连接mysql 支持用户注册/登陆功能 
- 实现定时器 支持请求超时功能
- 实现同步/异步写日志
