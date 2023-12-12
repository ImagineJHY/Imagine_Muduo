#ifndef IMAGINE_MUDUO_ACCEPTOR_H
#define IMAGINE_MUDUO_ACCEPTOR_H

#include "Connection.h"

namespace Imagine_Muduo
{

class Server;

class Acceptor : public Connection
{
 public:
    Acceptor();

    Acceptor(Connection* msg_conn);

    Acceptor(Server* server, std::shared_ptr<Channel> channel);

    Acceptor(Server* server, std::shared_ptr<Channel> channel, Connection* msg_conn);

    ~Acceptor();

    Connection* Create(const std::shared_ptr<Channel>& channel) const;

    // Acceptor读事件的默认回调函数, 会注册给Channel
    void ReadHandler();

    // Acceptor写事件的默认回调函数, 会注册给Channel
    void WriteHandler();

    void DefaultReadCallback(Connection* conn) const;

    void DefaultWriteCallback(Connection* conn) const;

    Connection* CreateMessageConnection(const std::shared_ptr<Channel>& channel) const;
 
 private:
    const Connection* msg_conn_;             // 为新建立的连接提供回调函数模板, 属于Server
};

} // namespace Imagine_Muduo

#endif