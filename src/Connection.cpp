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
        if (clear_read_buffer_) {
            read_buffer_.Clear(msg_begin_idx_, msg_end_idx_);
            LOG_INFO("Clear read buffer from %d to %d, buffer size is %d", msg_begin_idx_, msg_end_idx_, read_buffer_.GetLen());
        }
    } while (get_next_msg_);
    UpdateRevent();
}

void Connection::ProcessWrite()
{
    write_callback_(this);
    write_buffer_.Write(channel_->Getfd());
    LOG_INFO("write data %sb", write_buffer_.GetLen());
    if (clear_write_buffer_) {
        write_buffer_.Clear();
    }
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

int Connection::GetSockfd() const
{
    return channel_->Getfd();
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

Server* Connection::GetServer() const
{
    return server_;
}

Connection* const Connection::SetServer(Server* const server)
{
    server_ = server;
}

size_t Connection::GetMessageLen() const
{
    return msg_end_idx_ - msg_begin_idx_;
}

const char* Connection::GetData() const
{
    return read_buffer_.GetData();
}

size_t Connection::GetLen() const
{
    return read_buffer_.GetLen();
}

Connection* const Connection::AppendData(const char* data, size_t len)
{
    write_buffer_.append(data, len);

    return this;
}

Connection* const Connection::ClearReadBuffer()
{
    read_buffer_.Clear();

    return this;
}

Connection* const Connection::ClearWriteBuffer()
{
    write_buffer_.Clear();

    return this;
}

Connection* const Connection::SetAlive(bool keep_alive)
{
    keep_alive_ = keep_alive;
    
    return this;
}

Connection* const Connection::SetRevent(Connection::Event revent)
{
    next_event_ = revent;

    return this;
}

Connection* const Connection::SetMessageEndIdx(size_t msg_end_idx)
{
    if (msg_end_idx < msg_begin_idx_ || read_buffer_.GetLen() < msg_end_idx) {
        throw std::exception();
    }
    msg_end_idx_ = msg_end_idx;

    return this;
}

Connection* const Connection::IsTakeNextMessage(bool get_next_msg)
{
    get_next_msg_ = get_next_msg;

    return this;
}

Connection* const Connection::IsClearReadBuffer(bool is_clear)
{
    clear_read_buffer_ = is_clear;

    return this;
}

Connection* const Connection::IsClearWriteBuffer(bool is_clear)
{
    clear_write_buffer_ = is_clear;

    return this;
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

    return nullptr;
}

size_t const Connection::GetUseCount() const
{
    return channel_.use_count();
}

Connection* const Connection::Reset()
{
    channel_.reset();

    return this;
}

Connection* const Connection::ResetRecvTime()
{
    recv_time.SetTime(NOW_MS);

    return this;
}

Connection* const Connection::UpdateRevent()
{
    if (!keep_alive_) {
        server_->CloseConnection(ip_, port_);
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
            server_->CloseConnection(ip_, port_);
            break;
    }
    
    return this;
}

} // namesapce Imagine_Muduo