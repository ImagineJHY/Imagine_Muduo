#ifndef IMAGINE_MUDUO_BUFFER_H
#define IMAGINE_MUDUO_BUFFER_H

#include <sys/types.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <functional>

#include "Callbacks.h"

namespace Imagine_Muduo
{

class Buffer
{
    // public:
    //     using EventCommunicateCallback=std::function<bool(const char*,int)>;//粘包判断函数
 public:
    Buffer(int buffer_size = 1000)
    {
        buf_.reserve(buffer_size);
        read_idx_ = 0;
        write_idx_ = 0;
        total_size_ = buffer_size;
    }

    ~Buffer()
    {
        printf("!!!!!!!!!!!!!!!!!!!!!remove buffer:%p\n", this);
    }

    bool Read(int fd, EventCommunicateCallback callback = nullptr)
    {
        while (1) {
            char *temp_buf = new char[1000];
            int bytes_num = recv(fd, temp_buf, 1000, 0);
            // printf("bytes_num : %d\n",bytes_num);
            if (bytes_num == -1) {
                delete[] temp_buf;
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // printf("数据读取完毕!\n");
                    break;
                }

                return false;
            } else if (bytes_num == 0) {
                // 对方关闭连接
                delete[] temp_buf;

                // printf("对方关闭连接!\n");
                return false;
            }

            append(temp_buf, bytes_num);
            delete[] temp_buf;
        }
        // printf("num:%d\n",write_idx-read_idx);

        return true;
    }

    int Write(int fd)
    {
        printf("this is write func!\n");
        write(fd, &(buf_[read_idx_]), write_idx_ - read_idx_);

        return 0;
    }

    void append(const char *data, int len)
    {
        EnsureWritableBytes(len);
        for (int i = 0; i < len; i++) {
            buf_[write_idx_ + i] = data[i];
        }
        write_idx_ += len;
    }

    std::string RetrieveAsString(int len)
    {
        if (write_idx_ - read_idx_ < len) {
            return nullptr;
        }
        std::string temp_string;
        temp_string.reserve(len);
        for (int i = 0; i < len; i++) {
            temp_string[i] = buf_[read_idx_ + i];
        }
        read_idx_ += len;

        return temp_string;
    }

    std::string RetrieveAllString()
    {
        std::string temp_string;
        temp_string.reserve(write_idx_ - read_idx_);
        for (int i = read_idx_, j = 0; i < write_idx_; i++, j++) {
            temp_string.push_back(buf_[i]);
        }
        read_idx_ = write_idx_ = 0;

        return temp_string;
    }

    void EnsureWritableBytes(int len)
    {
        if (total_size_ - write_idx_ < len) {
            if (total_size_ - write_idx_ + read_idx_ < len) {
                buf_.reserve(total_size_ + len);
                total_size_ += len;
            } else {
                for (int i = read_idx_, j = 0; i < write_idx_; i++, j++) {
                    buf_[j] = buf_[i];
                }
                write_idx_ -= read_idx_;
                read_idx_ = 0;
            }
        }
    }

    char *GetData()
    {
        return &buf_[read_idx_];
    }

    int GetLen()
    {
        return write_idx_ - read_idx_;
    }

    void Clear()
    {
        read_idx_ = write_idx_ = 0;
        total_size_ = buf_.capacity();
    }

 private:
    std::vector<char> buf_;
    int read_idx_;
    int write_idx_;
    int total_size_;
};

} // namespace Imagine_Muduo

#endif