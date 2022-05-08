# MigangBot

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/2f2ff355bd7f482badfb151c8ad76744)](https://www.codacy.com/gh/LambdaYH/MigangBot/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=LambdaYH/MigangBot&amp;utm_campaign=Badge_Grade)

基于[ithewei/libhv](https://github.com/ithewei/libhv)网络库和[idealvin/cocoyaxi](https://github.com/idealvin/cocoyaxi)go-style coroutine


## 

| 父模块 | 子模块 | 模块功能 | 例子 |
| :----- | :----- | :------- | :------- |
| Echo | Echo | 回声 | [示例](examples/echo.md) |
| BotManage | help | 显示帮助信息 | [示例](examples/botmanage_help.md) |
| BotManage | status_info | 显示系统/机器人状态 | [示例](examples/botmanage_status_info.md) |
| BotManage | feedback | 用户可以给维护者发送消息，维护者也可以进行回复 | [示例](examples/botmanage_feedback.md) |
| BotManage | friend_invite | 处理好友邀请事件 | 顾名思义 |
| BotManage | group_invite | 处理入群事件 | 顾名思义 |
| TencentCloudNLP | AutoSummarization | 使用腾讯云NLP API进行摘要提取 | [示例](examples/TencentCloudNLP.md) | 
| TencentCloudNLP | KeywordsExtraction | 使用腾讯云NLP API进行关键词提取 | [示例](examples/TencentCloudNLP.md) | 
| EorzeaZhanbu | EorzeaZhanbu | 艾欧泽亚占卜 | [示例](examples/eorzea_zhanbu.md) | 
| BilibiliParser | BilibiliParser | 对群内的B站番剧/视频/直播等信息进行解析 | [示例](examples/bilibiliparser.md) | 

## For User

    libopencv

## For Developer

### Requirements

    Cmake
    tencentcloud-sdk-cpp

See [Github Action](https://github.com/LambdaYH/MigangBot/blob/main/.github/workflows/cmake.yml) for more Details.

### Step

#### Debian or Ubuntu

    sudo apt install git cmake build-essential default-libmysqlclient-dev libopencv-dev uuid-dev libssl-dev libcurl4-openssl-dev -y

    git clone https://github.com/TencentCloud/tencentcloud-sdk-cpp.git
    cd tencentcloud-sdk-cpp
    mkdir build && cd build
    cmake -DBUILD_SHARED_LIBS=off -DBUILD_MODULES="nlp" ..
    make && sudo make install

    git clone https://github.com/LambdaYH/MigangBot
    cd MigangBot
    git submodule update --init
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make

#### Use after build

    mkdir path/to/bot/root
    cp ./bin/MigangBot path/to/bot/root
    cp ../resources/* path/to/bot/root -r
    sudo chmod +x path/to/bot/root/MigangBot

    sudo apt install mariadb-server
    mysql_secure_installation
    mysql -uroot
    create database Your_Database_name;
    CREATE USER 'Your Databse User' IDENTIFIED BY 'enter_a_password_here';
    GRANT ALL ON Your_Database_name.* TO 'Your Databse User' IDENTIFIED BY 'enter_a_password_here';

    sudo apt install redis-server

## Todo

-   [x] 支持配置文件
-   [x] 实现OnebotAPI，对go-cqhttpAPI进行部分实现（以后用到了再去弄吧）
-   [x] 添加日志
-   [ ] 添加更多功能
-   [x] 接入mysql
-   [x] 可以在对应功能运行中调用API获取返回内容
-   [x] 支持汉字指令
-   [x] 简化插件添加流程
-   [ ] 加注释
-   [x] 支持注册各类事件
-   [x] 加文字转图片，图片转base64
-   [x] 实现权限控制
-   [x] 实现only_to_me

## 如何添加插件

首先写一个继承自Module的类并实现其接口，该类位于modules/module_interface.h文件中。

写完后在module_list中按照样例添加即可。
