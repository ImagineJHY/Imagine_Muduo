#ifndef IMAGINE_MUDUO_TCPSERVER_H
#define IMAGINE_MUDUO_TCPSERVER_H

#include "Server.h"

namespace Imagine_Muduo
{

class TcpServer : public Server
{
 public:
    TcpServer();

    TcpServer(std::string profile_name);

    TcpServer(YAML::Node config);

    TcpServer(std::string profile_name, Connection* msg_conn);

    TcpServer(YAML::Node config, Connection* msg_conn);

    ~TcpServer();

    void DefaultReadCallback(Connection* conn);

    void DefaultWriteCallback(Connection* conn);
};

} // namespace Imagine_Muduo

#endif