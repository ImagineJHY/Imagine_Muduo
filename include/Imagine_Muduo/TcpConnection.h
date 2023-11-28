#ifndef IMAGINE_MUDUO_TCPCONNECTION_H
#define IMAGINE_MUDUO_TCPCONNECTION_H

#include "Connection.h"

namespace Imagine_Muduo
{

class TcpConnection : public Connection
{
 public:
    TcpConnection();

    TcpConnection(std::shared_ptr<Channel> channel);

    TcpConnection(Server* server, std::shared_ptr<Channel> channel);

    ~TcpConnection();

    Connection* Create(std::shared_ptr<Channel> channel);

    void ReadHandler();

    void WriteHandler();

    void DefaultReadCallback(Connection* conn);

    void DefaultWriteCallback(Connection* conn);

 private:
    
};

} // namespace Imagine_Muduo

#endif