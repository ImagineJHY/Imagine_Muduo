#ifndef IMAGINE_MUDUO_BUFFER_H
#define IMAGINE_MUDUO_BUFFER_H

#include "Imagine_Log/Logger.h"
#include "Imagine_Log/SingletonLogger.h"
#include "Imagine_Muduo/common_definition.h"

#include <sys/types.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <functional>

namespace Imagine_Muduo
{

class Buffer
{
 public:
    Buffer(size_t buffer_size = 1000)
    {
        buf_.reserve(buffer_size);
        read_idx_ = 0;
        write_idx_ = 0;
        total_size_ = buffer_size;
    }

    ~Buffer()
    {
        // LOG_INFO("!!!!!!!!!!!!!!!!!!!!!remove buffer:%p", this);
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
                    // LOG_INFO("数据读取完毕!\n");
                    break;
                }

                return false;
            } else if (bytes_num == 0) {
                // 对方关闭连接
                delete[] temp_buf;

                // LOG_INFO("对方关闭连接!");
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
        LOG_INFO("this is write func!");
        write(fd, &(buf_[read_idx_]), write_idx_ - read_idx_);

        return 0;
    }

    void append(const char *data, size_t len)
    {
        EnsureWritableBytes(len);
        for (size_t i = 0; i < len; i++) {
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
        for (size_t i = 0; i < len; i++) {
            temp_string[i] = buf_[read_idx_ + i];
        }
        read_idx_ += len;

        return temp_string;
    }

    std::string RetrieveAllString()
    {
        std::string temp_string;
        temp_string.reserve(write_idx_ - read_idx_);
        for (size_t i = read_idx_, j = 0; i < write_idx_; i++, j++) {
            temp_string.push_back(buf_[i]);
        }
        read_idx_ = write_idx_ = 0;

        return temp_string;
    }

    void EnsureWritableBytes(size_t len)
    {
        if (total_size_ - write_idx_ < len) {
            if (total_size_ - write_idx_ + read_idx_ < len) {
                buf_.reserve(total_size_ + len);
                total_size_ += len;
            } else {
                for (size_t i = read_idx_, j = 0; i < write_idx_; i++, j++) {
                    buf_[j] = buf_[i];
                }
                write_idx_ -= read_idx_;
                read_idx_ = 0;
            }
        }
    }

    const char* Peek(size_t idx)
    {
        return &buf_[read_idx_ + idx];
    }

    size_t FindFirst(std::string target)
    {
        if (target.size() == 0) {
            return write_idx_ - read_idx_;
        }
        for(size_t i = read_idx_; i < write_idx_; i++) {
            if(buf_[i] == target[0]) {
                bool flag = true;
                for(size_t j = i; j < target.size(); j++) {
                    if (buf_[j] != target[j - i]) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    return i - read_idx_;
                }
            }
        }
        return write_idx_ - read_idx_;
    }

    const char *GetData() const
    {
        return &buf_[read_idx_];
    }

    size_t GetLen() const
    {
        return write_idx_ - read_idx_;
    }

    void Clear()
    {
        read_idx_ = write_idx_ = 0;
        total_size_ = buf_.capacity();
    }

    void Clear(size_t begin_idx, size_t end_idx)
    {
        if (begin_idx >= GetLen() || end_idx > GetLen()) {
            LOG_INFO("clear buffer exception read idx is %zu, write idx is %zu, capacity is %zu, total size is %d", read_idx_, write_idx_, buf_.capacity(), total_size_);
            LOG_INFO("clear buffer exception begin idx is %zu, end_idx is %zu", begin_idx, end_idx);
            throw std::exception();
        }
        if (begin_idx == 0) {
            read_idx_ = read_idx_ + end_idx;
        } else {
            if (begin_idx < write_idx_ - read_idx_ - end_idx) {
                for(size_t i = read_idx_ + begin_idx - 1; i >= read_idx_; i--) {
                    buf_[i + end_idx - begin_idx] = buf_[i];
                }
                read_idx_ = read_idx_ + (end_idx - begin_idx);
            } else {
                for(size_t i = read_idx_ + end_idx; i < write_idx_; i++) {
                    buf_[i - (end_idx - begin_idx)] = buf_[i];
                }
                write_idx_ = write_idx_ - (end_idx - begin_idx);
            }
        }
    }

 private:
    std::vector<char> buf_;
    size_t read_idx_;
    size_t write_idx_;
    size_t total_size_;
};

} // namespace Imagine_Muduo

#endif