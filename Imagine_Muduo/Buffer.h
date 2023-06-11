#ifndef IMAGINE_MUDUO_BUFFER_H
#define IMAGINE_MUDUO_BUFFER_H

#include<sys/types.h>
#include<string>
#include<vector>
#include<sys/socket.h>
#include<functional>

#include"Callbacks.h"

namespace Imagine_Muduo{

class Buffer{
    
// public:
//     using EventCommunicateCallback=std::function<bool(const char*,int)>;//粘包判断函数

public:

    Buffer(int size=1000){
        buf.reserve(size);
        read_idx=0;
        write_idx=0;
        total_size=size;
    }

    bool Read(int fd, EventCommunicateCallback callback_=nullptr){

        while(1){
            char* temp_buf=new char[1000];
            int bytes_num=recv(fd,temp_buf,1000,0);
            //printf("bytes_num : %d\n",bytes_num);
            if(bytes_num==-1){
                delete [] temp_buf;
                if(errno==EAGAIN||errno==EWOULDBLOCK){
                    //printf("数据读取完毕!\n");
                    break;
                }

                return false;
            }else if(bytes_num==0){//对方关闭连接
                delete [] temp_buf;

                //printf("对方关闭连接!\n");
                return false;
            }

            append(temp_buf,bytes_num);
            delete [] temp_buf;
        }
        //printf("num:%d\n",write_idx-read_idx);

        return true;
    }

    ssize_t Write(int fd){
        printf("this is write func!\n");
        write(fd,&(buf[read_idx]),write_idx-read_idx);

        return 0;
    }

    void append(const char* data, size_t len){
        EnsureWritableBytes(len);
        for(int i=0;i<len;i++){
            buf[write_idx+i]=data[i];
        }
        write_idx+=len;
    }

    std::string RetrieveAsString(size_t len){
        if(write_idx-read_idx<len){
            return nullptr;
        }
        std::string s;
        s.reserve(len);
        for(int i=0;i<len;i++){
            s[i]=buf[read_idx+i];
        }
        read_idx+=len;

        return s;
    }

    std::string RetrieveAllString(){
        std::string s;
        s.reserve(write_idx-read_idx);
        for(int i=read_idx,j=0;i<write_idx;i++,j++){
            s.push_back(buf[i]);
        }
        read_idx=write_idx=0;

        return s;
    }

    void EnsureWritableBytes(size_t len){
        if(total_size-write_idx<len){
            if(total_size-write_idx+read_idx<len){
                buf.reserve(total_size+len);
                total_size+=len;
            }else{
                for(int i=read_idx,j=0;i<write_idx;i++,j++){
                    buf[j]=buf[i];
                }
                write_idx-=read_idx;
                read_idx=0;
            }
        }
    }

    char* GetData(){
        return &buf[read_idx];
    }

    int GetLen(){
        return write_idx-read_idx;
    }

    void Clear(){
        read_idx=write_idx=0;
        total_size=buf.capacity();
    }

private:

    std::vector<char> buf;
    int read_idx;
    int write_idx;
    int total_size;

};



}




#endif