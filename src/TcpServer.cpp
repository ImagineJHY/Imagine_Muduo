#include "Imagine_Muduo/TcpServer.h"

namespace Imagine_Muduo
{

TcpServer::TcpServer() : Server()
{
}

TcpServer::~TcpServer()
{
}

TcpServer::TcpServer(std::string profile_name) : Server(profile_name)
{
}

TcpServer::TcpServer(YAML::Node config) : Server(config)
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