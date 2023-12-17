#ifndef IMAGINE_MUDUO_CONNECTION_H
#define IMAGINE_MUDUO_CONNECTION_H

#include "common_typename.h"

#include <memory>
#include <string>

namespace Imagine_Muduo
{

class Server;
class Buffer;
class Channel;
class EventLoop;

class Connection
{
 public:
   enum class MessageFormat
   {
      None = 0,
      FixedLenth,
      SpecialEOF
   };

   enum class MessageStatus
   {
      None = 0,
      Complete,
      InComplete,
      OverComplete
   };

   enum class Event
   {
      Read = 0,
      Write,
      ReadAndWrite
   };

 public:
   Connection();

   Connection(std::shared_ptr<Channel> channel);

   Connection(Server* server, std::shared_ptr<Channel> channel);

   virtual ~Connection();

   Connection* Init();

   virtual Connection* Create(const std::shared_ptr<Channel>& channel) const = 0;

   // Connection对于读事件的处理函数, 注册给Channel, 一般使用Acceptor及TcpConnection的默认实现即可
   virtual void ReadHandler() = 0;

   // Connection对于写事件的处理函数, 注册给Channel
   virtual void WriteHandler() = 0;

   // 粘包判断函数
   void PackageCoalescingDetector();

   // Connection对于请求的处理函数(TcpConnection的ReaadHandler会调用ProcessRead, 进而调用该函数, 对读取的消息进行处理, 这里可以写具体的业务逻辑)
   virtual void DefaultReadCallback(Connection* conn) const;

   virtual void DefaultWriteCallback(Connection* conn) const;

   void ProcessRead();

   void ProcessWrite();

   // 使用定长消息
   Connection* const SetMessageFormatWithFixedLength(size_t msg_length, char place_holder = '\0');

   // 使用特殊分隔符
   Connection* const SetMessageFormatWithSpecialEOF(std::string eof = "\0");

   Connection* const ClearMessageFormat();

   int GetSockfd() const;

   Connection* SetIp(const std::string& ip);

   std::string GetIp() const;

   Connection* SetPort(const std::string& port);

   std::string GetPort() const;

   std::string GetPeerIp() const;

   std::string GetPeerPort() const;

   Server* GetServer() const;

   Connection* SetServer(Server* server);

   size_t GetMessageLen() const;

   const char* GetData() const;

   size_t GetLen() const;

   Connection* AppendData(const char* data, size_t len);

   Connection* ClearReadBuffer();

   Connection* ClearWriteBuffer();

   Connection* SetAlive(bool keep_alive);

   Connection* SetRevent(Event revent);

   Connection* SetMessageEndIdx(size_t msg_end_idx);

   Connection* IsTakeNextMessage(bool get_next_msg);

   Connection* IsClearReadBuffer(bool is_clear);

   Connection* IsClearWriteBuffer(bool is_clear);

   Connection* SetReadCallback(ConnectionCallback read_callback);

   Connection* SetWriteCallback(ConnectionCallback write_callback);

   Connection* Close();

   size_t GetUseCount() const;

   Connection* Reset();

 protected:
   Connection* ResetRecvTime();

   Connection* UpdateRevent();

 protected:
   EventLoop* loop_;
   std::shared_ptr<Channel> channel_;
   Server* server_;
   Buffer* read_buffer_;
   Buffer* write_buffer_;
   ConnectionCallback read_callback_;
   ConnectionCallback write_callback_;

 private:
   std::string ip_;
   std::string port_;
   MessageFormat msg_format_;
   size_t msg_length_;
   char place_holder_;
   std::string eof_;

   TimeStamp recv_time;
   MessageStatus msg_status_;
   size_t msg_begin_idx_;
   size_t msg_end_idx_;
   bool keep_alive_;
   Event next_event_;
   bool get_next_msg_;
   bool clear_read_buffer_;
   bool clear_write_buffer_;
};

} // namespace Imagine_Muduo

#endif