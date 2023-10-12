#ifndef IMAGINE_MUDUO_TIMESTAMP_H
#define IMAGINE_MUDUO_TIMESTAMP_H

#include "TimeUtil.h"

namespace Imagine_Muduo
{

class TimeStamp
{
 public:
    explicit TimeStamp(long long time = 0) : micro_seconds_time_(time){};

    ~TimeStamp(){};

    long long GetTime() const { return micro_seconds_time_; }

    bool SetTime(long long time)
    {
        micro_seconds_time_ = time;
        return true;
    }

 private:
    long long micro_seconds_time_;
};

} // namespace Imagine_Muduo

#endif