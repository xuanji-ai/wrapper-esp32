# nix-wrapper-esp32

这是一个对原有ESP-IDF组件, 以及ESP-Registry组件进行C++风格封装的组件, 也包含个人常用开发板的一些封装

主要封装代码在wrapper中, device和board是在wrapper之上建立的


# 为什么封装?

主要简化以下步骤的操作

- IDF初始化结构体 xxx_config_t结构复杂, 封装后屏蔽配置结构体嵌套填参, 以及屏蔽一些运行时生成的句柄参数, 所有常量参数在XxxConfig结构体构造函数处一次性填完
- IDF和Registry部分常用组件初始化步骤繁琐, 适当封装常用组合, 并添加报错处理