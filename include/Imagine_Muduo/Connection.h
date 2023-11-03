#ifndef IMAGINE_MUDUO_CONNECTION_H
#define IMAGINE_MUDUO_CONNECTION_H

#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"

#include <memory>
#include <string>

namespace Imagine_Muduo
{

class Server;

class Connection
{
   enum class MessageFormat{
      None = 0,
      FixedLenth,
      SpecialEOF
   };

   enum class MessageStatus{
      None = 0,
      Complete,
      InComplete,
      OverComplete
   };

   enum class Event{
      Read = 0,
      Write,
      ReadAndWrite
   };

 public:
   Connection();

   Connection(std::shared_ptr<Channel> channel);

   Connection(Server* server, std::shared_ptr<Channel> channel);

   virtual ~Connection();

   Connection* const Init();

   virtual Connection* Create(std::shared_ptr<Channel> channel) = 0;

   // Channel的读回调函数
   virtual void ReadHandler() = 0;

   // Channel的写回调函数
   virtual void WriteHandler() = 0;

   // 粘包判断函数
   void PackageCoalescingDetector();

   virtual void DefaultReadCallback(Connection* conn);

   virtual void DefaultWriteCallback(Connection* conn);

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

   Server* GetServer() const;

   Connection* const SetServer(Server* const server);

   size_t GetMessageLen() const;

   const char* GetData() const;

   size_t GetLen() const;

   Connection* const AppendData(const char* data, size_t len);

   Connection* const ClearReadBuffer();

   Connection* const ClearWriteBuffer();

   Connection* const SetAlive(bool keep_alive);

   Connection* const SetRevent(Event revent);

   Connection* const IsTakeNextMessage(bool get_next_msg);

   Connection* const IsClearReadBuffer(bool is_clear);

   Connection* const IsClearWriteBuffer(bool is_clear);

   Connection* const SetReadCallback(ConnectionCallback read_callback);

   Connection* const SetWriteCallback(ConnectionCallback write_callback);

   Connection* const Close();

 protected:
   Connection* const ResetRecvTime();

   Connection* const UpdateRevent();

 protected:
   EventLoop* loop_;
   std::shared_ptr<Channel> channel_;
   Server* server_;
   Buffer read_buffer_;
   Buffer write_buffer_;
   ConnectionCallback read_callback_;
   ConnectionCallback write_callback_;

 private:
   std::string ip_;
   std::string port_;
   MessageFormat msg_format_;
   size_t msg_length_;
   char place_holder_;
   std::string eof_;

   Imagine_Tool::TimeStamp recv_time;
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