#ifndef IMAGINE_MUDUO_EVENTLOOP_H
#define IMAGINE_MUDUO_EVENTLOOP_H

#include "Imagine_Time/Timer.h"
#include "Imagine_Log/Logger.h"
#include "Imagine_Log/SingletonLogger.h"
#include "Imagine_Log/NonSingletonLogger.h"
#include "Imagine_Muduo/ThreadPool.h"
#include "Imagine_Muduo/common_definition.h"

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

class Channel;

class Poller;
class EpollPoller;

class EventLoop
{
 public:
   // 配置文件形式初始化
   EventLoop();

   EventLoop(std::string  profile_name);

   EventLoop(int port, int thread_num = 10, int max_channel = 10000, EventCallback read_cb = nullptr, EventCallback write_cb = nullptr, EventCommunicateCallback communicate_cb = nullptr);

   ~EventLoop();

   void Init(std::string profile_path);

   void InitProfilePath(std::string profile_name);
   
   void GenerateSubmoduleProfile(YAML::Node config);

   void loop();

   int GetChannelnum();

   bool AddListenChannel(std::string port);

   bool AddListenChannel(int port);

   bool AddEventChannel(std::string port);

   bool AddEventChannel(int port);

   void AddChannelnum();

   int GetMaxchannelnum();

   Poller *GetEpoll();

   EventCallback GetReadCallback();

   EventCallback GetWriteCallback();

   EventCommunicateCallback GetCommunicateCallback();

   void SetReadCallback(EventCallback read_callback_);

   void SetWriteCallback(EventCallback write_callback_);

   void SetCommunicateCallback(EventCommunicateCallback communicate_callback);

   void UpdateChannel(std::shared_ptr<Channel> channel);

   void Close(std::shared_ptr<Channel> channel);

   // 心跳检测根据fd删除channel用
   void Closefd(int fd);

   void DestroyClosedChannel();

   /*
   -所有的时间参数都是以秒为单位，并且是从loop开始的相对时间
   -delay_表示首次开始执行的延时时间,为0表示从当前开始每interval_时间执行一次
   -interval_表示每次执行的间隔时间,为0表示只执行一次
   -delay_与interval_不能同时为0
   */
   long long SetTimer(Imagine_Tool::TimerCallback timer_callback, double interval, double delay = 0.0);

   bool CloseTimer(long long timer_id);

   bool InsertTimer(Imagine_Tool::Timer *timer);

   std::vector<Imagine_Tool::Timer *> GetExpiredTimers(const Imagine_Tool::TimeStamp &now);

   // std::shared_ptr<Channel> GetTimer(){return timer_channel;}

 private:
  size_t thread_num_;
  size_t max_channel_num_;
  size_t port_;
  std::string log_name_;
  std::string log_path_;
  size_t max_log_file_size_;
  bool async_log_;
  bool singleton_log_mode_;
  std::string log_title_;
  bool log_with_timestamp_;
  Imagine_Tool::Logger* logger_;

  std::string profile_path_;
  std::string log_profile_name_;

 private:
   bool quit_;
   ThreadPool<std::shared_ptr<Channel>> *pool_;
   std::list<std::shared_ptr<Channel>> close_list_;
   pthread_t *destroy_thread_;
   pthread_mutex_t destroy_lock_;
   int channel_num_;           // 当前连接的客户端数目
   EventCallback read_callback_;
   EventCallback write_callback_;
   EventCommunicateCallback communicate_callback_ = nullptr;
   Poller *epoll_;
   // vector<Channel*> channels_;
   std::shared_ptr<Channel> listen_channel_;

   pthread_mutex_t timer_lock_;
   std::shared_ptr<Channel> timer_channel_;
   std::priority_queue<Imagine_Tool::Timer *, std::vector<Imagine_Tool::Timer *>, Imagine_Tool::TimeUtil::TimerPtrCmp> timers_;

   pthread_mutex_t timer_map_lock_;
   std::unordered_map<long long, Imagine_Tool::Timer *> timer_map_;
   // const int timerfd_;
};

} // namespace Imagine_Muduo

#endif