#include "Imagine_Muduo/TcpConnection.h"

#include "Imagine_Muduo/common_macro.h"
#include "Imagine_Muduo/Server.h"
#include "Imagine_Muduo/Buffer.h"
#include "Imagine_Muduo/Channel.h"

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

Connection* TcpConnection::Create(const std::shared_ptr<Channel>& channel) const
{
    return new TcpConnection(server_, channel);
}

void TcpConnection::ReadHandler()
{
    IMAGINE_MUDUO_LOG("Hello! This is TcpConnection!");
    if (!read_buffer_->Read(channel_->Getfd())) {
        IMAGINE_MUDUO_LOG("close channel:%d", channel_->Getfd());
        server_->CloseConnection(GetPeerIp(), GetPeerPort());
        // Close();
        return;
    }
    ResetRecvTime();
    ProcessRead();
}

void TcpConnection::WriteHandler()
{
    ProcessWrite();
}

void TcpConnection::DefaultReadCallback(Connection* conn) const
{
    IMAGINE_MUDUO_LOG("this is TcpConnection DefaultReadCallback!");
    return;
}

void TcpConnection::DefaultWriteCallback(Connection* conn) const
{
    return;
}

} // namespace Imagine_Muduo