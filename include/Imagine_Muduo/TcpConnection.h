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

    Connection* Create(const std::shared_ptr<Channel>& channel) const;

    void ReadHandler();

    void WriteHandler();

    void DefaultReadCallback(Connection* conn) const;

    void DefaultWriteCallback(Connection* conn) const;
};

} // namespace Imagine_Muduo

#endif