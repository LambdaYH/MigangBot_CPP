# MigangBotCPP

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/2f2ff355bd7f482badfb151c8ad76744)](https://www.codacy.com/gh/LambdaYH/MigangBotCPP/dashboard?utm_source=github.com&utm_medium=referral&utm_content=LambdaYH/MigangBotCPP&utm_campaign=Badge_Grade)

## 

| 模块名 | 模块功能 |
| ----- | ------- |
|Echo| 回声|

## For User

### Requirements

    sudo apt install default-libmysqlclient-dev -y

## For Developer

### Requirements

    [Boost](https://github.com/boostorg/boost)
    Cmake
    libmysqlclient

See [Github Action](https://github.com/LambdaYH/MigangBotCPP/blob/main/.github/workflows/cmake.yml) for more Details.

### Step

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
-   [x] 接入mysql
-   [x] 可以在对应功能运行中调用API获取返回内容
-   [x] 支持汉字指令
-   [x] 简化插件添加流程
-   [ ] 加注释
-   [x] 支持注册各类事件

## 如何添加插件

首先写一个继承自PluginInterface的类并实现其接口，该类位于plugins/plugin_interface.h文件中。

写完后在module_list中按照样例添加即可。
