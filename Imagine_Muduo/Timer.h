#ifndef IMAGINE_MUDUO_TIMER_H
#define IMAGINE_MUDUO_TIMER_H

#include<functional>

#include"TimeStamp.h"
#include"TimeUtil.h"
#include"Callbacks.h"

namespace Imagine_Muduo{


class Timer{

// public:
//     using TimerCallback=std::function<void()>;

public:
    Timer(const TimeStamp& call_time_, const TimerCallback& timer_callback_, double interval_=0)
        :call_time(call_time_),timer_callback(timer_callback_),interval(interval_),repeat_(interval_>0.0),timer_id(Timer::id++),alive(true)
    {
    }

    ~Timer(){};

    void RunCallback(){
        timer_callback();
    }

    bool ResetCallTime(){
        if(!alive)return false;
        if(repeat_)call_time.SetTime(TimeUtil::MicroSecondsAddSeconds(TimeUtil::GetNow(),interval));
        else return false;
        return true;
    }

    TimeStamp GetCallTime()const{return call_time;}

    bool Close(){alive=false;return true;}

    long long GetTimerId(){return timer_id;}

    bool IsAlive(){return alive;}

private:
    TimeStamp call_time;//记录绝对时间,微秒为单位
    TimerCallback timer_callback;//回调函数
    const double interval;//记录时间间隔,秒为单位
    bool repeat_;//是否重复定时

    const long long timer_id;
    bool alive=true;

private:
    static long long id;
};



}

#endif