#ifndef IMAGINE_MUDUO_TCPSERVER_H
#define IMAGINE_MUDUO_TCPSERVER_H

#include "Server.h"

namespace Imagine_Muduo
{

class TcpServer : public Server
{
 public:
    TcpServer();

    TcpServer(const std::string& profile_name);

    TcpServer(const YAML::Node& config);

    TcpServer(const std::string& profile_name, Connection* msg_conn);

    TcpServer(const YAML::Node& config, Connection* msg_conn);

    ~TcpServer();

    void DefaultReadCallback(Connection* conn);

    void DefaultWriteCallback(Connection* conn);
};

} // namespace Imagine_Muduo

#endif