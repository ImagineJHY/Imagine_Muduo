#include "Imagine_Muduo/TcpServer.h"

namespace Imagine_Muduo
{

TcpServer::TcpServer() : Server()
{
}

TcpServer::~TcpServer()
{
}

TcpServer::TcpServer(const std::string& profile_name) : Server(profile_name)
{
}

TcpServer::TcpServer(const YAML::Node& config) : Server(config)
{
}

TcpServer::TcpServer(const std::string& profile_name, Connection* msg_conn) : Server(profile_name, msg_conn)
{
}

TcpServer::TcpServer(const YAML::Node& config, Connection* msg_conn) : Server(config, msg_conn)
{
}

void TcpServer::DefaultReadCallback(Connection* conn)
{
    return;
}

void TcpServer::DefaultWriteCallback(Connection* conn)
{
    return;
}

} // namespace Imagine_Muduo