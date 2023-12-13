#ifndef IMAGINE_MUDUO_EVENTLOOP_H
#define IMAGINE_MUDUO_EVENTLOOP_H

#include "common_definition.h"
#include "common_typename.h"

#include <pthread.h>
#include <vector>
#include <list>
#include <unistd.h>
#include <functional>
#include <queue>
#include <memory>
#include <unordered_map>

namespace Imagine_Muduo
{

template <typename T>
class ThreadPool;
class Channel;
class Poller;

class EventLoop
{
 public:
   EventLoop();

   EventLoop(const std::string&  profile_name);

   EventLoop(const YAML::Node& config);

   ~EventLoop();

   void Init(const std::string& profile_path);

   void Init(const YAML::Node& config);

   void InitLoop();

   void loop();

   int GetChannelnum() const;

   EventLoop* AddListenChannel(const std::string& port);

   EventLoop* AddListenChannel(int port);

   EventLoop* AddEventChannel(const std::string& port);

   EventLoop* AddEventChannel(int port);

   std::shared_ptr<Channel> GetListenChannel() const;

   int GetMaxchannelnum() const;

   EventLoop* AddChannel(std::shared_ptr<Channel> channel);

   const EventLoop* UpdateChannel(std::shared_ptr<Channel> channel) const;

   EventLoop* CloseChannel(std::shared_ptr<Channel> channel);

   // 心跳检测根据fd删除channel用
   EventLoop* Closefd(int fd);

   /*
   -所有的时间参数都是以秒为单位，并且是从loop开始的相对时间
   -delay_表示首次开始执行的延时时间,为0表示从当前开始每interval_时间执行一次
   -interval_表示每次执行的间隔时间,为0表示只执行一次
   -delay_与interval_不能同时为0
   */
   long long SetTimer(TimerCallback timer_callback, double interval, double delay = 0.0);

   EventLoop* CloseTimer(long long timer_id);

   EventLoop* InsertTimer(Timer *timer);

   std::vector<Timer *> GetExpiredTimers(const TimeStamp &now);

 private:
  // 配置文件字段
  size_t thread_num_;                                                             // 线程池线程数目
  size_t max_channel_num_;                                                        // 允许的最大连接数
  size_t port_;                                                                   // 监听端口
  bool singleton_log_mode_;                                                       // 单例日志(目前仅支持单例日志)
  Logger* logger_;                                                                // 日志对象

 private:
   bool quit_;                                                                    // loop退出标识
   ThreadPool<std::shared_ptr<Channel>> *thread_pool_;                            // 线程池对象
   int channel_num_;                                                              // 当前连接的客户端数目
   Poller *epoll_;                                                                // I/O多路复用(epoll)对象
   std::shared_ptr<Channel> listen_channel_;                                      // 负责监听端口的channel
   pthread_mutex_t timer_lock_;                                                   // 定时器队列的锁
   std::shared_ptr<Channel> timer_channel_;                                       // 负责定时器计时的channel
   std::priority_queue<Timer *, std::vector<Timer *>, TimerPtrCmp> timers_;       // 定时器队列
   pthread_mutex_t timer_map_lock_;                                               // 定时器hash_map的锁
   std::unordered_map<long long, Timer *> timer_map_;                             // 定时器hash_map
};

} // namespace Imagine_Muduo

#endif