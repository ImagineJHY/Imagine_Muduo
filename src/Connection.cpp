#include "Imagine_Muduo/Connection.h"
#include "Imagine_Muduo/Server.h"

namespace Imagine_Muduo
{

Connection::Connection() : channel_(nullptr), server_(nullptr)
{
    Init();
}

Connection::Connection(std::shared_ptr<Channel> channel) : channel_(channel)
{
    Init();
}

Connection::Connection(Server* server, std::shared_ptr<Channel> channel) : channel_(channel), server_(server)
{
    Init();
}

Connection::~Connection()
{
}

Connection* const Connection::Init()
{
    msg_format_ = MessageFormat::None;
    msg_status_ = MessageStatus::None;
    keep_alive_ = true;
    next_event_ = Event::Read;
    get_next_msg_ = false;
    clear_read_buffer_ = true;
    clear_write_buffer_ = true;
    if (channel_.get() != nullptr) {
        loop_ = channel_->GetLoop();
        channel_->SetReadHandler(std::bind(&Connection::ReadHandler, this));
        channel_->SetWriteHandler(std::bind(&Connection::WriteHandler, this));
    }
    SetReadCallback(std::bind(&Connection::DefaultReadCallback, this, std::placeholders::_1));
    SetWriteCallback(std::bind(&Connection::DefaultWriteCallback, this, std::placeholders::_1));

    return this;
}

void Connection::PackageCoalescingDetector()
{
    size_t read_size = read_buffer_.GetLen();
    msg_begin_idx_ = 0;
    msg_end_idx_ = read_size;
    if (msg_format_ == MessageFormat::None) {
        return;
    }
    switch (msg_format_) {
        case MessageFormat::None:
            {
                break;
            }
        case MessageFormat::FixedLenth:
            {
                if (read_size < msg_length_) {
                    msg_status_ = MessageStatus::InComplete;
                } else if (read_size > msg_length_) {
                    msg_status_ = MessageStatus::OverComplete;
                    msg_end_idx_ = msg_length_;
                    for(size_t idx = msg_end_idx_ - 1; idx >= msg_begin_idx_; idx--) {
                        if (*(read_buffer_.Peek(idx)) != place_holder_) {
                            msg_end_idx_ = idx + 1;
                            break;
                        }
                    }
                } else {
                    msg_status_ = MessageStatus::Complete;
                    msg_end_idx_ = msg_length_;
                }
                break;
            }
        case MessageFormat::SpecialEOF:
            {
                msg_end_idx_ = read_buffer_.FindFirst(eof_) + eof_.size();
                if (msg_end_idx_ > read_size) {
                    msg_status_ = MessageStatus::InComplete;
                } else if (msg_end_idx_ < read_size) {
                    msg_status_ = MessageStatus::OverComplete;
                } else {
                    msg_status_ = MessageStatus::Complete;
                }
                break;
            }
    }
}

void Connection::DefaultReadCallback(Connection* conn)
{
    return;
}

void Connection::DefaultWriteCallback(Connection* conn)
{
    return;
}

void Connection::ProcessRead()
{
    do {
        PackageCoalescingDetector();
        read_callback_(this);
        read_buffer_.Clear(msg_begin_idx_, msg_end_idx_);
    } while (get_next_msg_);
    UpdateRevent();
}

void Connection::ProcessWrite()
{
    write_callback_(this);
    UpdateRevent();
}

Connection* const Connection::SetMessageFormatWithFixedLength(size_t msg_length, char place_holder)
{
    msg_format_ = MessageFormat::FixedLenth;
    msg_length_ = msg_length;
    place_holder_ = place_holder;

    return this;
}

Connection* const Connection::SetMessageFormatWithSpecialEOF(std::string eof)
{
    msg_format_ = MessageFormat::SpecialEOF;
    eof_ = eof;

    return this;
}

Connection* const Connection::ClearMessageFormat()
{
    msg_format_ = MessageFormat::None;

    return this;
}

Connection* Connection::SetIp(const std::string& ip)
{
    ip_ = ip;

    return this;
}

std::string Connection::GetIp() const
{
    return ip_;
}

Connection* Connection::SetPort(const std::string& port)
{
    port_ = port;

    return this;
}

std::string Connection::GetPort() const
{
    return port_;
}

Connection* const Connection::SetServer(Server* const server)
{
    server_ = server;
}

Server* Connection::GetServer() const
{
    return server_;
}

Connection* const Connection::SetReadCallback(ConnectionCallback read_callback)
{
    read_callback_ = read_callback;

    return this;
}

Connection* const Connection::SetWriteCallback(ConnectionCallback write_callback)
{
    write_callback_ = write_callback;

    return this;
}

Connection* const Connection::Close()
{
    channel_->Close();
    channel_.reset();
    server_->RemoveConnection(ip_, port_);

    return this;
}

Connection* const Connection::ResetRecvTime()
{
    recv_time.SetTime(NOW_MS);

    return this;
}

Connection* const Connection::UpdateRevent()
{
    if (clear_read_buffer_) {
        read_buffer_.Clear();
    }
    if (clear_write_buffer_) {
        write_buffer_.Clear();
    }
    if (!keep_alive_) {
        Close();
        return this;
    }
    switch (next_event_) {
        case Event::Read:
            channel_->SetEvents(EPOLLIN | EPOLLONESHOT | EPOLLRDHUP);
            break;
        case Event::Write:
            channel_->SetEvents(EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP);
            break;
        default:
            Close();
    }
    
    return this;
}

} // namesapce Imagine_Muduo