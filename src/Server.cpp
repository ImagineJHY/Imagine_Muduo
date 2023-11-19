#include "Imagine_Muduo/Server.h"

namespace Imagine_Muduo
{

Server::Server()
{
}

Server::Server(std::string profile_name, Connection* msg_conn, Connection* acceptor) : loop_(new EventLoop(profile_name)), acceptor_(acceptor), msg_conn_(msg_conn)
{
    Init();
}

Server::Server(YAML::Node config, Connection* msg_conn, Connection* acceptor) : loop_(new EventLoop(config)), acceptor_(acceptor), msg_conn_(msg_conn)
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

    destroy_thread_ = new pthread_t;
    if (!destroy_thread_) {
        throw std::exception();
    }
}

void Server::Start()
{
    if (pthread_create(destroy_thread_, nullptr, [](void *argv) -> void *
        {
            Server* server = (Server*)argv;
            while(1) {
                server->DestroyConnection();
            }

            return nullptr;
        }, this) != 0) {
        delete[] destroy_thread_;
        LOG_INFO("loop exception!");
        throw std::exception();
    }

    if (pthread_detach(*destroy_thread_)) {
        delete[] destroy_thread_;
        LOG_INFO("destroy exception");
        throw std::exception();
    }
    loop_->loop();
}

long long Server::SetTimer(Imagine_Tool::TimerCallback timer_callback, double interval, double delay)
{
    return loop_->SetTimer(timer_callback, interval, delay);
}

Server* const Server::RemoveTimer(long long timerfd)
{
    loop_->CloseTimer(timerfd);

    return this;
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
    std::pair<std::string, std::string> pair = std::make_pair(new_conn->GetPeerIp(), new_conn->GetPeerPort());
    std::unique_lock<std::mutex> lock(map_lock_);
    if (conn_map_.find(pair) == conn_map_.end()) {
        LOG_INFO("Add Connection %p", new_conn);
        conn_map_.insert(std::make_pair(std::make_pair(new_conn->GetPeerIp(), new_conn->GetPeerPort()), new_conn));
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

void Server::DestroyConnection()
{
    pthread_mutex_lock(&destroy_lock_);
    while(close_list_.empty()) {
        Connection* del_connection = close_list_.back();
        close_list_.pop_back();
        pthread_mutex_unlock(&destroy_lock_);
        while(del_connection->GetUseCount() > 1);
        LOG_INFO("Connection Use Count is 1");
        del_connection->Reset();
        LOG_INFO("DELETE Connection %p", del_connection);
        delete del_connection;
        pthread_mutex_lock(&destroy_lock_);
    }
    pthread_mutex_unlock(&destroy_lock_);
}

Server* const Server::CloseConnection(std::string ip, std::string port)
{
    Connection* del_conn = RemoveConnection(ip, port);
    if (del_conn != nullptr) {
        del_conn->Close();
        pthread_mutex_lock(&destroy_lock_);
        close_list_.push_back(del_conn);
        pthread_mutex_unlock(&destroy_lock_);
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