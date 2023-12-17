#ifndef IMAGINE_MUDUO_SERVER_H
#define IMAGINE_MUDUO_SERVER_H

#include "Acceptor.h"
#include "TcpConnection.h"
#include "common_definition.h"
#include "common_typename.h"

#include "yaml-cpp/yaml.h"

#include <unordered_map>

namespace Imagine_Muduo
{

class Server
{
 public:
   Server();

   Server(const std::string& profile_name, Connection* msg_conn = new TcpConnection(), Connection* acceptor = new Acceptor());

   Server(const YAML::Node& config, Connection* msg_conn = new TcpConnection(), Connection* acceptor = new Acceptor());

   virtual ~Server();

   void Init();

   void Start();

   virtual void DefaultReadCallback(Connection* conn) = 0;

   virtual void DefaultWriteCallback(Connection* conn) = 0;

   long long SetTimer(TimerCallback timer_callback, double interval, double delay = 0.0);

   Server* const RemoveTimer(long long timerfd);

   Server* const AddAndSetConnection(Connection* new_conn);

   Server* const AddConnection(Connection* new_conn);

   Server* const SetReadCallback(Connection* new_conn);

   Server* const SetWriteCallback(Connection* new_conn);

   Connection* GetMessageConnection() const;

   void DestroyConnection();

   Server* const CloseConnection(const std::string& ip, const std::string& port);

 private:
   Connection* const RemoveConnection(const std::string& ip, const std::string& port);

 private:
   EventLoop* loop_;                                                                                          // Loop对象
   Connection* acceptor_;                                                                                     // 接收连接的Connection对象(对应监听端口的Channel)
   Connection* msg_conn_;                                                                                     // 与客户端通信的Connection对象(提供模板)
   std::unordered_map<std::pair<std::string, std::string>, Connection*, HashPair, EqualPair> conn_map_;       // 所有已经建立连接的Connection对象集合
   std::mutex map_lock_;                                                                                      // conn_map_的锁
   ConnectionCallback read_callback_;                                                                         // 请求业务处理处理函数, 由msg_conn_决定 
   ConnectionCallback write_callback_;                                                                        // 写请求业务处理函数, 由msg_conn_决定
   std::list<Connection*> close_list_;                                                                        // 连接关闭队列
   pthread_t *destroy_thread_;                                                                                // 连接关闭线程
   pthread_mutex_t destroy_lock_;                                                                             // 连接关闭队列的锁
};

} // namespace Imagine_Muduo

#endif