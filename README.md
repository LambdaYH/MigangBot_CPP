# MigangBotCPP

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/2f2ff355bd7f482badfb151c8ad76744)](https://www.codacy.com/gh/LambdaYH/MigangBotCPP/dashboard?utm_source=github.com&utm_medium=referral&utm_content=LambdaYH/MigangBotCPP&utm_campaign=Badge_Grade)

## 

| 父模块 | 子模块 | 模块功能 |
| :----- | :----- | :------- |
| Echo | Echo | 回声 |
| BotManage | help | 显示帮助信息 |
| BotManage | status_info | 显示系统/机器人状态 |
| BotManage | feedback | 用户可以发送留言给维护者，维护者也可以进行回复 |

## For Developer

### Requirements

    [Boost](https://github.com/boostorg/boost)
    Cmake
    libmysqlclient
    tencentcloud-sdk-cpp

See [Github Action](https://github.com/LambdaYH/MigangBotCPP/blob/main/.github/workflows/cmake.yml) for more Details.

### Step

#### Debian or Ubuntu

    sudo apt install git cmake build-essential default-libmysqlclient-dev libopencv-dev uuid-dev libssl-dev libcurl4-openssl-dev ninja-build -y

    git clone https://github.com/TencentCloud/tencentcloud-sdk-cpp.git
    cd tencentcloud-sdk-cpp
    mkdir build && cd build
    cmake -DBUILD_SHARED_LIBS=off -DBUILD_MODULES="nlp" ..
    make && sudo make install

    git clone https://github.com/LambdaYH/MigangBotCPP
    cd MigangBotCPP
    git submodule update --init
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make

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
-   [ ] 加文字转图片，图片转base64
-   [x] 实现权限控制
-   [x] 实现only_to_me

## 如何添加插件

首先写一个继承自Module的类并实现其接口，该类位于modules/module_interface.h文件中。

写完后在module_list中按照样例添加即可。
