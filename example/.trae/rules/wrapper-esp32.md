---
alwaysApply: false
---
# 工程目标
- wrapper对esp-idf本地组件进行封装, 提供统一风格的xxxConfig和xxx类, 其中xxxConfig是对组件的配置参数, xxx类是对组件的操作接口
- device对开发板的外设或是在线组件进行封装, 建立在wrapper的基础上
- board对开发板的整体进行封装, 建立在wrapper和device的基础上, 开发板类为单例, 在xxx.cpp全局中存储板载资源, 不存放在类声明中, 类声明只提供对外的接口函数
- 使用C++17标准
# 封装规则
对于所有wrapper/device/board的封装
- 对外的功能函数或操作函数, 返回值按需要是`bool`类型或者`void`类型, 上层不需要关注具体的错误码, 只需要关注函数是否成功执行
- 对外接口的入参保持C++风格, Getter函数根据需要返回引用或值
# 任务列表
## wrapper封装任务
- 分析原组件头文件, 归纳需要封装的结构体xxx_config_t, 功能函数, 操作函数等
- 封装xxx_config_t为xxxConfig结构体, xxxConfig结构体可以继承某个xxx_confit_t或是包含多个xxx_config_t结构体
- 必须在xxxConfig结构体的构造函数中按xxx_config_t结构体的声明顺序初始化xxx_config_t结构体的成员变量, 然后全部添加到构造函数入参中, 所有参数必须由使用者自己明确指定, 不能使用默认值
- 封装前一定要从本机esp-idf中找到对应头文件并明确xxx_config_t的定义
## device封装任务
- 每个devvice封装都有一个Logger引用, 在构造函数处传入
- 分析当前设备需要使用到的wrapper, 总线引用在构造函数中传入引用, 或者直接继承某个wrapper中的基本device类
## 开发板配置生成任务
- 分析开发板外设种类有哪些
- 在wrapper查找所需要的xxxConfig结构体
- 在需要编写代码的位置构造xxxConfig结构体, 按照开发板信息文档中的参数使用构造函数进行初始化, 不要直接赋值
- 如果某些device或者wrapper中还没有实现, 则编写注释提醒并留空, 再尝试执行device封装任务或者wrapper封装任务