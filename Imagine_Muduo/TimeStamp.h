#ifndef IMAGINE_MUDUO_TIMESTAMP_H
#define IMAGINE_MUDUO_TIMESTAMP_H

#include"TimeUtil.h"

namespace Imagine_Muduo{

class TimeStamp{
    
public:
    explicit TimeStamp(long long time_=0):micro_seconds_time(time_){};

    ~TimeStamp(){};

    long long GetTime()const{return micro_seconds_time;}

    bool SetTime(long long time_){
        micro_seconds_time=time_;
        return true;
    }

private:
    long long micro_seconds_time;
};


}




#endif