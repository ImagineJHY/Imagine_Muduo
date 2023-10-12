# Readme

## 简介

Imagine_Muduo是参考陈硕老师的muduo思想实现的一个框架，目前设定的应用场景是为服务器提供客户端连接框架，主要功能有：

- 对网络编程与具体业务逻辑进行了解耦
- 支持注册监听端口
- 支持使用线程池实现高并发
- 支持注册数据包边界问题处理回调函数
- 支持注册读写事件的回调函数(参数和返回值都需要设置为struct iovec*)
- 支持注册定时器，并支持为定时器注册不同的回调函数

与muduo的差异性有：

- 自动注册一个socket用于监听端口(后期可扩展为不自动注册)
- 暂未扩展到主从Reactor模型，目前是单Reactor模型+线程池的形式
- 不支持向Channel注册不同的读写回调函数
- 当前只支持epoll进行IO多路复用

## 快速上手

添加头文件EventLoop.h,并使用namespace Imagine_Muduo即可开始使用

- 操作系统：Linux
- 依赖：Linux下线程库pthread
- 启动：调用EventLoop::loop()函数即可启动服务器

## 说明

- EventLoop构造函数

  ```cpp
  //函数原型
  EventLoop(int port,int max_channel, EventCallback read_cb, EventCallback write_cb, EventCommunicateCallback communicate_cb);
  /*
  -参数
  	-port:端口值(默认服务器使用,可设为0不建立监听端口socket(其它逻辑尚未实现,慎用))
  	-max_channel:服务器支持的最大请求数
  	-read_cb:读回调函数
  	-write_cb:写回调函数
  	-communicate_cb:tcp边界问题处理函数(可为nullptr,在读写回调中自行进行处理)
  
  注:当前线程池默认是十个线程,暂未提供改变数目的接口...
  */
  ```

  

- 处理tcp数据包边界问题的回调函数

  ```cpp
  //回调函数指针原型
  using EventCommunicateCallback=std::function<bool(const char*,int)>;
  /*
  -参数
  	-第一个参数为收到的TCP数据内容
  	-第二个参数为数据字节数
  -返回值
  	-true:表示发生粘包
  	-false:未发生粘包
  */
  ```

- 读写事件回调函数

  ```cpp
  //回调函数指针原型
  using EventCallback=std::function<struct iovec*(const struct iovec*)>;
  /*
  -当前采用struct iovec进行读写控制
  -read_callback会将参数iovec的第一块的iov_base写为与对方通信的socketfd
  -read_callback以及write_callback均通过struct iovec*作为参数和返回值
  	-返回值中第一块struct iovec作为标志块
      -第一块中的iov_len标识了iovec的总块数
      -第一块中的iov_base总是需要在reactor中进行空间释放(要求必须通过new的方法生成)
      -第一块中的iov_base中前iov_len个char*标识了每一块是否需要在reactor中进行空间释放(仅write_iovec进行判断,当前read默认一定不需要在reactor中释放)
      -第一块中的iov_base中的第iov_len+1个char*用于标记是否断开连接,'1'代表保持连接,'0'代表断开连接
      -第一块中的iov_base中的第iov_len+2个char*用于标记是否发生TCP粘包,'1'代表是,'0'代表否
      -第一块中的iov_base中的第iov_len+3个char*用于标记接下来是否监听读事件,'1'代表是,'0'代表否
      -第一块中的iov_base中的第iov_len+4个char*用于标记接下来是否监听写事件,'1'代表是,'0'代表否
      
  注:当前不支持读写文件信息,即文件内容需要一次写到iovec中
  */
  ```

- 定时器相关函数

  ```cpp
  long long EventLoop::SetTimer(TimerCallback timer_callback_, double interval_, double delay_);
  /*
  -参数
  	-timer_callback_:定时回调函数
  	-interval_:若有值,其意义为定时周期,即该定时器会周期性启动
  	-delay_:若有值,其意义为第一次启动的延时时间
  -返回值
  	-大于0:定时器的id,便于删除该定时器(若为周期定时器)
  	-0:注册失败
  	
  注:
  	-interval_和delay_不能同时为0
  */
  
  bool EventLoop::CloseTimer(long long timer_id);
  /*
  -参数
  	-timer_id:需要关闭的定时器的id
  -返回值
  	-true:删除成功
  	-false:删除失败
  */
  ```

- 关闭连接

  ```cpp
  //函数原型
  void EventLoop::Closefd(int fd);
  /*
  -参数
  	-fd:需要关闭的socketfd(read_callback中获得)
  */
  ```

  