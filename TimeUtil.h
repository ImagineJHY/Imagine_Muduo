#ifndef IMAGINE_MUDUO_TIMEUTIL_H
#define IMAGINE_MUDUO_TIMEUTIL_H

#include<functional>
#include<sys/uio.h>

namespace Imagine_Muduo{


class TimeStamp;
class Timer;

class TimeUtil{

// public:
//     using TimerCallback=std::function<void()>;

public:
    static int CreateTimer();

    static bool ReadTimerfd(int timerfd_);

    static long long GetTimerfdSettingTime(int timerfd_);

    static struct timespec GetTimespecFromNow(const TimeStamp& time_);

    static void SetDefaultTimerfd(const int timerfd_);

    static void ResetTimerfd(const int timerfd_, const TimeStamp& time_);

    static long long GetNow();

    static long long MicroSecondsAddSeconds(long long time_, double interval);

public:
    static const int micro_seconds_per_second;

public:
    class TimerPtrCmp{
        public:
            bool operator()(const Timer* a, const Timer* b);
    };
};


}



#endif