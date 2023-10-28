#include "Imagine_Muduo/TcpConnection.h"

namespace Imagine_Muduo
{

TcpConnection::TcpConnection() : Connection()
{
    // channel_->SetReadCallback(std::bind(&ReadHandler, this));
    // channel_->SetWriteCallback(std::bind(&WriteHandler, this));
}

TcpConnection::TcpConnection(std::shared_ptr<Channel> channel) : Connection(channel)
{
    // channel_->SetReadCallback(std::bind(&Connection::ReadHandler, this));
    // channel_->SetWriteCallback(std::bind(&Connection::WriteHandler, this));
}

TcpConnection::TcpConnection(Server* server, std::shared_ptr<Channel> channel) : Connection(server, channel)
{
}

TcpConnection::~TcpConnection()
{
}

Connection* TcpConnection::Create(std::shared_ptr<Channel> channel)
{
    return new TcpConnection(server_, channel);
}

void TcpConnection::ReadHandler()
{
    LOG_INFO("Hello! This is TcpConnection!");
    if (!read_buffer_.Read(channel_->Getfd())) {
        LOG_INFO("close channel:%d", channel_->Getfd());
        Close();
        return;
    }
    ResetRecvTime();
    ProcessRead();
}

void TcpConnection::WriteHandler()
{
    ProcessWrite();
}

void TcpConnection::DefaultReadCallback(Connection* conn)
{
    LOG_INFO("this is TcpConnection DefaultReadCallback!");
    return;
}

void TcpConnection::DefaultWriteCallback(Connection* conn)
{
    return;
}

} // namespace Imagine_Muduo