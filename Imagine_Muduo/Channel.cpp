#include"Channel.h"

#include<memory>
#include<sys/uio.h>

#include"EventLoop.h"
#include"Buffer.h"
#include"TimeUtil.h"
#include"Poller.h"

using namespace Imagine_Muduo;

void Channel::MakeSelf(std::shared_ptr<Channel> self_){
    self=self_;
}

void Channel::SetReadCallback(EventCallback callback){
    read_callback=std::move(callback);
}

void Channel::SetWriteCallback(EventCallback callback){
    write_callback=std::move(callback);
}

void Channel::SetCloseCallback(EventCallback callback){
    close_callback=std::move(callback);
}

void Channel::SetErrorCallback(EventCallback callback){
    error_callback=std::move(callback);
}

void Channel::SetCommunicateCallback(EventCommunicateCallback callback){
    communicate_callback=callback;
}

void Channel::EnableRead(){
    events|=EPOLLIN;
    Update();
}

void Channel::EnableWrite(){
    events|=EPOLLOUT;
    Update();
}

void Channel::DisableRead(){
    events&=~EPOLLIN;
    Update();
}

void Channel::DisableWrite(){
    events&=~EPOLLOUT;
    Update();
}

int Channel::SetRevents(int revents_){
    revents=revents_;

    return 0;
}

int Channel::SetEvents(int events_){
    events=events_;
    Update();

    return 0;
}

int Channel::GetRevents(){
    return revents;
}

int Channel::GetEvents(){
    return events;
}

void Channel::InitIovec(struct iovec* read_str,struct sockaddr_in* addr, bool get_read_buf_){

    read_str[0].iov_base=&fd;
    read_str[0].iov_len=3;
    socklen_t addr_size=sizeof(*addr);
    getpeername(fd,(struct sockaddr*)addr,&addr_size);
    read_str[1].iov_base=addr;
    read_str[1].iov_len=addr_size;
    if(get_read_buf_){
        read_str[2].iov_base=read_buffer.GetData();
        read_str[2].iov_len=read_buffer.GetLen();
    }else{
        read_str[2].iov_base=write_buffer.GetData();
        read_str[2].iov_len=write_buffer.GetLen();
    }

}

void Channel::ProcessIovec(struct iovec* io_block){
    if(!io_block)return;

    int block_num=io_block[0].iov_len;
    char* flag_char=(char*)(io_block[0].iov_base)+block_num;
    if(*flag_char=='1')alive=true;
    else alive=false;
    read_or_write=0;
    if(*(flag_char+1)=='1'){//发生粘包
        clear_readbuf=false;
    }else clear_readbuf=true;

    if(*(flag_char+2)=='1'){
        read_or_write|=EPOLLIN;
    }else if(*(flag_char+3)=='1'){
        read_or_write|=EPOLLOUT;
    }

    for(int i=block_num-1;i>0;i--){
        if(*((char*)(io_block[0].iov_base)+i)=='1')delete [] (char*)(io_block[i].iov_base);
    }

    delete [] (char*)(io_block[0].iov_base);
    delete [] io_block;
}

void Channel::Setfd(int fd_){
    fd=fd_;
}

int Channel::Getfd(){
    return fd;
}

void Channel::SetAddr(struct sockaddr_in& addr){
    client_addr=addr;
}

struct sockaddr_in Channel::GetAddr(){
    return client_addr;
}

void Channel::SetLoop(EventLoop* loop_){
    loop=loop_;
}

EventLoop* Channel::GetLoop(){
    return loop;
}

void Channel::SetNonBlocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
}

std::shared_ptr<Channel> Channel::Create(EventLoop* loop_, int value, int type){

    if(value<0){
        throw std::exception();
    }

    int reuse=1;
    int sockfd;
    struct sockaddr_in saddr;
    int events;
    int listenfd;
    std::shared_ptr<Channel> new_channel=std::make_shared<Channel>();
    new_channel->SetEventHandler(std::bind(&Channel::DefaultEventHandler,new_channel.get()));

    if(type==1){//创建通信Channel
        new_channel->SetReadEventHandler(std::bind(&Channel::DefaultEventfdReadEventHandler,new_channel.get()));
        new_channel->SetWriteEventHandler(std::bind(&Channel::DefaultEventfdWriteEventHandler,new_channel.get()));
        listenfd=value;
        socklen_t saddr_len=sizeof(saddr);
        sockfd=accept(listenfd,(struct sockaddr*)&saddr,&saddr_len);
        if(sockfd<0){
            // printf("sockfd is %d",sockfd);
            // printf("errno is %d\n",errno);
            // printf("Create exception!\n");
            if(errno==EAGAIN){
                return nullptr;
            }
            throw std::exception();
        }
        setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&reuse,sizeof(reuse));//设置端口复用
        events=EPOLLIN|EPOLLRDHUP|EPOLLONESHOT;

    }else if(type==2){//创建timerChannel

        new_channel->SetReadEventHandler(std::bind(&Channel::DefaultTimerfdReadEventHandler,new_channel.get()));
        sockfd=TimeUtil::CreateTimer();
        setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&reuse,sizeof(reuse));//设置端口复用
        events=EPOLLIN|EPOLLRDHUP|EPOLLONESHOT|EPOLLET;
        listenfd=0;

    }else{//创建监听Channel
        new_channel->SetReadEventHandler(std::bind(&Channel::DefaultListenfdReadEventHandler,new_channel.get()));
        listenfd=sockfd=socket(PF_INET,SOCK_STREAM,0);
        if(sockfd==-1){
            throw std::exception();
        }

        setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&reuse,sizeof(reuse));//设置端口复用

        saddr.sin_port=htons(value);
        saddr.sin_family=AF_INET;
        saddr.sin_addr.s_addr=INADDR_ANY;
        bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));//绑定端口

        if(listen(sockfd,5)==-1){
            printf("Create listen exception!\n");
            throw std::exception();
        }
        events=EPOLLIN|EPOLLRDHUP|EPOLLONESHOT;
    }

    SetNonBlocking(sockfd);
    //Channel* new_channel=new Channel;
    //std::shared_ptr<Channel> new_channel=std::make_shared<Channel>();
    //printf("befor MakeSelf, usecount is %d\n",new_channel.use_count());
    new_channel->MakeSelf(new_channel);
    //printf("after MakeSelf, usecount is %d\n",new_channel.use_count());
    new_channel->SetLoop(loop_);
    new_channel->Setfd(sockfd);
    new_channel->SetAddr(saddr);
    new_channel->SetEvents(events);
    new_channel->SetListenfd(listenfd);

    return new_channel;
}

void Channel::Destroy(std::shared_ptr<Channel> channel){
    channel->self.reset();
    while(channel.use_count()>2);
    //printf("delete, usecount is %d\n",channel.use_count());
}

void Channel::Update(){
        loop->UpdateChannel(self);
}

void Channel::Close(){
    loop->Close(self);
    //printf("关闭socket成功!\n");
}

bool Channel::Send(struct iovec* data, int len){
    int send_id=0;
    int send_num;
    while(1){
        send_num=writev(fd,data,len);
        if(send_num<=-1){
            if(errno == EAGAIN){//缓冲区满导致写入失败，继续写
                SetEvents(EPOLLOUT|EPOLLONESHOT|EPOLLRDHUP);
                write_flag=false;
                return true;
            }

            return false;
        }

        if(send_num<data[send_id].iov_len){
            data[send_id].iov_base=(char*)(data[send_id].iov_base)+send_num;
            data[send_id].iov_len-=send_num;
        }else{
            int num=data[send_id].iov_len;
            while(send_num>num){
                data[send_id++].iov_len=0;
                send_num-=num;
                num=data[send_id].iov_len;
            }
            if(send_num==num){
                //printf("sendover!\n");
                write_flag=true;
                return true;
            }
            data[send_id].iov_base=(char*)(data[send_id].iov_base)+send_num;
            data[send_id].iov_len-=send_num;
        }
    }
}

Channel::~Channel(){
    //printf("remove channel!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}

void Channel::HandleEvent(){
    //printf("this is HandleEvent!\n");
    this->handler();
}

void Channel::DefaultEventHandler(){
    
/*
0502:
    -当前采用struct iovec进行读写控制
    -read_callback以及write_callback均通过struct iovec*作为参数和返回值
    -返回值中
        -第一块struct iovec作为标志块
            -第一块中的iov_len标识了iovec的总块数
            -第一块中的iov_base总是需要在reactor中进行空间释放(要求必须通过new的方法生成)
            -第一块中的iov_base中前iov_len个char*标识了每一块是否需要在reactor中进行空间释放(仅write_iovec进行判断,当前read默认一定不需要在reactor中释放)
            -第一块中的iov_base中的第iov_len+1个char*用于标记是否断开连接,'1'代表保持连接,'0'代表断开连接
            -第一块中的iov_base中的第iov_len+2个char*用于标记是否发生TCP粘包,'1'代表是,'0'代表否
            -第一块中的iov_base中的第iov_len+3个char*用于标记接下来是否监听读事件,'1'代表是,'0'代表否
            -第一块中的iov_base中的第iov_len+4个char*用于标记接下来是否监听写事件,'1'代表是,'0'代表否
*/

    //printf("\n\n\nim processing!fd():%d\n",fd);

    if((revents & EPOLLIN)&&read_handler)read_handler();
    else if((revents & EPOLLOUT)&&write_handler)write_handler();
    // if(fd==loop->GetTimer()->Getfd()){
    //     printf("this timer turn is over!\n");
    // }
    //printf("DefaultEventHandler执行完毕!,fd is %d\n",fd);
}

void Channel::DefaultListenfdReadEventHandler(){

    struct sockaddr_in client_addr;
    socklen_t client_addr_len=sizeof(client_addr);

    EventLoop* loop=this->GetLoop();
    if(loop->GetChannelnum()>=loop->GetMaxchannelnum()){
        //close(conn_fd);//关闭连接，返回错误码
        return;
    }else{
        
        std::shared_ptr<Channel> conn_channel=Channel::Create(loop,this->Getfd());
        if(conn_channel==nullptr)return;
        conn_channel->SetReadCallback(loop->GetReadCallback());
        conn_channel->SetWriteCallback(loop->GetWriteCallback());
        conn_channel->SetCommunicateCallback(loop->GetCommunicateCallback());
        loop->GetEpoll()->AddChannel(conn_channel);
        loop->AddChannelnum();
    }

    SetEvents(EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);
    //printf("this is listenfd!\n");
    loop->GetEpoll()->Update(this->Getfd(),this->GetEvents());
}

void Channel::DefaultEventfdReadEventHandler()
{    
    if(!read_buffer.Read(fd)){//读取全部内容到reaad_buffer
        //printf("对方已关闭连接!\n");
        Close();
        return;
    }

    if(communicate_callback){
        if(!communicate_callback(read_buffer.GetData(),read_buffer.GetLen())){
            //粘包
            //printf("Tcp粘包!\n");
            SetEvents(EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);

            return;
        }
    }
    if(read_callback){

        struct iovec read_str[3];
        struct sockaddr_in addr;
        InitIovec(read_str,&addr);
        //int temp_fd=fd;
        read_str[0].iov_base=&fd;
        read_iovec=read_callback(read_str);//将读取到的内容传给回调函数，并返回需要写的内容,read_iovec需要在Process中删除,read_iovec写的是需要返回的东西，因此指向的内容也需要在Process中删除

        if(read_iovec){

            //将数据读到write缓冲区
            int len=read_iovec[0].iov_len;
            for(int i=1;i<len;i++){//将要写的内容添加到缓冲区
                write_buffer.append((char*)(read_iovec[i].iov_base),read_iovec[i].iov_len);
            }

            ProcessIovec(read_iovec);
            if(!alive){
                write_buffer.Write(fd);
                Close();

                return;
            }
            if(!clear_readbuf){//发生粘包
                write_buffer.Clear();//清空写缓冲区
                SetEvents(EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);
                return;
            }

            //不再需要通信,直接返回
            if((!read_or_write)||(!alive)){

                write_buffer.Write(fd);
                Close();

                return;
            }

            read_buffer.Clear();

            SetEvents(read_or_write|EPOLLONESHOT|EPOLLRDHUP);
        }
    }
}

void Channel::DefaultTimerfdReadEventHandler()
{
    TimeStamp now(TimeUtil::GetNow());
    TimeUtil::ReadTimerfd(this->Getfd());
    //printf("after reading timer!\n");
    std::vector<Timer*> expired_timers=this->GetLoop()->GetExpiredTimers(now);
    //printf("expired num is %d\n",expired_timers.size());
    SetEvents(EPOLLIN|EPOLLONESHOT|EPOLLRDHUP|EPOLLET);
    for(int i=0;i<expired_timers.size();i++){
        if(!expired_timers[i]->IsAlive()){
            //printf("delete timer!!!!!\n");
            delete expired_timers[i];
        }else{
            expired_timers[i]->RunCallback();
            if(expired_timers[i]->ResetCallTime()){
                this->GetLoop()->InsertTimer(expired_timers[i]);
            }else{
                //printf("delete timer!\n");
                delete expired_timers[i];
            }
        }
    }
}

void Channel::DefaultEventfdWriteEventHandler(){

    if(write_callback){
        //iov_base第一个字节用于标识该段char*由谁删除,1表示需要在这里删除
        struct iovec* write_iovec_;
        if(write_callback_flag&&write_iovec){
            int len=write_iovec[0].iov_len;
            write_iovec_=new struct iovec[len];
            for(int i=0;i<len;i++){
                write_iovec_[i].iov_base=write_iovec[i].iov_base;
                write_iovec_[i].iov_len=write_iovec[i].iov_len;
            }
        }
        if(!write_callback_flag){
            struct iovec read_str[3];
            struct sockaddr_in addr;
            InitIovec(read_str,&addr,false);
            write_iovec=write_callback(read_str);//将写缓冲区的内容传给写回调函数，并返回需要写的内容,write_iovec需要在Process中删除,write_iovec写的是一些常态化数据,如文件内容,write_buffer缓冲区内容等，不需要在Process中删除
            write_callback_flag=true;
            int len=write_iovec[0].iov_len;
            write_iovec_=new struct iovec[len];
            for(int i=0;i<len;i++){
                write_iovec_[i].iov_base=write_iovec[i].iov_base;
                write_iovec_[i].iov_len=write_iovec[i].iov_len;
            }
        }
        if(!Send(write_iovec_+1,write_iovec_[0].iov_len-1)){
            //错误关闭

            delete [] write_iovec_;
            if(write_iovec)ProcessIovec(write_iovec);
            //printf("sure close!\n");
            Close();

            return;
        }

        delete [] write_iovec_;

        if(write_flag){
            write_buffer.Clear();

            if(write_iovec){
                ProcessIovec(write_iovec);
                if((!read_or_write)||(!alive)){
                    Close();
                    return;
                }
                SetEvents(read_or_write|EPOLLONESHOT|EPOLLRDHUP);
            }
            write_flag=false;
            write_callback_flag=false;
            write_iovec=nullptr;
        }
    }else{
        Close();
        return;
    }
}

bool Channel::SetEventHandler(EventHandler handler_){
    this->handler=handler_;
}

bool Channel::SetReadEventHandler(EventHandler read_handler_){
    this->read_handler=read_handler_;
}

bool Channel::SetWriteEventHandler(EventHandler write_handler_){
    this->write_handler=write_handler_;
}

// bool Channel::Process(){

// /*
// 0502:
//     -当前采用struct iovec进行读写控制
//     -read_callback以及write_callback均通过struct iovec*作为参数和返回值
//     -返回值中
//         -第一块struct iovec作为标志块
//             -第一块中的iov_len标识了iovec的总块数
//             -第一块中的iov_base总是需要在reactor中进行空间释放(要求必须通过new的方法生成)
//             -第一块中的iov_base中前iov_len个char*标识了每一块是否需要在reactor中进行空间释放(仅write_iovec进行判断,当前read默认一定不需要在reactor中释放)
//             -第一块中的iov_base中的第iov_len+1个char*用于标记是否断开连接,'1'代表保持连接,'0'代表断开连接
//             -第一块中的iov_base中的第iov_len+2个char*用于标记是否发生TCP粘包,'1'代表是,'0'代表否
//             -第一块中的iov_base中的第iov_len+3个char*用于标记接下来是否监听读事件,'1'代表是,'0'代表否
//             -第一块中的iov_base中的第iov_len+4个char*用于标记接下来是否监听写事件,'1'代表是,'0'代表否
// */

//     //printf("\n\n\nim processing!fd():%d\n",fd);

//     if(listen_fd==fd){
//         //read_callback(nullptr);
//         DefaultListenfdReadEventHandler();
//         return true;
//     }

//     if(revents & EPOLLIN){
//         //读
//         //if(read_callback){
//         //    read_callback
//         //}
//         if(!read_buffer.Read(fd)){//读取全部内容到reaad_buffer
//             Close();
//             return false;
//         }

//         if(communicate_callback){
//             if(!communicate_callback(read_buffer.GetData(),read_buffer.GetLen())){
//                 //粘包
//                 //printf("Tcp粘包!\n");
//                 SetEvents(EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);

//                 return true;
//             }
//         }

//         if(read_callback){
   
//             struct iovec read_str[3];
//             char value[3];
//             read_str[0].iov_base=value;
//             read_str[0].iov_len=3;
//             struct sockaddr_in addr;
//             socklen_t addr_size=sizeof(addr);
//             getpeername(fd,(struct sockaddr*)&addr,&addr_size);
//             read_str[1].iov_base=(void*)&addr;
//             read_str[1].iov_len=addr_size;
//             read_str[2].iov_base=read_buffer.GetData();
//             read_str[2].iov_len=read_buffer.GetLen();
//             read_iovec=read_callback(read_str);//将读取到的内容传给回调函数，并返回需要写的内容,read_iovec需要在Process中删除,read_iovec写的是需要返回的东西，因此指向的内容也需要在Process中删除

//             if(read_iovec){

//                 //将数据读到write缓冲区
//                 int len=read_iovec[0].iov_len;
//                 for(int i=1;i<len;i++){//将要写的内容添加到缓冲区
//                     write_buffer.append((char*)(read_iovec[i].iov_base),read_iovec[i].iov_len);
//                 }

//                 ProcessIovec(read_iovec);
//                 if(!clear_readbuf){//发生粘包
//                     write_buffer.Clear();//清空写缓冲区
//                     SetEvents(EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);
//                     return true;
//                 }

//                 //不再需要通信,直接返回
//                 if((!read_or_write)||(!alive)){

//                     Close();

//                     return true;
//                 }

//                 read_buffer.Clear();

//                 SetEvents(read_or_write|EPOLLONESHOT|EPOLLRDHUP);
//             }
//             //write_buffer.append(&write_str[0],write_str.size());//将要写的内容添加到缓冲区
//             //if(write_callback)SetEvents(EPOLLOUT|EPOLLONESHOT|EPOLLRDHUP);
//             //else SetEvents(EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);
//         }
//     }
//     if(revents & EPOLLOUT){
//         //写
//         if(write_callback){
//             //iov_base第一个字节用于标识该段char*由谁删除,1表示需要在这里删除
//             struct iovec* write_iovec_;
//             if(write_callback_flag&&write_iovec){
//                 int len=write_iovec[0].iov_len;
//                 write_iovec_=new struct iovec[len];
//                 for(int i=0;i<len;i++){
//                     write_iovec_[i].iov_base=write_iovec[i].iov_base;
//                     write_iovec_[i].iov_len=write_iovec[i].iov_len;
//                 }
//             }
//             if(!write_callback_flag){
//                 struct iovec read_str[3];
//                 char value[3];
//                 read_str[0].iov_base=value;
//                 read_str[0].iov_len=3;
//                 struct sockaddr_in addr;
//                 socklen_t addr_size=sizeof(addr);
//                 getpeername(fd,(struct sockaddr*)&addr,&addr_size);
//                 read_str[1].iov_base=(void*)&addr;
//                 read_str[1].iov_len=addr_size;
//                 read_str[2].iov_base=write_buffer.GetData();
//                 read_str[2].iov_len=write_buffer.GetLen();
//                 write_iovec=write_callback(read_str);//将写缓冲区的内容传给写回调函数，并返回需要写的内容,write_iovec需要在Process中删除,write_iovec写的是一些常态化数据,如文件内容,write_buffer缓冲区内容等，不需要在Process中删除
//                 write_callback_flag=true;
//                 int len=write_iovec[0].iov_len;
//                 write_iovec_=new struct iovec[len];
//                 for(int i=0;i<len;i++){
//                     write_iovec_[i].iov_base=write_iovec[i].iov_base;
//                     write_iovec_[i].iov_len=write_iovec[i].iov_len;
//                 }
//             }
//             if(!Send(write_iovec_+1,write_iovec_[0].iov_len-1)){
//                 //错误关闭
//                 //printf("close!\n");

//                 delete [] write_iovec_;
//                 if(write_iovec)ProcessIovec(write_iovec);

//                 //printf("sure close!\n");

//                 Close();

//                 return false;
//             }

//             delete [] write_iovec_;

//             if(write_flag){
//                 write_buffer.Clear();

//                 if(write_iovec){
//                     ProcessIovec(write_iovec);
//                     if((!read_or_write)||(!alive)){
//                         Close();
//                         return false;
//                     }
//                     SetEvents(read_or_write|EPOLLONESHOT|EPOLLRDHUP);

//                     // int len=write_iovec[0].iov_len;
//                     // for(int i=len-1;i>=0;i--){
//                     //     if(*((char*)(write_iovec[0].iov_base)+i)=='1')delete [] write_iovec[i].iov_base;
//                     // }
//                     // delete [] write_iovec;

//                 }
//                 // for(int i=0;i<len;i++){
//                 //     delete (write_iovec+i);
//                 // }

//                 write_flag=false;
//                 write_callback_flag=false;
//                 write_iovec=nullptr;
//                 //SetEvents(EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);
//             }
//         }else{
//             Close();
//             return false;
//         }
//         //write_buffer.Write(fd);
//         //SetEvents(EPOLLIN|EPOLLOUT|EPOLLONESHOT|EPOLLRDHUP);
//     }

//     return true;
// }