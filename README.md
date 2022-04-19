# MigangBotCPP

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/2f2ff355bd7f482badfb151c8ad76744)](https://www.codacy.com/gh/LambdaYH/MigangBotCPP/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=LambdaYH/MigangBotCPP&amp;utm_campaign=Badge_Grade)

## 

目前极其简陋，仅支持简单的回声功能

## Requirements

    [Boost](https://github.com/boostorg/boost)
    Cmake
    libmysqlclient

## Build

    git clone https://github.com/LambdaYH/MigangBotCPP
    cd MigangBotCPP
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make

## Todo

-   [x] 支持配置文件
-   [x] 实现OnebotAPI，对go-cqhttpAPI进行部分实现（以后用到了再去弄吧）
-   [x] 添加日志
-   [ ] 添加更多功能
-   [ ] 接入mysql，记录用户信息，群功能启用情况等
-   [x] 可以在对应功能运行中调用API获取返回内容
-   [x] 支持汉字指令
-   [x] 简化插件添加流程
-   [ ] 加注释
-   [x] 支持注册各类事件
