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

    Acceptor(Server* server, Connection* msg_conn);

    Acceptor(Server* server, std::shared_ptr<Channel> channel);

    Acceptor(Server* server, std::shared_ptr<Channel> channel, Connection* msg_conn);

    ~Acceptor();

    Connection* Create(std::shared_ptr<Channel> channel);

    void ReadHandler();

    void WriteHandler();

    void DefaultReadCallback(Connection* conn);

    void DefaultWriteCallback(Connection* conn);

    Connection* CreateMessageConnection(std::shared_ptr<Channel> channel);

    Acceptor* const SetMessageConnection(Connection* msg_conn);
 
 private:
    Connection* msg_conn_;
};

} // namespace Imagine_Muduo

#endif