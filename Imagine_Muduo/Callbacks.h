#ifndef IMAGINE_MUDUO_CALLBACKS_H
#define IMAGINE_MUDUO_CALLBACKS_H

#include<functional>

namespace Imagine_Muduo{

using EventCallback=std::function<struct iovec*(const struct iovec*)>;//允许用户注册读写事件回调函数,采用分散读写的方式
using TimerCallback=std::function<void()>;//允许用户注册定时器回调函数
using EventCommunicateCallback=std::function<bool(const char*,int)>;//允许用户注册TCP边界问题处理回调函数
using EventHandler=std::function<void()>;//不同的Channel有不同的处理逻辑,暂时写死,不允许用户更改

}



#endif