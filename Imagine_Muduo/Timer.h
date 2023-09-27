#ifndef IMAGINE_MUDUO_TIMER_H
#define IMAGINE_MUDUO_TIMER_H

#include <functional>

#include "TimeStamp.h"
#include "TimeUtil.h"
#include "Callbacks.h"

namespace Imagine_Muduo
{

class Timer
{
    // public:
    //     using TimerCallback=std::function<void()>;
public:
    Timer(const TimeStamp &call_time, const TimerCallback &timer_callback, double interval = 0)
        : call_time_(call_time), timer_callback_(timer_callback), interval_(interval), repeat_(interval > 0.0), timer_id_(Timer::id_++), alive_(true) {}

    ~Timer(){};

    void RunCallback()
    {
        timer_callback_();
    }

    bool ResetCallTime()
    {
        if (!alive_) {
            return false;
        }
        if (repeat_) {
            call_time_.SetTime(TimeUtil::MicroSecondsAddSeconds(TimeUtil::GetNow(), interval_));
        } else {
            return false;
        }

        return true;
    }

    TimeStamp GetCallTime() const { return call_time_; }

    bool Close()
    {
        alive_ = false;
        return true;
    }

    long long GetTimerId() { return timer_id_; }

    bool IsAlive() { return alive_; }

 private:
    TimeStamp call_time_;          // 记录绝对时间,微秒为单位
    TimerCallback timer_callback_; // 回调函数
    const double interval_;        // 记录时间间隔,秒为单位
    bool repeat_;                 // 是否重复定时

    const long long timer_id_;
    bool alive_ = true;

 private:
    static long long id_;
};

} // namespace Imagine_Muduo

#endif