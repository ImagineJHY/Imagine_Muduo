#include"TimeUtil.h"

#include<sys/time.h>
#include<unistd.h>
#include<sys/timerfd.h>
#include<string.h>
#include<errno.h>

#include"Timer.h"
#include"TimeStamp.h"

using namespace Imagine_Muduo;

const int TimeUtil::micro_seconds_per_second=1000*1000;

int TimeUtil::CreateTimer(){
    int timerfd_=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    if(timerfd_<0){
        throw std::exception();
    }

    return timerfd_;
}

bool TimeUtil::ReadTimerfd(int timerfd_){
    unsigned long long num;
    int ret=read(timerfd_,&num,sizeof(num));
    if(ret!=sizeof(num)){
        //printf("std::exception()::next time is %lld\n",TimeUtil::GetTimerfdSettingTime(timerfd_));
        //throw std::exception();
    }
    if(errno==EAGAIN){
        //printf("......................%lld.....................\n",num);
        return false;
    }
    //printf("......................%d.....................\n",ret);
    //printf("there are %lld clocks occured!\n",num);
    return true;
}

long long TimeUtil::GetTimerfdSettingTime(int timerfd_){
    struct itimerspec set_time;
    timerfd_gettime(timerfd_,&set_time);

    return static_cast<long long>(set_time.it_value.tv_sec)*TimeUtil::micro_seconds_per_second+static_cast<long long>(set_time.it_value.tv_nsec)/1000;
}

struct timespec TimeUtil::GetTimespecFromNow(const TimeStamp& time_){

    long long micro_seconds=time_.GetTime()-GetNow();
    if(micro_seconds<0){
        printf("ur 2 small!\n");
    }
    //printf("micro seconds is %lld\n",micro_seconds);
    if(micro_seconds<100){
        micro_seconds=100;
        printf("too short .....................................................................................too short\n");
    }
    struct timespec time_spec;
    //printf("Set seconds is %ld\n",new_value_.it_value.tv_sec*1000000+new_value_.it_value.tv_nsec*1000)
    time_spec.tv_sec=static_cast<time_t>(micro_seconds/micro_seconds_per_second);
    time_spec.tv_nsec=static_cast<long>((micro_seconds%micro_seconds_per_second)*1000);

    return time_spec;
}

void TimeUtil::SetDefaultTimerfd(const int timerfd_){

    TimeStamp time_(MicroSecondsAddSeconds(GetNow(),5.0));
    struct itimerspec old_value;
    struct itimerspec new_value;
    memset(&old_value,0,sizeof(old_value));
    memset(&new_value,0,sizeof(new_value));
    new_value.it_value=GetTimespecFromNow(time_);
    //memset(&new_value,0,sizeof(new_value));
    int ret = timerfd_settime(timerfd_, 0, &new_value, &old_value);
    if (ret)
    {
        printf("exception here3\n");
        throw std::exception();
    }
}

void TimeUtil::ResetTimerfd(const int timerfd_,const TimeStamp& time_){

    struct itimerspec old_value;
    struct itimerspec new_value;
    memset(&old_value,0,sizeof(old_value));
    memset(&new_value,0,sizeof(new_value));
    new_value.it_value=GetTimespecFromNow(time_);
    int ret = timerfd_settime(timerfd_, 0, &new_value, &old_value);
    //printf("reset timer after %lf\n",(new_value.it_value.tv_sec)+(new_value.it_value.tv_nsec/1000)*0.000001);
    if (ret)
    {
        printf("exception here1\n");
        throw std::exception();
    }
}

long long TimeUtil::GetNow(){

    struct timeval tv;
    gettimeofday(&tv,NULL);

    return static_cast<long long>(tv.tv_sec)*micro_seconds_per_second+static_cast<long long>(tv.tv_usec);
}

long long TimeUtil::MicroSecondsAddSeconds(long long time_, double interval){
    long long interval_=static_cast<long long>(interval*micro_seconds_per_second);
    return static_cast<long long>(time_+interval_);
}

bool TimeUtil::TimerPtrCmp::operator()(const Timer* a, const Timer* b){
    return a->GetCallTime().GetTime()<b->GetCallTime().GetTime()?true:false;
}