#ifndef IMAGINE_MUDUO_COMMON_DEFINITION_H
#define IMAGINE_MUDUO_COMMON_DEFINITION_H

#include <functional>

namespace Imagine_Muduo
{

class HashPair
{
 public:
    template <typename first, typename second>
    std::size_t operator()(const std::pair<first, second> &p) const
    {
        auto hash1 = std::hash<first>()(p.first);
        auto hash2 = std::hash<second>()(p.second);
        return hash1 ^ hash2;
    }
};

class EqualPair
{
 public:
    template <typename first, typename second>
    bool operator()(const std::pair<first, second> &a, const std::pair<first, second> &b) const
    {
        return a.first == b.first && a.second == b.second;
    }
};

using EventCallback = std::function<struct iovec *(const struct iovec *)>;      // 允许用户注册读写事件回调函数,采用分散读写的方式
// using TimerCallback = std::function<void()>;                                    // 允许用户注册定时器回调函数
using EventCommunicateCallback = std::function<bool(const char *, int)>;        // 允许用户注册TCP边界问题处理回调函数
using EventHandler = std::function<void()>;                                     // 不同的Channel有不同的处理逻辑,暂时写死,不允许用户更改

class Connection;
using ChannelCallback = std::function<void(void)>;
using ConnectionCallback = std::function<void(Connection* conn)>;

} // namespace Imagine_Muduo

#endif