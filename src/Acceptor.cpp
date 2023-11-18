#include "Imagine_Muduo/Acceptor.h"
#include "Imagine_Muduo/Server.h"

namespace Imagine_Muduo
{

Acceptor::Acceptor() : Connection()
{
}

Acceptor::Acceptor(Connection* msg_conn) : Connection(), msg_conn_(msg_conn)
{
}

Acceptor::Acceptor(Server* server, Connection* msg_conn) : Connection(), msg_conn_(msg_conn)
{
}

Acceptor::Acceptor(Server* server, std::shared_ptr<Channel> channel) : Connection(server, channel)
{
    // LOG_INFO("setting!");
    // channel_->SetReadCallback(std::bind(&Acceptor::ReadHandler, this));
    // channel_->SetWriteCallback(std::bind(&Acceptor::WriteHandler, this));
}

Acceptor::Acceptor(Server* server, std::shared_ptr<Channel> channel, Connection* msg_conn) : Connection(server, channel), msg_conn_(msg_conn)
{
}

Acceptor::~Acceptor()
{
}

Connection* Acceptor::Create(std::shared_ptr<Channel> channel)
{
    return new Acceptor(server_, channel, server_->GetMessageConnection());
}

void Acceptor::ReadHandler()
{
    LOG_INFO("this is Acceptor's ReadHandler!");
    if (loop_->GetChannelnum() >= loop_->GetMaxchannelnum()) {
        LOG_INFO("channel num over quantity! channel num is %d, max_channel_num is %d", loop_->GetChannelnum(), loop_->GetMaxchannelnum());
        return;
    } else {
        std::shared_ptr<Channel> channel = Channel::Create(loop_, channel_->Getfd());
        if (channel == nullptr) {
            return;
        }
        channel->ParsePeerAddr();
        LOG_INFO("%s:%s", channel->GetPeerIp(), channel->GetPeerPort());
        Connection* new_conn = CreateMessageConnection(channel);
        if (server_ != nullptr) {
            server_->AddAndSetConnection(new_conn);
        }
        loop_->GetEpoll()->AddChannel(channel);
        loop_->AddChannelnum();
    }
    channel_->SetEvents(EPOLLIN | EPOLLONESHOT | EPOLLRDHUP);
    // printf("this is listenfd!\n");
}

void Acceptor::WriteHandler()
{
    return;
}

void Acceptor::DefaultReadCallback(Connection* conn)
{
    LOG_INFO("hhhhhhhhhh");
    std::shared_ptr<Channel> channel = Channel::Create(loop_, channel_->Getfd());
    if (channel == nullptr) {
        return;
    }
    loop_->GetEpoll()->AddChannel(channel);
    loop_->AddChannelnum();
    Connection* new_conn = this->CreateMessageConnection(channel);
    if (server_ != nullptr) {
        server_->AddAndSetConnection(new_conn);
    }
}

void Acceptor::DefaultWriteCallback(Connection* conn)
{
    return;
}

Connection* Acceptor::CreateMessageConnection(std::shared_ptr<Channel> channel)
{
    return msg_conn_->Create(channel);
}

Acceptor* const Acceptor::SetMessageConnection(Connection* msg_conn)
{
    msg_conn_ = msg_conn;

    return this;
}

} // namespace Imagine_Muduo