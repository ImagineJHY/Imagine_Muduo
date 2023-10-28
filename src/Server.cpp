#include "Imagine_Muduo/Server.h"

namespace Imagine_Muduo
{

Server::Server()
{
}

Server::Server(std::string profile_name, Connection* acceptor, Connection* msg_conn) : loop_(new EventLoop(profile_name)), acceptor_(acceptor), msg_conn_(msg_conn)
{
    Init();
}

Server::Server(YAML::Node config, Connection* acceptor, Connection* msg_conn) : loop_(new EventLoop(config)), acceptor_(acceptor), msg_conn_(msg_conn)
{
    Init();
}
    
Server::~Server()
{
    delete acceptor_;
}

void Server::Init()
{
    if (acceptor_ != nullptr) {
        acceptor_->SetServer(this);
        msg_conn_->SetServer(this);
        read_callback_ = std::bind(&Connection::DefaultReadCallback, msg_conn_, std::placeholders::_1);
        write_callback_ = std::bind(&Connection::DefaultWriteCallback, msg_conn_, std::placeholders::_1);
    }
    if (loop_ != nullptr) {
        Connection*  old_acceptor = acceptor_;
        acceptor_ = acceptor_->Create(loop_->GetListenChannel());
        delete old_acceptor;
    }
}

void Server::Start()
{
    loop_->loop();
}

Server* const Server::AddAndSetConnection(Connection* new_conn)
{
    SetReadCallback(new_conn);
    SetWriteCallback(new_conn);
    AddConnection(new_conn);

    return this;
}

Server* const Server::AddConnection(Connection* new_conn)
{
    std::pair<std::string, std::string> pair = std::make_pair(new_conn->GetIp(), new_conn->GetPort());
    std::unique_lock<std::mutex> lock(map_lock_);
    if (conn_map_.find(pair) == conn_map_.end()) {
        conn_map_.insert(std::make_pair(std::make_pair(new_conn->GetIp(), new_conn->GetPort()), new_conn));
    }

    return this;
}

Server* const Server::SetReadCallback(Connection* new_conn)
{
    new_conn->SetReadCallback(read_callback_);
}

Server* const Server::SetWriteCallback(Connection* new_conn)
{
    new_conn->SetWriteCallback(write_callback_);
}

Connection* Server::GetMessageConnection() const
{
    return msg_conn_;
}

Server* const Server::CloseConnection(std::string ip, std::string port)
{
    Connection* del_conn = RemoveConnection(ip, port);
    if (del_conn != nullptr) {
        del_conn->Close();
    }

    return this;
}

Connection* const Server::RemoveConnection(std::string ip, std::string port)
{
    std::unique_lock<std::mutex> lock(map_lock_);
    auto it = conn_map_.find(std::make_pair(ip, port));
    if (it != conn_map_.end()) {
        conn_map_.erase(it);

        return it->second;
    }

    return nullptr;
}

} // namespace Imagine_Muduo