#ifndef IMAGINE_MUDUO_BUFFER_H
#define IMAGINE_MUDUO_BUFFER_H

#include <sys/types.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <functional>
#include <unistd.h>

namespace Imagine_Muduo
{

class Buffer
{
 public:
    Buffer(size_t buffer_size = 1000);

    ~Buffer();

    bool Read(int fd);

    int Write(int fd);

    void append(const char *data, size_t len);

    std::string RetrieveAsString(int len);

    std::string RetrieveAllString();

    void EnsureWritableBytes(size_t len);

    const char* Peek(size_t idx);

    size_t FindFirst(const std::string& target) const;

    const char *GetData() const;

    size_t GetLen() const;

    void Clear();

    void Clear(size_t begin_idx, size_t end_idx);

 private:
    std::vector<char> buf_;
    size_t read_idx_;
    size_t write_idx_;
    size_t total_size_;
};

} // namespace Imagine_Muduo

#endif