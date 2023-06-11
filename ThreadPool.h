#ifndef IMAGINE_MUDUO_THREADPOOL_H
#define IMAGINE_MUDUO_THREADPOOL_H

#include<pthread.h>
#include<list>
#include<semaphore.h>
#include<stdio.h>

namespace Imagine_Muduo{


template <typename T>
class ThreadPool{

// public:
//     int block_num=0;

public:
    ThreadPool(int thread_num=10,int max_request=10000);

    ~ThreadPool();

    void PutTask(T task);

    T GetTask();

    static void* Worker(void* data);

private:
    int thread_num;
    int max_request;
    int quit=0;
    pthread_t* threads;
    std::list<T> tasks;
    pthread_mutex_t lock;
    sem_t sem;
};

template <typename T>
ThreadPool<T>::ThreadPool(int thread_num,int max_request):thread_num(thread_num),max_request(max_request),threads(nullptr),quit(0)
{
    if(thread_num<0||max_request<0){
        throw std::exception();
    }

    threads=new pthread_t[thread_num];
    if(!threads){
        throw std::exception();
    }

    if(pthread_mutex_init(&lock,nullptr)!=0){
        throw std::exception();
    }
    
    if(sem_init(&sem,0,0)!=0){
        throw std::exception();
    }

    for(int i=0;i<thread_num;i++){
        printf("create pthread %d ...\n",i);
        if(pthread_create(threads+i,nullptr,Worker,this)!=0){
            delete [] threads;
            throw std::exception();
        }

        if(pthread_detach(*(threads+i))){
            delete [] threads;
            throw std::exception();
        }
    }
}

template <typename T>
ThreadPool<T>::~ThreadPool(){
    delete [] threads;
    quit=true;
    pthread_mutex_destroy(&lock);
    sem_destroy(&sem);
}

template <typename T>
void ThreadPool<T>::PutTask(T task){
    if(tasks.size()<max_request){
        pthread_mutex_lock(&lock);
        tasks.push_back(task);
        //printf("I put a task!\n");
        //printf("block_num is %d\n,task num is %d\n",block_num,tasks.size());
        pthread_mutex_unlock(&lock);
        sem_post(&sem);
    }else{
        //给客户端返回一个错误码
    }
}

template <typename T>
T ThreadPool<T>::GetTask(){
    while(!quit){
        //block_num++;
        sem_wait(&sem);
        pthread_mutex_lock(&lock);
        //block_num--;
        //printf("block_num is %d\n,task num is %d\n",block_num,tasks.size());
        if(tasks.empty()){
            pthread_mutex_unlock(&lock);
            continue;
        }
        T task=tasks.front();
        tasks.pop_front();
        pthread_mutex_unlock(&lock);

        return task;
    }
}

template <typename T>
void* ThreadPool<T>::Worker(void* data){
    ThreadPool<T>* threadpool=(ThreadPool<T>*)data;
    while(!threadpool->quit){
        T task=threadpool->GetTask();
        //printf("I Get a Task!\n");
        //printf("任务开始,use_count为:%d\n",task.use_count());
        if(task)task->HandleEvent();
        //task->Process();
        //printf("任务完成,use_count为:%d\n",task.use_count());
    }

    return nullptr;
}





}




#endif