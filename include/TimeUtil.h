#ifndef IMAGINE_MUDUO_TIMEUTIL_H
#define IMAGINE_MUDUO_TIMEUTIL_H

#include <functional>
#include <sys/uio.h>

namespace Imagine_Muduo
{

class TimeStamp;
class Timer;

class TimeUtil
{
// public:
// using TimerCallback=std::function<void()>;
 public:
    static int CreateTimer();

    static bool ReadTimerfd(int timer_fd);

    static long long GetTimerfdSettingTime(int timer_fd);

    static struct timespec GetTimespecFromNow(const TimeStamp &time);

    static void SetDefaultTimerfd(const int timer_fd);

    static void ResetTimerfd(const int timer_fd, const TimeStamp &time);

    static long long GetNow();

    static long long MicroSecondsAddSeconds(long long time, double interval);

 public:
    static const int micro_seconds_per_second;

 public:
    class TimerPtrCmp
    {
     public:
        bool operator()(const Timer *a, const Timer *b);
    };
};

} // namespace Imagine_Muduo

#endif