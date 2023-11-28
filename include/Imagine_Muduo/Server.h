#ifndef IMAGINE_MUDUO_SERVER_H
#define IMAGINE_MUDUO_SERVER_H

#include "Acceptor.h"
#include "TcpConnection.h"

#include <unordered_map>

namespace Imagine_Muduo
{

class Server
{
 public:
   Server();

   Server(std::string profile_name, Connection* msg_conn = new TcpConnection(), Connection* acceptor = new Acceptor());

   Server(YAML::Node config, Connection* msg_conn = new TcpConnection(), Connection* acceptor = new Acceptor());

   virtual ~Server();

   void Init();

   void Start();

   virtual void DefaultReadCallback(Connection* conn) = 0;

   virtual void DefaultWriteCallback(Connection* conn) = 0;

   long long SetTimer(Imagine_Tool::TimerCallback timer_callback, double interval, double delay = 0.0);

   Server* const RemoveTimer(long long timerfd);

   Server* const AddAndSetConnection(Connection* new_conn);

   Server* const AddConnection(Connection* new_conn);

   Server* const SetReadCallback(Connection* new_conn);

   Server* const SetWriteCallback(Connection* new_conn);

   Connection* GetMessageConnection() const;

   void DestroyConnection();

   Server* const CloseConnection(std::string ip, std::string port);

 private:
   Connection* const RemoveConnection(std::string ip, std::string port);

 private:
   EventLoop* loop_;
   Connection* acceptor_;
   Connection* msg_conn_;
   std::unordered_map<std::pair<std::string, std::string>, Connection*, HashPair, EqualPair> conn_map_;
   std::mutex map_lock_;
   ConnectionCallback read_callback_;
   ConnectionCallback write_callback_;

   std::list<Connection*> close_list_;
   pthread_t *destroy_thread_;
   pthread_mutex_t destroy_lock_;
};

} // namespace Imagine_Muduo

#endif