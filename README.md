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
//mkdir build && cp -r resources ./build
sh build.sh
```
---

## Run

``` 
./serverone -ip 127.0.0.1 -p 20999 -u sql_username -w sql_password -d sql_database 
            -g 1 -l 1 -e 3 -s 8 -t 8 -a 1  -c 1
```

`i:p:u:w:d:g:l:e:s:t:a:c:`

- i: ip,
- p: port, 
- u: sql_username, 
- w: sql_password, 
- d: sql_database, 
- g: use_log, 0为false，1为true
- l: lingermode, onoff 0/1
- e: et, 0:lt+lt, 1:lt+et, 2:et+lt, 3:et+et
- s: sqlthreadnum, 
- t: threadnum, 
- a: actor_mode, 0 = proactor, 1 = reactor
- c: async, log use async
 
 
 