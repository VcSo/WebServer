# C++ WebServer

![server](https://img.shields.io/github/actions/workflow/status/VcSpace/WebServer/c-cpp.yml?branch=main)

- 使用 c++编写的 Webserver， 项目使用线程池，Epoll，Reactor/Proactor模式，同步/异步写日志， 
- 支持解析HTTP GET/POST 请求，实现Web端查看网页，图片，视频，
- 连接 mysql，实现了用户注册与登陆功能 。

---

## sql

```
create database serverm;
use serverm;
create table user(
            uid int(10) not null auto_increment primary key,
            username char(50) not null,
            password char(16) not null,
            create_time datetime not null
)ENGINE=InnoDB,default charset=utf8;

insert into user(uid, username,password,create_time) values(1,'123','456',now());
#insert into user(username,password,create_time) values('vv','456',now());
```

---

## build

```
mkdir build && cp -r ./resources/ ./build
cd build
cmake ..
make -j4
./serverone
```
