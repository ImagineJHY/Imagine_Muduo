#include "Imagine_Muduo/Buffer.h"

#include "Imagine_Muduo/common_macro.h"

namespace Imagine_Muduo
{

Buffer::Buffer(size_t buffer_size)
{
    buf_.reserve(buffer_size);
    read_idx_ = 0;
    write_idx_ = 0;
    total_size_ = buffer_size;
}

Buffer::~Buffer()
{
}

bool Buffer::Read(int fd)
{
    while (1) {
        char *temp_buf = new char[1000];
        int bytes_num = recv(fd, temp_buf, 1000, 0);
        if (bytes_num == -1) {
            delete[] temp_buf;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // IMAGINE_MUDUO_LOG("数据读取完毕!\n");
                break;
            }

            return false;
        } else if (bytes_num == 0) {
            // 对方关闭连接
            delete[] temp_buf;

            // IMAGINE_MUDUO_LOG("对方关闭连接!");
            return false;
        }

        append(temp_buf, bytes_num);
        delete[] temp_buf;
    }
    // printf("num:%d\n",write_idx-read_idx);

    return true;
}

int Buffer::Write(int fd)
{
    IMAGINE_MUDUO_LOG("this is write func!");
    write(fd, &(buf_[read_idx_]), write_idx_ - read_idx_);

    return 0;
}

void Buffer::append(const char *data, size_t len)
{
    EnsureWritableBytes(len);
    for (size_t i = 0; i < len; i++) {
        buf_[write_idx_ + i] = data[i];
    }
    write_idx_ += len;
}

std::string Buffer::RetrieveAsString(int len)
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

std::string Buffer::RetrieveAllString()
{
    std::string temp_string;
    temp_string.reserve(write_idx_ - read_idx_);
    for (size_t i = read_idx_, j = 0; i < write_idx_; i++, j++) {
        temp_string.push_back(buf_[i]);
    }
    read_idx_ = write_idx_ = 0;

    return temp_string;
}

void Buffer::EnsureWritableBytes(size_t len)
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

const char* Buffer::Peek(size_t idx)
{
    return &buf_[read_idx_ + idx];
}

size_t Buffer::FindFirst(const std::string& target) const
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

const char *Buffer::GetData() const
{
    return &buf_[read_idx_];
}

size_t Buffer::GetLen() const
{
    return write_idx_ - read_idx_;
}

void Buffer::Clear()
{
    read_idx_ = write_idx_ = 0;
    total_size_ = buf_.capacity();
}

void Buffer::Clear(size_t begin_idx, size_t end_idx)
{
    if (begin_idx >= GetLen() || end_idx > GetLen()) {
        IMAGINE_MUDUO_LOG("clear buffer exception read idx is %zu, write idx is %zu, capacity is %zu, total size is %d", read_idx_, write_idx_, buf_.capacity(), total_size_);
        IMAGINE_MUDUO_LOG("clear buffer exception begin idx is %zu, end_idx is %zu", begin_idx, end_idx);
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

    if (1000 < read_idx_ && static_cast<double>(write_idx_ - read_idx_) / buf_.size() < 0.5) {
        for(size_t i = 0; i < write_idx_ - read_idx_; i++) {
            buf_[i] = buf_[read_idx_ + i];
        }
        write_idx_ = write_idx_ - read_idx_;
        read_idx_ = 0;
    }
}

} // namespace Imagine_Muduo
