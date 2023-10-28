#ifndef IMAGINE_MUDUO_SERVER_H
#define IMAGINE_MUDUO_SERVER_H

#include "Acceptor.h"
#include "TcpConnection.h"

#include <unordered_map>

namespace Imagine_Muduo
{

class Server
{
 public:
    Server();

    Server(std::string profile_name, Connection* acceptor = new Acceptor(), Connection* msg_conn = new TcpConnection());

    Server(YAML::Node config, Connection* acceptor = new Acceptor(), Connection* msg_conn = new TcpConnection());

    virtual ~Server();

    void Init();

    void Start();

    virtual void DefaultReadCallback(Connection* conn) = 0;

    virtual void DefaultWriteCallback(Connection* conn) = 0;

    Server* const AddAndSetConnection(Connection* new_conn);

    Server* const AddConnection(Connection* new_conn);

    Server* const SetReadCallback(Connection* new_conn);

    Server* const SetWriteCallback(Connection* new_conn);

    Connection* GetMessageConnection() const;

    Server* const CloseConnection(std::string ip, std::string port);

    Connection* const RemoveConnection(std::string ip, std::string port);

 private:
    EventLoop* loop_;
    Connection* acceptor_;
    Connection* msg_conn_;
    std::unordered_map<std::pair<std::string, std::string>, Connection*, HashPair, EqualPair> conn_map_;
    std::mutex map_lock_;
    ConnectionCallback read_callback_;
    ConnectionCallback write_callback_;
};

} // namespace Imagine_Muduo

#endif